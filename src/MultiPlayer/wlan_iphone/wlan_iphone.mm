
#include "wlan_iphone.h"

#if defined(IPHONE) && defined(HAS_MULTIPLAYER) && defined(HAS_WLAN)

#include <unistd.h>
#include "memoryallocation.h"

#undef WIN32
#import "MultiPlayer/wlan_iphone/NetReachability.h"


//`void debug_out(const char* x, ...);
extern "C" unsigned long OS_GetTime(void);

#ifndef USE_FANCY_MENUS
	#include "hg/HighGear.h"
#endif

// constructor
CWlanIPhone::CWlanIPhone()
{
	//debug_out("CWlanIPhone:: constructor");

	ListenSocket = INVALID_SOCKET;
	DiscoverSocket = INVALID_SOCKET;
	for (int i = 0; i < DEVICE_MAX; i++)
		DataSocket[i] = INVALID_SOCKET;

	state = STATE_NONE;
}

// update
bool CWlanIPhone::update()
{
	switch(state)
	{
		case STATE_ACCEPT:
		{
			fd_set setR;
			timeval tval = {0, 0};
			int nfds = 0;
			FD_ZERO(&setR);
			FD_SET(ListenSocket, &setR);
			nfds = nfds < ListenSocket ? ListenSocket : nfds;
			FD_SET(DiscoverSocket, &setR);
			nfds = nfds < DiscoverSocket ? DiscoverSocket : nfds;
			nfds++;
			
			int iResult = select(nfds, &setR, NULL, NULL, &tval);
			if (iResult == SOCKET_ERROR)
			{
				//debug_out("CWlanIPhone::update accept: error=%d\n", errno);
				CleanUp();
				return false;
			}
			else if (iResult == 0)
			{
				break;
			}

			if (FD_ISSET(ListenSocket, &setR))
			{
				//debug_out("CWlanIPhone::update listen 1\n");
				struct sockaddr addr;
				socklen_t addrLen = sizeof(struct sockaddr);
#ifdef USE_FANCY_MENUS
				int devIdx = m_iDevicesCount;
#else
				int devIdx = CHighGear::GetInstance()->device_to_confirm_nb;
#endif

				// Accept a client socket
				DataSocket[devIdx] = accept(ListenSocket, &addr, &addrLen);
				if (DataSocket[devIdx] == INVALID_SOCKET)
				{
					//debug_out("CWlanIPhone::AcceptLoop accept failed: %d\n", errno);
					CleanUp();
					return false;
				}

				//debug_out("CWlanIPhone::update listen 2\n");

				DeviceDetails* dd = NEW DeviceDetails();

				dd->iAddress = NEW char[sizeof(struct sockaddr)];
				memcpy(dd->iAddress, &addr, sizeof(struct sockaddr));

//				if (getnameinfo(&addr, addrLen, dd->iName, sizeof(dd->iName), NULL, 0, 0) != 0)
//				{
//					//debug_out("CWlanIPhone::Error when retrieving client name\n");
//					//CleanUp();
//					//return false;
//				}
				
				// we must receive the host name
				iResult = recv(DataSocket[devIdx], (char *)dataBuff, DATA_BUFF_SIZE, 0);
				if (iResult == -1)
				{
					//debug_out("CWlanIPhone::DeviceReceive: recv error %d\n", errno);
					CleanUp();
					return false;
				}
				
				iResult = iResult < sizeof(dd->iName) ? iResult : sizeof(dd->iName) - 1;
				
				memcpy(dd->iName, dataBuff, iResult);
				dd->iName[iResult] = 0; // adding null terminator

				//debug_out("CWlanIPhone::update listen 3\n");

				Connected(devIdx);
				
#ifdef USE_FANCY_MENUS
				AddDevice(dd);
#else
				CHighGear::GetInstance()->ConfirmRemoteDevice(dd);
#endif
			}

			if (FD_ISSET(DiscoverSocket, &setR))
			{
				//debug_out("CWlanIPhone::update discover 1\n");

				struct sockaddr addr;
				socklen_t addrLen = sizeof(struct sockaddr);

				int len = recvfrom(DiscoverSocket, (char*)dataBuff, DATA_BUFF_SIZE, 0, &addr, &addrLen);
				if (len == SOCKET_ERROR)
				{
					//debug_out("CWlanIPhone::Error when receiving data with udp socket: %d\n", errno);
					CleanUp();
					return false;
				}

				//debug_out("CWlanIPhone::update discover 2\n");

				// checking signature
				if (memcmp(&KUidGame, dataBuff, sizeof(KUidGame)) == 0)
				{
					//debug_out("CWlanIPhone::update discover 3\n");
					
					strcpy((char*)(dataBuff + sizeof(KUidGame)), m_pLocalDeviceName);

					// sending response to client
					if (sendto(DiscoverSocket, (char*)dataBuff, sizeof(KUidGame) + strlen(m_pLocalDeviceName), 0, &addr, addrLen) == SOCKET_ERROR)
					{
						//debug_out("CWlanIPhone::Error when responding using udp socket: %d\n", errno);
						CleanUp();
						return false;
					}

					//debug_out("CWlanIPhone::update discover 4\n");
				}
			}

			break;
		}

		case STATE_DISCOVER:
		{
			if (counter > 0 && timer < OS_GetTime())
			{
				timer = OS_GetTime() + DISCOVER_TIMER;
				counter --;
				
				// sending broadcast message
				struct sockaddr_in addr_Broadcast;
				
				bzero (&addr_Broadcast, sizeof(addr_Broadcast));
				addr_Broadcast.sin_family      = AF_INET;
				addr_Broadcast.sin_port        = htons(atoi(DISCOVER_PORT));
				addr_Broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);
				
				if (sendto(DiscoverSocket, &KUidGame, sizeof(KUidGame), 0, (struct sockaddr *)&addr_Broadcast, sizeof(addr_Broadcast)) == SOCKET_ERROR)
				{
					//debug_out("CWlanIPhone::Error when sending discover message: %d\n", errno);
					return false;
				}
				
				//debug_out("CWlanIPhone::update discover: sending broadcast message");
			}
			
			fd_set setR;
			timeval tval = {0, 0};
			FD_ZERO(&setR);
			FD_SET(DiscoverSocket, &setR);

			int iResult;
			iResult = select(DiscoverSocket + 1, &setR, NULL, NULL, &tval);
			if (iResult == SOCKET_ERROR)
			{
				//debug_out("CWlanIPhone::update discover: select error=%d\n", errno);
				CleanUp();
				return false;
			}
			else if (iResult == 0)
			{
				break;
			}

			struct sockaddr addr;
			socklen_t addrLen = sizeof(struct sockaddr);

			int len = recvfrom(DiscoverSocket, (char*)dataBuff, DATA_BUFF_SIZE, 0, &addr, &addrLen);
			if (len == SOCKET_ERROR)
			{
				//debug_out("CWlanIPhone::Error when receiving data with udp socket: %d\n", errno);
				return false;
			}

			// checking signature and if the device is not already added
			if ((memcmp(&KUidGame, dataBuff, sizeof(KUidGame)) == 0) && (FindDevice(&addr) < 0))
			{
				DeviceDetails* dd = NEW DeviceDetails();

				dd->iAddress = NEW char[sizeof(struct sockaddr)];
				memcpy(dd->iAddress, &addr, sizeof(struct sockaddr));
				
				memcpy(dd->iName, dataBuff + sizeof(KUidGame), len - sizeof(KUidGame));
				
				//debug_out("CWlanIPhone::Server name = %s\n", dd->iName);

				AddDevice(dd);
			}
			break;
		}

		case STATE_RECV_DATA:
		{
			unsigned char i;

			fd_set setR;
			timeval tval = {0, 0};
			int nfds = 0;
			FD_ZERO(&setR);
			for (i = 0; i < m_iDevicesCount; i++)
			{
				if (DataSocket[i] >= 0)
				{
					FD_SET(DataSocket[i], &setR);
					nfds = nfds < DataSocket[i] ? DataSocket[i] : nfds;
				}
			}
			nfds++;

			int iResult = select(nfds, &setR, NULL, NULL, &tval);
			if (iResult == SOCKET_ERROR)
			{
				//debug_out("CWlanIPhone::DeviceReceive: select error %d\n", errno);
				return false;
			}

		//	//debug_out("CWlanIPhone::DeviceReceive select returns : %d\n", iResult);
			if (iResult == 0)
				break;

			for (i = 0; i < m_iDevicesCount; i++)
			{
				if (DataSocket[i] >= 0 && FD_ISSET(DataSocket[i], &setR))
				{
					iResult = recv(DataSocket[i], (char *)dataBuff, DATA_BUFF_SIZE, 0);
					if (iResult == -1)
					{
						if (errno == ECONNRESET || errno == ETIMEDOUT)
							iResult = 0; // connection closed
						else
						{
							//debug_out("CWlanIPhone::DeviceReceive: recv error %d\n", errno);
							return false;
						}
					}

					if (iResult > 0)
					{
//						// ignoring disconnected devices
//						if (m_bDevicesCnx[i])
						{
							//debug_out("CWlanIPhone::OnDataRecv: type = %d len = %d clientId = %d", dataBuff[0], iResult, i);
							if (!OnDataRecv(dataBuff, iResult, i))
							{
								//debug_out("CWlanIPhone::Queue overflow !!!\n");
								return false;
							}
						}
					}
					else if (iResult == 0)
					{
						ConnectionClosedWith(i);
					}
				}
			}

			break;
		}
	}

	return true;
}

// hosts search
bool CWlanIPhone::DiscoverServers()
{
	//debug_out("CWlanIPhone::DiscoverServers\n");

	InitDiscover(false);
	
	timer = OS_GetTime();
	counter = DISCOVER_BROADCAST_COUNT;

	state = STATE_DISCOVER;

	return true;
}

// connections
bool CWlanIPhone::StartServer()
{
	//debug_out("CWlanIPhone::StartServer\n");

	isServer = true;

//	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
//	if (iResult != 0)
//	{
//		//debug_out("CWlanIPhone::StartHost WSAStartup failed: %d\n", iResult);
//		return false;
//	}

	InitDiscover(true);

	// Initialize listen socket
	struct addrinfo *result = NULL,	hints;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, LISTEN_PORT, &hints, &result);
	if ( iResult != 0 )
	{
		//debug_out("CWlanIPhone::StartHost getaddrinfo failed: %d\n", iResult);
		CleanUp();
		return false;
	}

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		//debug_out("CWlanIPhone::StartHost Error at socket(): %ld\n", errno);
		freeaddrinfo(result);
		CleanUp();
		return false;
	}

	int bOptVal = 1;
	
	if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, sizeof(int)) == SOCKET_ERROR)
	{
		//debug_out("CWlanIPhone::StartHost setsockopt failed: %d\n", errno);
		freeaddrinfo(result);
		CleanUp();
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		//debug_out("CWlanIPhone::StartHost bind failed: %d\n", errno);
		freeaddrinfo(result);
		CleanUp();
		return false;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		//debug_out("CWlanIPhone::StartHost Error at bind(): %ld\n", errno );
		CleanUp();
		return false;
	}

	state = STATE_ACCEPT;

	return true;
}

bool CWlanIPhone::InitDiscover(bool server)
{
	// Initialize discover socket
	int iResult;
	struct addrinfo *result = NULL, hints;

	bzero( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DISCOVER_PORT, &hints, &result);
	if ( iResult != 0 )
	{
		//debug_out("CWlanIPhone::InitDiscover getaddrinfo failed: %d\n", iResult);
		CleanUp();
		return false;
	}

	DiscoverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (DiscoverSocket == INVALID_SOCKET)
	{
		//debug_out("CWlanIPhone::InitDiscover Error at socket(): %ld\n", errno);
		freeaddrinfo(result);
		CleanUp();
		return false;
	}
	
	int bOptVal = 1;
	
	if (setsockopt(DiscoverSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, sizeof(int)) == SOCKET_ERROR)
	{
		//debug_out("CWlanIPhone::InitDiscover setsockopt failed: %d\n", errno);
		freeaddrinfo(result);
		CleanUp();
		return false;
	}
	
	if (server)
	{
		iResult = bind(DiscoverSocket, result->ai_addr, result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			//debug_out("CWlanIPhone::InitDiscover bind failed: %d\n", errno);
			freeaddrinfo(result);
			CleanUp();
			return false;
		}
	}
		
	freeaddrinfo(result);

	return true;
}

void CWlanIPhone::StopListening()
{
	//debug_out("CWlanIPhone::StopListening\n");

	CLOSE(ListenSocket);
	ListenSocket = INVALID_SOCKET;

	CLOSE(DiscoverSocket);
	DiscoverSocket = INVALID_SOCKET;

	state = STATE_RECV_DATA;
}

bool CWlanIPhone::StartClient()
{
	//debug_out("CWlanIPhone::StartClient\n");

	isServer = false;

	return true;
}

// connection
bool CWlanIPhone::Connect(int serverIdx)
{
	if (state!=STATE_DISCOVER)
		return false;

	if (isServer)
		return false;

	//debug_out("CWlanIPhone::Connect to server %d\n", serverIdx);

	struct sockaddr_in* addr;
	int iResult;

	addr = (struct sockaddr_in*)m_pDevices[serverIdx]->iAddress;

	addr->sin_port = htons(atoi(LISTEN_PORT));

	// clean
	for (int i = m_iDevicesCount - 1; i >= 0; i--)
	{
		CLOSE(DataSocket[i]);
		DataSocket[i] = INVALID_SOCKET;

		if (i != serverIdx)
			RemoveDevice(i);
	}

	CLOSE(DiscoverSocket);
	DiscoverSocket = INVALID_SOCKET;


	// Create a SOCKET for connecting to server
	DataSocket[0] = socket(addr->sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (DataSocket[0] == INVALID_SOCKET)
	{
		//debug_out("CWlanIPhone::Connect Error at socket(): %ld\n", errno);
		CleanUp();
		return false;
	}

	// Connect to server.
	iResult = connect(DataSocket[0], (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if (iResult == SOCKET_ERROR)
	{
		//debug_out("CWlanIPhone::Connect Error connect: %d\n", errno);
		CleanUp();
		return false;
	}
	
	iResult = send(DataSocket[0], m_pLocalDeviceName, strlen(m_pLocalDeviceName), 0);
	if (iResult == SOCKET_ERROR)
	{
		//debug_out("CWlanIPhone::Connect Error: send failed: %d\n", errno);
		CleanUp();
		return false;
	}

	Connected(0);

	state = STATE_RECV_DATA;
	
	return true;
}

void CWlanIPhone::Disconnect()
{
	//debug_out("CWlanIPhone::Disconnect\n");

	for (unsigned char i = 0; i < m_iDevicesCount; i++)
	{
		CLOSE(DataSocket[i]);
		DataSocket[i] = INVALID_SOCKET;
	}

	CLOSE(ListenSocket);
	ListenSocket = INVALID_SOCKET;

	CLOSE(DiscoverSocket);
	DiscoverSocket = INVALID_SOCKET;

	ClearDevicesList();
}

// send/receive data
bool CWlanIPhone::SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId)
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
	else if (DataSocket[clientId] >= 0)
	{
		//debug_out("CWlanIPhone::SendData: type = %d len = %d clientId = %d", data[0], dataLen, clientId);

		fd_set setW;
		FD_ZERO(&setW);
		FD_SET(DataSocket[clientId], &setW);

		int iResult = select(DataSocket[clientId] + 1, NULL, &setW, NULL, NULL);
		if (iResult == SOCKET_ERROR)
		{
			//debug_out("CWlanIPhone::DeviceSend: select error %d\n", errno);
			return false;
		}

		iResult = send(DataSocket[clientId], (const char *)data, dataLen, 0);
		if (iResult == SOCKET_ERROR)
		{
			if (errno == EPIPE || errno == ECONNRESET || errno == EHOSTUNREACH || errno == ENETDOWN || errno == ENETUNREACH)
			{
				ConnectionClosedWith(clientId);
			}
			else
			{
				//debug_out("CWlanIPhone::DeviceSend send failed: %d\n", errno);
			}
			return false;
		}
	}

	return true;
}

// local device name
char* CWlanIPhone::GetLocalDeviceName()
{
	if (m_pLocalDeviceName[0] == 0)
	{
		if (gethostname(m_pLocalDeviceName, LOCAL_DEVICE_NAME_SIZE) == SOCKET_ERROR)
		{
			//debug_out ("Error %d when getting local host name.", errno);
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

void CWlanIPhone::SetLocalDeviceName(char* name)
{
	strcpy(m_pLocalDeviceName, name);
}

///////////////////////////////////////////////////////////////////////////////
// power
///////////////////////////////////////////////////////////////////////////////

bool CWlanIPhone::GetPowerState()
{
	bool canConnect;
	NetReachability* reachability = [[NetReachability alloc] initWithDefaultRoute:NO];
	
	canConnect = [reachability isReachable] && ![reachability isUsingCell];
	
	[reachability release];
	
	return canConnect;
}

void CWlanIPhone::SetPowerState(bool state)
{
	
}

bool CWlanIPhone::IsConnectionAvailable()
{
	return true;
}

void CWlanIPhone::ConnectionClosedWith(unsigned int deviceId)
{
	if (DataSocket[deviceId] >= 0)
	{
		// connection closed
		if(m_iDevicesConnected != 0)
			m_iDevicesConnected--;

		if (m_iDevicesConnected == 0)
			isConnected = false;	

		// set connexion state of the device
		m_bDevicesCnx[deviceId] = false;

		CLOSE(DataSocket[deviceId]);
		DataSocket[deviceId] = INVALID_SOCKET;

		//debug_out("CWlanIPhone:: Connection closed by device %d\n", deviceId);
	}
}

void CWlanIPhone::CleanUp()
{
	for (int i = 0; i < DEVICE_MAX; i++)
	{
		CLOSE(DataSocket[i]);
		DataSocket[i] = INVALID_SOCKET;
	}

	CLOSE(ListenSocket);
	ListenSocket = INVALID_SOCKET;

	CLOSE(DiscoverSocket);
	DiscoverSocket = INVALID_SOCKET;
	
	ClearDevicesList();
	
	state = STATE_NONE;
}

void CWlanIPhone::RemoveDevice(unsigned int deviceId)
{
//	freeaddrinfo((struct addrinfo *)m_pDevices[deviceId]->iAddress);
//	m_pDevices[deviceId]->iAddress = NULL;

	Comms::RemoveDevice(deviceId);
}

int CWlanIPhone::FindDevice(void* address)
{
	int addrLen = ((struct sockaddr*)address)->sa_len;
	
	for (unsigned int i = 0; i < m_iDevicesCount; i++)
	{
		if (memcmp(address, m_pDevices[i]->iAddress, addrLen) == 0)
			return i;
	}
	
	return -1;
}

// destructor
CWlanIPhone::~CWlanIPhone()
{
	CleanUp();
	//debug_out("CWlanIPhone:: destructor");
}

#endif // HAS_MULTIPLAYER && IPHONE && HAS_WLAN
