
#include "Config.h"

#if defined(HAS_MULTIPLAYER) && defined(WIN32)


#include <stdio.h>
#include <stdlib.h>

#include <memory.h>
#include "memoryallocation.h"
#include "WinNetwork.h"
#include <Ws2tcpip.h>

#ifndef USE_FANCY_MENUS
	#include "hg/HighGear.h"
#endif

// constructor
CWinNetwork::CWinNetwork()
{
	debug_out("CWinNetwork:: constructor");

	ListenSocket = INVALID_SOCKET;
	DiscoverSocket = INVALID_SOCKET;
	for (int i = 0; i < DEVICE_MAX; i++)
		DataSocket[i] = INVALID_SOCKET;

	state = STATE_NONE;

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
		debug_out("CWinNetwork::CWinNetwork: WSAStartup failed: %d\n", iResult);
}

// update
bool CWinNetwork::update()
{
	switch(state)
	{
		case STATE_ACCEPT:
		{
			fd_set setR;
			timeval tval = {0, 0};
			FD_ZERO(&setR);
			int nfds = 0;
			
			FD_SET(ListenSocket, &setR);
			nfds = nfds < ListenSocket ? ListenSocket : nfds;
			
			FD_SET(DiscoverSocket, &setR);
			nfds = nfds < DiscoverSocket ? DiscoverSocket : nfds;
			nfds++;


			int iResult = select(nfds, &setR, NULL, NULL, &tval);

			if (iResult == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				debug_out("CWinNetwork::update accept: error=%d\n", error);
				CleanUp();
				return false;
			}
			else if (iResult == 0)
			{
				break;
			}

			if (FD_ISSET(ListenSocket, &setR))
			{
				struct sockaddr addr;
				int addrLen = sizeof(struct sockaddr);
#ifdef USE_FANCY_MENUS
				int devIdx = m_iDevicesCount;
#else
				int devIdx = CHighGear::GetInstance()->device_to_confirm_nb;
#endif

				// Accept a client socket
				DataSocket[devIdx] = accept(ListenSocket, &addr, &addrLen);
				if (DataSocket[devIdx] == INVALID_SOCKET)
				{
					debug_out("CWinNetwork::AcceptLoop accept failed: %d\n", WSAGetLastError());
					CleanUp();
					return false;
				}

				DeviceDetails* dd = NEW DeviceDetails();

				dd->iAddress = NEW sockaddr(addr);

//				if (getnameinfo(&addr, addrLen, dd->iName, sizeof(dd->iName), NULL, 0, 0) != 0)
//				{
//					debug_out("CWinNetwork::Error when retrieving client name\n");
//					CleanUp();
//					return false;
//				}
				
				// we must receive the host name
				iResult = recv(DataSocket[devIdx], (char *)dataBuff, DATA_BUFF_SIZE, 0);
				if (iResult == -1)
				{
					debug_out("CWinNetwork::DeviceReceive: recv error %d\n", WSAGetLastError());
					CleanUp();
					return false;
				}
				
				iResult = iResult < sizeof(dd->iName) ? iResult : sizeof(dd->iName) - 1;
				
				memcpy(dd->iName, dataBuff, iResult);
				dd->iName[iResult] = 0; // adding null terminator

				Connected(devIdx);
				
#ifdef USE_FANCY_MENUS
				AddDevice(dd);
#else
				CHighGear::GetInstance()->ConfirmRemoteDevice(dd);
#endif
			}

			if (FD_ISSET(DiscoverSocket, &setR))
			{
				debug_out("CWinNetwork::update discover 1\n");

				struct sockaddr addr;
				int addrLen = sizeof(struct sockaddr);

				int len = recvfrom(DiscoverSocket, (char*)dataBuff, DATA_BUFF_SIZE, 0, &addr, &addrLen);
				if (len == SOCKET_ERROR)
				{
					debug_out("CWinNetwork::Error when receiving data with udp socket: %d\n", WSAGetLastError());
					CleanUp();
					return false;
				}

				debug_out("CWinNetwork::update discover 2\n");

				// checking signature
				if (memcmp(&KUidGame, dataBuff, sizeof(KUidGame)) == 0)
				{
					debug_out("CWinNetwork::update discover 3\n");
					
					strcpy((char*)(dataBuff + sizeof(KUidGame)), m_pLocalDeviceName);

					// sending response to client
					if (sendto(DiscoverSocket, (char*)dataBuff, sizeof(KUidGame) + strlen(m_pLocalDeviceName), 0, &addr, addrLen) == SOCKET_ERROR)
					{
						debug_out("CWinNetwork::Error when responding using udp socket: %d\n", WSAGetLastError());
						CleanUp();
						return false;
					}

					debug_out("CWinNetwork::update discover 4\n");
				}
			}

			break;
		}

		case STATE_DISCOVER:
		{
			fd_set setR;
			timeval tval = {0, 0};
			FD_ZERO(&setR);
			FD_SET(DiscoverSocket, &setR);

			int iResult;
			iResult = select(DiscoverSocket + 1, &setR, NULL, NULL, &tval);
			if (iResult == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				debug_out("CWinNetwork::update discover: error=%d\n", error);
				CleanUp();
				return false;
			}
			else if (iResult == 0)
			{
				break;
			}

			struct sockaddr addr;
			int addrLen = sizeof(struct sockaddr);

			int len = recvfrom(DiscoverSocket, (char*)dataBuff, DATA_BUFF_SIZE, 0, &addr, &addrLen);
			if (len == SOCKET_ERROR)
			{
				debug_out("CWinNetwork::Error when receiving data with udp socket: %d\n", WSAGetLastError());
				CleanUp();
				return false;
			}

			// checking signature
			if (memcmp(&KUidGame, dataBuff, sizeof(KUidGame)) == 0)
			{
				DeviceDetails* dd = NEW DeviceDetails();

				dd->iAddress	  = NEW sockaddr(addr);
				
				memcpy(dd->iName, dataBuff + sizeof(KUidGame), len - sizeof(KUidGame));

				debug_out("CWinNetwork::Server name = %s\n", dd->iName);

				AddDevice(dd);
			}
			break;
		}

		case STATE_RECV_DATA:
		{		
			int i;

			fd_set setR;
			timeval tval = {0, 0};
			FD_ZERO(&setR);
			int nfds = 0;

			for (i = 0; i < m_iDevicesCount; i++)
			{
				FD_SET(DataSocket[i], &setR);
				nfds = nfds < DataSocket[i] ? DataSocket[i] : nfds;
			}
			nfds++;

			int iResult = select(nfds, &setR, NULL, NULL, &tval);
			if (iResult == SOCKET_ERROR)
			{
				debug_out("CWinNetwork::DeviceReceive: select error %d\n", WSAGetLastError());
				CleanUp();
				return false;
			}

		//	debug_out("CWinNetwork::DeviceReceive select returns : %d\n", iResult);
			if (iResult == 0)
				break;

			for (i = 0; i < m_iDevicesCount; i++)
			{
				if (FD_ISSET(DataSocket[i], &setR))
				{
					iResult = recv(DataSocket[i], (char *)dataBuff, DATA_BUFF_SIZE, 0);
					if (iResult == -1)
					{
						debug_out("CWinNetwork::DeviceReceive: recv error %d\n", WSAGetLastError());
						CleanUp();
						return false;
					}

					if (iResult > 0)
					{
//						// ignoring disconnected devices
//						if (m_bDevicesCnx[i])
						{
							//debug_out("CWinNetwork::OnDataRecv: type = %d len = %d clientId = %d\n", dataBuff[0], iResult, i);
							if (!OnDataRecv(dataBuff, iResult, i))
							{
								debug_out("CWinNetwork::Queue overflow !!!\n");
								return false;
							}
						}
					}
					else if (iResult == 0)
					{	
						// connection closed
						if(m_iDevicesConnected != 0)
							m_iDevicesConnected--;

						if (m_iDevicesConnected == 0)
							isConnected = false;											

						// set connexion state of the device
						m_bDevicesCnx[i] = false;

						debug_out("CWinNetwork:: Connection closed by device %d\n", i);
					}
				}
			}

			break;
		}
	}

	return true;
}

// hosts search
bool CWinNetwork::DiscoverServers()
{
	debug_out("CWinNetwork::DiscoverServers\n");

	InitDiscover(false);

	struct sockaddr_in addr_Broadcast;

	ZeroMemory( &addr_Broadcast, sizeof(addr_Broadcast) );
    addr_Broadcast.sin_family      = AF_INET;
    addr_Broadcast.sin_port        = htons(atoi(DISCOVER_PORT));
    addr_Broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	// send broadcast message
	if (sendto(DiscoverSocket, (char*)&KUidGame, sizeof(KUidGame), 0, (struct sockaddr *)&addr_Broadcast, sizeof(addr_Broadcast)) == SOCKET_ERROR)
	{
		debug_out("CWinNetwork::Error when sending discover message: %d\n", WSAGetLastError());
		CleanUp();
		return false;
	}

	state = STATE_DISCOVER;

	return true;
}

// connections
bool CWinNetwork::StartServer()
{
	debug_out("CWinNetwork::StartServer\n");

	isServer = true;

//	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
//	if (iResult != 0)
//	{
//		debug_out("CWinNetwork::StartHost WSAStartup failed: %d\n", iResult);
//		return false;
//	}

	InitDiscover(true);

	// Initialize listen socket
	struct addrinfo *result = NULL,	hints;

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, LISTEN_PORT, &hints, &result);
	if ( iResult != 0 )
	{
		debug_out("CWinNetwork::StartHost getaddrinfo failed: %d\n", iResult);
		CleanUp();
		return false;
	}

	if (ListenSocket == INVALID_SOCKET)
	{
		// Create a SOCKET for the server to listen for client connections
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET)
		{
			debug_out("CWinNetwork::StartHost Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			CleanUp();
			return false;
		}

		// Setup the TCP listening socket
		iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			debug_out("CWinNetwork::StartHost bind failed: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			CleanUp();
			return false;
		}

		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			debug_out("CWinNetwork::StartHost Error at bind(): %ld\n", WSAGetLastError() );
			CleanUp();
			return false;
		}
	}

	freeaddrinfo(result);

	state = STATE_ACCEPT;

	return true;
}

bool CWinNetwork::InitDiscover(bool server)
{
	// Initialize discover socket
	int iResult;
	struct addrinfo *result = NULL, hints;

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DISCOVER_PORT, &hints, &result);
	if ( iResult != 0 )
	{
		debug_out("CWinNetwork::InitDiscover getaddrinfo failed: %d\n", iResult);
		CleanUp();
		return false;
	}

	if (DiscoverSocket == INVALID_SOCKET)
	{
		DiscoverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (DiscoverSocket == INVALID_SOCKET)
		{
			debug_out("CWinNetwork::InitDiscover Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			CleanUp();
			return false;
		}

		if (server)
		{
			iResult = bind(DiscoverSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				debug_out("CWinNetwork::InitDiscover bind failed: %d\n", WSAGetLastError());
				freeaddrinfo(result);
				CleanUp();
				return false;
			}
		}

		int bOptVal = 1;

		if (setsockopt(DiscoverSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, sizeof(int)) == SOCKET_ERROR)
		{
			debug_out("CWinNetwork::InitDiscover setsockopt failed: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			CleanUp();
			return false;
		}
	}

	freeaddrinfo(result);

	return true;
}

void CWinNetwork::StopListening()
{
	debug_out("CWinNetwork::StopListening\n");

	closesocket(ListenSocket);
	ListenSocket = INVALID_SOCKET;

	closesocket(DiscoverSocket);
	DiscoverSocket = INVALID_SOCKET;

	state = STATE_RECV_DATA;
}

bool CWinNetwork::StartClient()
{
	debug_out("CWinNetwork::StartClient\n");

	isServer = false;

	return true;
}

// connection
bool CWinNetwork::Connect(int serverIdx)
{
	if (state!=STATE_DISCOVER)
		return false;

	if (isServer)
		return false;

	debug_out("CWinNetwork::Connect to server %d\n", serverIdx);

	isConnecting = true;

	struct sockaddr_in* addr;
	int iResult;

	addr = (struct sockaddr_in*)m_pDevices[serverIdx]->iAddress;

	addr->sin_port = htons(atoi(LISTEN_PORT));

	// clean
	for (int i = m_iDevicesCount - 1; i >= 0; i--)
	{
		closesocket(DataSocket[i]);
		DataSocket[i] = INVALID_SOCKET;

		if (i != serverIdx)
			RemoveDevice(i);
	}
/*
	closesocket(DiscoverSocket);
	DiscoverSocket = INVALID_SOCKET;
*/

	// Create a SOCKET for connecting to server
	DataSocket[0] = socket(addr->sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (DataSocket[0] == INVALID_SOCKET)
	{
		isConnecting = false;
		debug_out("CWinNetwork::ConnectLoop Error at socket(): %ld\n", WSAGetLastError());
		CleanUp();
		return false;
	}

	// Connect to server.
	iResult = connect(DataSocket[0], (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if (iResult == SOCKET_ERROR)
	{
		isConnecting = false;
		debug_out("CWinNetwork::ConnectLoop Error connect: %d\n", WSAGetLastError());
		CleanUp();
		return false;
	}
	
	iResult = send(DataSocket[0], m_pLocalDeviceName, strlen(m_pLocalDeviceName), 0);
	if (iResult == SOCKET_ERROR)
	{
		isConnecting = false;
		debug_out("CWinNetwork::Connect Error: send failed: %d\n", WSAGetLastError());
		CleanUp();
		return false;
	}

	Connected(0);

	state = STATE_RECV_DATA;
	
	return true;
}

void CWinNetwork::Disconnect(int idx)
{
	debug_out("CWinNetwork::Disconnect\n");

	for (unsigned char i = 0; i < m_iDevicesCount; i++)
	{
		Comms::Disconnect(i);

		if( DataSocket[i] != INVALID_SOCKET )
		{
			closesocket(DataSocket[i]);
			DataSocket[i] = INVALID_SOCKET;
		}
	}
/*
	if( ListenSocket != INVALID_SOCKET )
	{
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
	}

	if( DiscoverSocket != INVALID_SOCKET )
	{
		closesocket(DiscoverSocket);
		DiscoverSocket = INVALID_SOCKET;
	}
*/
	ClearDevicesList();
}

// send/receive data
bool CWinNetwork::SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId)
{
	if (clientId == CLIENT_ID_ALL)
	{
		bool error = false;
		for (unsigned char clId = 0; clId < m_iDevicesCount; clId++)
		{
			if (!SendData(data, dataLen, clId))
				error = true;
		}

		return !error;
	}
	else
	{
		//debug_out("CWinNetwork::SendData: type = %d len = %d clientId = %d\n", data[0], dataLen, clientId);

		fd_set setW;
		FD_ZERO(&setW);
		FD_SET(DataSocket[clientId], &setW);

		int iResult = select(DataSocket[clientId] + 1, NULL, &setW, NULL, NULL);
		if (iResult == SOCKET_ERROR)
		{
			debug_out("CWinNetwork::DeviceSend: select error %d\n", WSAGetLastError());
			CleanUp();
			return false;
		}

		iResult = send(DataSocket[clientId], (const char *)data, dataLen, 0);
		if (iResult == SOCKET_ERROR)
		{
			debug_out("CWinNetwork::DeviceSend send failed: %d\n", WSAGetLastError());
			CleanUp();
			return false;
		}
	}

	return true;
}

// local device name
char* CWinNetwork::GetLocalDeviceName()
{
	if (m_pLocalDeviceName[0] == 0)
	{
		if (gethostname(m_pLocalDeviceName, LOCAL_DEVICE_NAME_SIZE) == SOCKET_ERROR)
		{
			debug_out ("Error %d when getting local host name.", WSAGetLastError());
			m_pLocalDeviceName[0] = 0; // empty string
		}
		else
		{
			char* p = strstr(m_pLocalDeviceName, ".");
			if (p)
				p[0] = 0;
		}
		
	}

	return m_pLocalDeviceName;
}

void CWinNetwork::SetLocalDeviceName(char* name)
{
	strcpy(m_pLocalDeviceName, name);
}

bool CWinNetwork::IsConnectionAvailable()
{
	return true;
}

void CWinNetwork::CleanUp()
{
	for (int i = 0; i < DEVICE_MAX; i++)
	{
	    if( DataSocket[i] != INVALID_SOCKET )
	    {
		    closesocket(DataSocket[i]);
		    DataSocket[i] = INVALID_SOCKET;
		}
	}
    
    if( ListenSocket != INVALID_SOCKET )
    {
	    closesocket(ListenSocket);
	    ListenSocket = INVALID_SOCKET;
	}
    
    if( DiscoverSocket != INVALID_SOCKET )
    {
	    closesocket(DiscoverSocket);
	    DiscoverSocket = INVALID_SOCKET;
	}

	WSACleanup();
}

void CWinNetwork::RemoveDevice(unsigned int deviceId)
{
	if ((m_pDevices[deviceId]) && (m_pDevices[deviceId]->iAddress))
	{
		// free address 
		// freeaddrinfo((addrinfo *)m_pDevices[deviceId]->iAddress);
		SAFE_DELETE(m_pDevices[deviceId]->iAddress);
		m_pDevices[deviceId]->iAddress = NULL;
	}

	Comms::RemoveDevice(deviceId);
}

// destructor
CWinNetwork::~CWinNetwork()
{
	ClearDevicesList();
	CleanUp();
	debug_out("CWinNetwork:: destructor");
}

#endif // HAS_MULTIPLAYER && WIN32
