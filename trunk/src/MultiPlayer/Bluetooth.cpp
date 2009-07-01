
#include "MultiPlayer/Bluetooth.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include <stdio.h>
#include <string.h>

#ifdef __WIN32__
#include <memory.h>
#endif // __WIN32__

#include "memoryallocation.h"
#include "MultiPlayer/Bluetooth.h"
#include "MultiPlayer/Comms.h"

#include "bluetooth/AsyncBase.h"
#include "bluetooth/AsyncAccept.h"
#include "bluetooth/AsyncDisc.h"
#include "bluetooth/Series60Comms.h"

class DeviceDetails;
class TDeviceDetails;

void TxtUniConv( char* in, int in_len, char* out );
int	 UniTxtConv( char* in, char* out );

TDeviceDetails DeviceDetailsToTDeviceDetails(DeviceDetails devDetails);

// constructor
CBluetooth::CBluetooth() : Comms()
{
	debug_out("CBluetooth::CBluetooth()");
	iComms	  = NEW Series60Comms(this);
	iResolver = NEW CAsyncDisc(this);

	m_iConnectionType = CONNECTION_BLUETOOTH;
}

void CBluetooth::Reinitialize()
{
	// from comms constructor
	isConnected = false;
    isServer	= false;

	queue_start = queue_end = 0;
	m_iDevicesCount			= 0;
	m_iDevicesIdx			= 0;
	m_iDevicesConnected		= 0;

	if (m_pLocalDeviceName)
		memset(m_pLocalDeviceName, 0, LOCAL_DEVICE_NAME_SIZE);

	memset(m_bDevicesCnx, 0, DEVICE_MAX * sizeof(bool));

	for (int i=0; i<DEVICE_MAX; i++)
		m_pDevices[i] = NULL;

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	m_bHostnamePrefixed = false;
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING
	
}

bool CBluetooth::DiscoverServers()
{
	iResolver->Cancel();
	iResolver->startDiscovering();

	return true;
}

bool CBluetooth::DiscoveryFinished()
{
	return !iResolver->inProgress;
}

//--------------------------------------------------------
//--------------------------------------------------------

void CBluetooth::FoundDevice(/*const*/ DeviceDetails *pDevice, /*TInt*/int aError)
{
	debug_out("\n CBluetooth::FoundDevice ...");

	if (KErrNone == aError)
	{
		// does it have the prefix  ?
#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
		if(MatchDeviceNamePrefix(*pDevice))
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING
		{
			debug_out("\n CBluetooth::FoundDevice - MATCH PREFIX OK");
			AddDevice(pDevice);		
		}
		else
		{
			debug_out("\n CBluetooth::FoundDevice PREFIX NOT OK !");
		}
	}
	else if (KErrEof ==  aError)
	{
		// Search complete!
		CHighGear::GetInstance()->device_to_confirm_nb = 0;

		// cancel search
		if (iResolver)
			iResolver->Cancel();
	}
}

bool CBluetooth::StartServer()
{
	iComms->StartHostL();
}

bool CBluetooth::StartClient()
{
	iComms->StartClientL();
}

// send/receive data
bool CBluetooth::SendData(unsigned char* data, unsigned int  dataLen, unsigned char clientId /*= CLIENT_ID_ALL*/)
{
	A_ASSERT( dataLen <= DATA_BUFF_SIZE );
	A_ASSERT( clientId < m_iDevicesCount );

	TBuf8<DATA_BUFF_SIZE>	aData;

	aData.Copy( (TUint8*)data, (TInt)(dataLen) );

	if (clientId == CLIENT_ID_ALL)
	{
		iComms->SendData( aData );

		if (KErrNone == iComms->SendData( aData ))
		{
			return true; // (0);
		}
		else
		{
			return false;// (1);
		}
	}
	else
	{
		// iComms->SendData( (TInt)clientId, aData );

		if (KErrNone == iComms->SendData( (TInt)clientId, aData ))
		{
			return true;// (0);
		}
		else
		{
			return false;// (1);
		}
	}	
}

//--------------------------------------------------------
//--------------------------------------------------------

// local device name
char * CBluetooth::GetLocalDeviceName()
{
	if (m_pLocalDeviceName[0] == 0)
	{
		THostName n;

		// get BT name (try many times, because sometime it doesn't work !
		for (int i = 0; i < 10; i++)
		{
			iComms->GetLocalDeviceName(n);
			int	len = (int)n.Length();

			TxtUniConv( (char*)n.Ptr(), len, m_pLocalDeviceName );
			if (m_pLocalDeviceName[0] != 0)
				break;
		}
	}

	return m_pLocalDeviceName;
}

void CBluetooth::SetLocalDeviceName(char* name)
{
	debug_out("\n CBluetooth::SetLocalDeviceName = %s", name);

	// convert name to THostName ...	
	THostName hName;//(name);

	int length = strlen(name);
	char* out = NEW char[length << 1];
	UniTxtConv(name, out);

	hName.Copy((TUint16*)out, length);

	iComms->SetLocalDeviceName(hName);

	strcpy(m_pLocalDeviceName, name);

	debug_out("\n ... CBluetooth::SetLocalDeviceName m_pLocalDeviceName = %s", m_pLocalDeviceName);
}

//--------------------------------------------------------
//--------------------------------------------------------

bool CBluetooth::IsConnectionAvailable()
{
	debug_out("+++++++++++---------------->>>>> CHighGear::IsBTConnectionAvailable 00\n");
	__UHEAP_MARK;

	_LIT( KL2CAPSTR, "L2CAP" );

	RSocketServ socketServer;
	RSocket socket;
	TInt error = KErrNone;
	TPckgBuf <TInt> linkCountBuffer;
	TBool aStatus = ETrue;

	// connect to socket server
	error = socketServer.Connect();
	if (KErrNone == error)
	{
		// Find BT L2CAP protocol
		TProtocolDesc protDesc;
		error = socketServer.FindProtocol( STATIC_CAST(TProtocolName, KL2CAPSTR() ) , protDesc );

		if (KErrNone == error)
		{
			// Open BT Socket
			error = socket.Open(socketServer, protDesc.iAddrFamily, protDesc.iSockType, protDesc.iProtocol);
			if (KErrNone == error)
			{
				// Get link count from socket via GetOpt

#ifdef SYMBIAN8 // rov - needed ? - removed on 91 - error -18: KErrNotReady
				error = socket.GetOpt( KLMGetACLLinkCount, KSolBtLM, linkCountBuffer );
				if ( KErrNone == error)
				{
					// is link count != 0
					TInt linkCount = linkCountBuffer();
					if (linkCount)
					{
						aStatus = EFalse;
					}
				}
#endif // SYMBIAN8

				// Close socket
				socket.Close();
			}
		}
		// Close socket server
		socketServer.Close();
	}


	if (KErrNone != error)
	{
		debug_out( "Error in IsConnectionAvailable: %d", error );
		User::Leave(error);
	}

	__UHEAP_MARKEND;
	debug_out( "IsConnectionAvailable: %d", aStatus );

	return aStatus;
}

//--------------------------------------------------------
//--------------------------------------------------------

bool CBluetooth::Connect(int serverIdx)
{
	isConnecting = true;

	// stop searching
	iResolver->Cancel();

	// connect to device
	iComms->Connect(serverIdx);
}

void CBluetooth::Disconnect(int iDeviceID)
{
	if( iResolver )
		iResolver->Cancel();

	if( iComms )
		iComms->Disconnect();

	Comms::Disconnect(iDeviceID);
}

//--------------------------------------------------------
//--------------------------------------------------------

/*void CBluetooth::Disconnected(TInt aDeviceID)
{
	if (m_iDevicesConnected > 0)
		m_iDevicesConnected--;

	if (!isServer)
	{
		isConnected = false;	// if client, means no connection to server anymore
	}
	else
	{
		// if server, dec connexion nb
		if (m_iDevicesConnected == 0)
			isConnected = false;	// if cx nb == 0, means no client anymore

		// set connexion state of the device
		m_bDevicesCnx[aDeviceID] = false;
	}
}*/

void CBluetooth::StopListening()
{
	debug_out("\n CBluetooth::StopListening() ...");

	// iComms->StopListening();

	// also remove the name of the server -> no other client see it as available
	RemoveDeviceNamePrefix();

	debug_out("\n ... CBluetooth::StopListening()");
}

//--------------------------------------------------------
//--------------------------------------------------------

bool CBluetooth::GetPowerState()
{
	// check the BT state	
	TBool	state;

	debug_out("\n CBluetooth::GetPowerState(), iComms: %p", iComms);

	iComms->BluetoothPowerState( state );	


	if (state)
		return true;

	return false;	
}	

void CBluetooth::SetPowerState(bool state)
{
	// check the BT state	
	TBool	crtState;
	TBool   newState = EFalse;

	if (state)
		newState = ETrue;

	iComms->BluetoothPowerState(crtState);
	if (crtState != newState)							// bluetooth disabled
		iComms->SetBluetoothPowerState(newState);		// turn on bluetooth (security)
}



// removes the DeviceDetails ... and also frees it !
// 
void CBluetooth::RemoveDevice(unsigned int deviceId)
{
	debug_out("\n CBluetooth::RemoveDevice() ... ");


	if ((m_pDevices[deviceId]) && (m_pDevices[deviceId]->iAddress))
	{
		MM_DELETE((TBTSockAddr*)m_pDevices[deviceId]->iAddress);
		m_pDevices[deviceId]->iAddress = NULL;

	}

	Comms::RemoveDevice(deviceId);
}


CBluetooth::~CBluetooth()
{
	debug_out("CBluetooth::~CBluetooth()");

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	RemoveDeviceNamePrefix();
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING	

	if (iComms)
		iComms->Disconnect();

	SAFE_DELETE(iComms);
	SAFE_DELETE(iResolver);	

	ClearDevicesList();
}

#endif // HAS_BLUETOOTH
