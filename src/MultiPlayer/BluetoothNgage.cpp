#include "bluetoothNgage.h"
#ifdef HAS_BLUETOOTH_NGAGE
/*****************************************************************************************************
*
*This class is derivated from Comms class which will do the communication, this class will be only
*the transport layer for Ngage. One interesting thing is that in the Ngage architecture we do not
*have a normal client - server architecture - in Ngage we have the client which advertise his services
*and the server connects anyone who knows the service it is looking for
*Because of this we set isConnected to true only after server and client exchanged their names
*
*VERY IMPORTANT DO NOT CREATE BLUETOOTH OBJECT TWICE - EVEN IF WE DELETE THE FIRST ONE - IT MAY CRUSH
*ON SDK 1.1.2 - NOKIA BUG
****************************************************************************************************/




#include <wchar.h>          // libc ( swprintf )
#include <string.h>
#include <stdlib.h>
#include <runtime.h>
#include <input.h>
#include <errorcodes.h>
#include <standardtypes.h>
#include <ngiafbluetooth.h>
#include <ngiarenaframework.h>

using namespace ngi;

#include "hg/HighGear.h"
#include "Menu/DemoMenu.h"

// ---------------------------------------------------------
// CBluetoothPage()
// Default constructor.
// ---------------------------------------------------------
//

//initializations
CBluetoothNgage::CBluetoothNgage()
{

	mBluetooth = NULL;
	mBTDeviceDataList.Reset();
	mDeviceDataList = NULL;
	isConnected = 0;
	discoveredDev = 0;
	discoveredServ = 0;
//intiial state
	state = IDLE;
	devicesAnswered = 0;

	queueHead = 0;
	queueTail = 0;
	m_iConnectionType = CONNECTION_BLUETOOTH;


}


// ---------------------------------------------------------
// Initialize()
// Initialize the playback page.
// ---------------------------------------------------------
//
bool CBluetoothNgage::Initialize()
{
	return TRUE; 
}

// ---------------------------------------------------------
// CreateBluetoothInterface()
// create the bluetooth interface
// ---------------------------------------------------------
//
ReturnCode CBluetoothNgage::CreateBluetoothInterface(IArenaFramework* aArena)
{
	ReturnCode retVal = aArena->CreateBluetoothService(mBluetooth, *this );
	
	debug_out("mBluetooth: %p, ret: %d", mBluetooth, retVal);

	return retVal;
}

//from here we will comment only the callbacks which were useful for this implementation


//INAFBluetoothObserver
void CBluetoothNgage::EchoReceived(INAFBTMsgData& aMsgData) NO_THROW
{
	debug_out("FILE %s LINE %d EchoReceived\n",__FILE__,__LINE__);
}

void CBluetoothNgage::GameTextReceived(INAFBTMsgData& aMsgData) NO_THROW
{
	debug_out("FILE %s LINE %d GameTextReceived\n",__FILE__,__LINE__);
}

void CBluetoothNgage::PrivateGameTextReceived(INAFBTMsgData& aMsgData) NO_THROW
{
	debug_out("FILE %s LINE %d PrivateGameTextReceived\n",__FILE__,__LINE__);
}

//we just received data from server - so we are the client
//the server receives data via GameDataToServerReceived
void CBluetoothNgage::GameDataReceived(INAFBTMsgData& aMsgData) NO_THROW
{
	debug_out("FILE %s LINE %d GameDataReceived\n",__FILE__,__LINE__);

	uint32 len=0;
	const uint8* data = aMsgData.Data(len);
	debug_out("FILE %s LINE %d GameDataReceived %d state%d\n",__FILE__,__LINE__,len,state);
	if (!isServer)
	{
//if we are waiting for server name, we add the server
		if (state == WAITING_FOR_HOST_NAME)
		{
			DeviceDetails* dd = NEW DeviceDetails();

			sprintf(dd->iName,"%s", data );
			debug_out("FILE %s LINE %d GameDataReceived %s\n",__FILE__,__LINE__,dd->iName);
			AddDevice(dd);	
			strcpy(DemoMenu::GetInstance()->availableServers[0],dd->iName);
			state = CONNECTED;
			isConnected = 1;
		}
//we are connected and we pass the received data to internal buffers of Comms class
		if (state == CONNECTED && isConnected)
		{
			//if ((queueTail + 1) % MAX_BUFFERS == queueHead)
			{
				//there is no storage place for this buffer
				//debug_out("ERROR no more buffers for receive data\n");
			}
			//else
			{
				//queueTail = (queueTail + 1) % MAX_BUFFERS;
				//memcpy(dataBuff[queueTail],data,len);
				//lengths[queueTail] = len;
				OnDataRecv((unsigned char *)data, len, 0);
			}
		}
	}
//	else


}

void CBluetoothNgage::PrivateGameDataReceived(INAFBTMsgData& aMsgData) NO_THROW
{
	debug_out("FILE %s LINE %d PrivateGameDataReceived\n",__FILE__,__LINE__);
}
//the server received data
void CBluetoothNgage::DataToServerReceived(INAFBTMsgData& aMsgData) NO_THROW
{
	debug_out("FILE %s LINE %d DataToServerReceived state = %d\n",__FILE__,__LINE__,state);

	uint32 len=0;
	const uint8 * data = aMsgData.Data(len);

	{
//we received the client name, so we add it
		if (state != CONNECTED && !isConnected)
		{
			debug_out("FILE %s LINE %d DataToServerReceived\n",__FILE__,__LINE__);
			//if (len > 0)
			{
				DeviceDetails * dd = NEW DeviceDetails();

				sprintf(dd->iName,"%s", data );
				
				debug_out("FILE %s LINE %d DataToServerReceived %s\n",__FILE__,__LINE__,dd->iName);
				
				//AddDevice(dd);
				devicesAnswered++;
				Connected(0);
				CHighGear::GetInstance()->ConfirmRemoteDevice(dd);
			}
			if (devicesAnswered == discoveredServ)
			{
//we go to state to send our name to clients				
				state = SENDING_HOST_NAME;
				//isConnected = 1;
			}
		}
//we are connected - we just pass the data to Comms buffers
		if (state == CONNECTED && isConnected)
		{
			debug_out("FILE %s LINE %d taking message\n",__FILE__,__LINE__);
			//if ((queueTail + 1) % MAX_BUFFERS == queueHead)
			{
				//there is no storage place for this buffer
			//	debug_out("ERROR no more buffers for receive data\n");
			}
			//else
			{
				debug_out("FILE %s LINE %d taking message 2 %d %d\n",__FILE__,__LINE__,queueTail,queueHead);
			//	queueTail = (queueTail + 1) % MAX_BUFFERS;
			//	memcpy(dataBuff[queueTail],data,len);
			//	lengths[queueTail] = len;
				OnDataRecv((unsigned char *)data, len, 0);
			}	
		}
	
	}

}

void CBluetoothNgage::HostClientDisconnectCallback(uint32 aClientId) NO_THROW
{	
	debug_out("FILE %s LINE %d HostClientDisconnectCallback\n",__FILE__,__LINE__);
}
//the client just started
void CBluetoothNgage::StartSlaveCallback(void) NO_THROW
{
	debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
	state = START_CONNECTING;
}

//calbback for discovering devices, after we discover devices we have to discover services
void CBluetoothNgage::DiscoverDevicesCallback(INAFBTDeviceDataList* aDeviceDataList) NO_THROW
{           		
	mDeviceDataList = aDeviceDataList;

	int32 count = aDeviceDataList->Count();
	discoveredDev = count;
	if (discoveredDev == 0)
	{
		if (state == IDLE)
		{
			Disconnect();
		}
		else
		{
			mBluetooth->StopDiscoverDevices();
		}
		mBluetooth->DiscoverDevices();
	}
	debug_out("FILE %s LINE %d devices %d\n",__FILE__,__LINE__,count);
}
//callback for discover services
void CBluetoothNgage::DiscoverServicesCallback(INAFBTDeviceDataList* aDeviceDataList) NO_THROW
{	
	mDeviceDataList = aDeviceDataList;
	int32 count = aDeviceDataList->Count();
	debug_out("FILE %s LINE %d services %d\n",__FILE__,__LINE__,count);
	discoveredServ = count;
	if (discoveredServ == 0)
	{
		Disconnect();
		mBluetooth->DiscoverDevices();
	}
}
//callback for connection succeed
void CBluetoothNgage::ConnectDevicesCallback(void) NO_THROW
{
	debug_out("FILE %s LINE %d devices connected\n",__FILE__,__LINE__);
	//isConnected = 1;
	if (isServer)
		state = WAITING_FOR_HOST_NAME;

}
//the client was just connected
void CBluetoothNgage::SlaveConnectedCallback() NO_THROW
{
	debug_out("FILE %s LINE %d slave connected\n",__FILE__,__LINE__);
	state = SENDING_CLIENT_NAME;
}
//the client was just disconnected
void CBluetoothNgage::SlaveDisconnectedCallback() NO_THROW
{
	debug_out("FILE %s LINE %d SlaveDisconnectedCallback\n",__FILE__,__LINE__);
	isConnected = 0;
}

void CBluetoothNgage::HostReceiveDiscoverDeviceCallback(INAFBTDeviceData* aDeviceData) NO_THROW
{
	debug_out("FILE %s LINE %d HostReceiveDiscoverDeviceCallback\n",__FILE__,__LINE__);
}

void CBluetoothNgage::HostReceiveDiscoverServiceCallback(INAFBTDeviceData* aDeviceData) NO_THROW
{
	debug_out("FILE %s LINE %d HostReceiveDiscoverServiceCallback\n",__FILE__,__LINE__);
}


void CBluetoothNgage::ProgressReport( EArenaObserverState aState ) NO_THROW
{
	debug_out( "state: %d", aState );
}

	// update
bool CBluetoothNgage::update()
{
	/*if( CHighGear::GetInstance()->mAfterInterrupt )
	{
		mBluetooth->Release();
		mBluetooth = NULL;
		ReturnCode retVal = CApplication::GetArenaObject()->ArenaFramework()->CreateBluetoothService(mBluetooth, *this );
		debug_out("mBluetooth: %p, ret: %d", mBluetooth, retVal);
	}*/

	//if we are the server
	if (isServer)
	{
		//if we discovered devices,start discover services
		if (state == START_DISCOVERING_DEVICES && discoveredDev)
		{
			debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
			if ( mBluetooth  )
			{
				debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
				mBluetooth->DiscoverServices(KBT_serviceID);
				state = START_DISCOVERING_SERVICES;
			}
		}
		//we discovered devices which know the services we are looking for
		//so connect
		if (state == START_DISCOVERING_SERVICES && discoveredServ )
		{
			debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
			if ( mBluetooth  )
			{
				debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
				mBluetooth->ConnectDevices(mDeviceDataList);
				state = START_CONNECTING;
			}			
		}

		//send the host name
		if (state == SENDING_HOST_NAME)
		{
			char16 localname[64];
			char   localnamechar[64];
			localname[0] = 0;
			
			for (int i = 0 ; i < 10; i++)
			{
				if (!localname[0])
					mBluetooth->GetLocalName(localname);
				else
					break;
			}
			if (!localname[0])
			{
				localname[0] = 'a';
				localname[1] = 0;
			}

			for (int i = 0 ; i < wcslen (localname) ; i++)
				localnamechar[i] = (char)localname[i];

			localnamechar[wcslen (localname)] = 0;

			mBluetooth->SendGameData((const uint8*)localnamechar, strlen(localnamechar)+1);
			debug_out("FILE %s LINE %d sent host name %s\n",__FILE__,__LINE__,localnamechar);
			state = CONNECTED;
			isConnected = 1;
			return;

		}
/*		if (state == CONNECTED)
		{
			if (queueTail != queueHead)
			{
				//debug_out("FILE %s LINE %d read message %d %d\n",__FILE__,__LINE__,queueTail,queueHead);
				//queueHead = (queueHead + 1) % MAX_BUFFERS;
				//bool val = OnDataRecv(dataBuff[queueHead], lengths[queueHead], 0);
				//debug_out("FILE %s LINE %d OnDataRecv = %d\n",__FILE__,__LINE__,val);
			}
		}*/
	}
	else
	{
		//send the client name to server
		if ( state == SENDING_CLIENT_NAME)
		{
			char16 localname[64];
			char   localnamechar[64];
			localname[0] = 0;
			
			for (int i = 0 ; i < 10; i++)
			{
				if (!localname[0])
					mBluetooth->GetLocalName(localname);
				else
					break;
			}
			if (!localname[0])
			{
				localname[0] = 'a';
				localname[1] = 0;
			}

			for (int i = 0 ; i < wcslen(localname) ; i++)
				localnamechar[i] = (char)localname[i];

			localnamechar[wcslen(localname)] = 0;

			debug_out("FILE %s LINE %d DataToServer = %s\n",__FILE__,__LINE__,localnamechar);
			mBluetooth->SendGameDataToServer((const uint8*)localnamechar, strlen(localnamechar)+1);

			
			state = WAITING_FOR_HOST_NAME;
			//isConnected = 1;
		}
/*		if (state == CONNECTED && isConnected)
		{
			if (queueTail != queueHead)
			{
				queueHead = (queueHead + 1) % MAX_BUFFERS;
				//OnDataRecv(dataBuff[queueHead], lengths[queueHead], 0);

			}
		}*/
	}
}

    // hosts search
bool CBluetoothNgage::DiscoverServers() // fills the device list with servers
{
	return true;
}

    // connections
bool CBluetoothNgage::StartServer()
{
//make sure everything is stopped and start discover devices
	debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
	Disconnect();
	debug_out("FILE %s LINE %d bl=%p\n",__FILE__,__LINE__,mBluetooth);
	mBluetooth->DiscoverDevices();
	state = START_DISCOVERING_DEVICES;

	return true;
}

void CBluetoothNgage::StopListening()
{
//do not discover devices and services anymore
	if (mBluetooth)
	{
		Disconnect();
		//mBluetooth->StopDiscoverServices();
		//mBluetooth->StopDiscoverDevices();
	}
}

bool CBluetoothNgage::StartClient()
{
//stop everything and start advertise our service
	Disconnect();
	debug_out("FILE %s LINE %d %p\n",__FILE__,__LINE__,mBluetooth);
	mBluetooth->StartSlave(KBT_serviceID);
	state = STARTS_SLAVE;
	debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
	

}

    // send/receive data
bool CBluetoothNgage::SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId)
{
//depending if we are the server or client send the data using apropriate function
	if (isServer)
	{
		if (isConnected)
		{
			debug_out("SERVER send data\n");
			mBluetooth->SendGameData((const uint8*)data, dataLen);	
		}
	}
	else
	{
		
		ReturnCode code = mBluetooth->SendGameDataToServer((const uint8*)data, dataLen);	
		debug_out("CLIENT send data %d\n",code);
	}
}

    // local device name - convert from UNICODE to ASCII
char* CBluetoothNgage::GetLocalDeviceName()
{
	char16 localname[64];
	memset(localname,0,64*sizeof(char16));
	memset(m_pLocalDeviceName,0,LOCAL_DEVICE_NAME_SIZE);
	
	for (int i = 0 ; i < 10; i++)
	{
		if (!localname[0])
			mBluetooth->GetLocalName(localname);
		else
			break;
	}
	if (!localname[0])
	{
		localname[0] = 'a';
		localname[1] = 0;
	}

	debug_out("FILE %s LINE %d %p %s\n",__FILE__,__LINE__,mBluetooth,localname);
	for (int i = 0 ; i < wcslen (localname) ; i++)
		m_pLocalDeviceName[i] = (char)localname[i];

	m_pLocalDeviceName[wcslen (localname)] = 0;
	



	return m_pLocalDeviceName;
}

void CBluetoothNgage::SetLocalDeviceName(char* name)
{

}

	// connection
bool CBluetoothNgage::Connect(int serverIdx)
{
	if(!isServer)
	{
		isConnected = 0;//it was already connected but because this was the protocol implemented we have 
						//to wait until it validates server
		return 1;
	}
	return 1;
}

void CBluetoothNgage::Disconnect(/*int*/) // rax
{
//depending on how far we went with the connection do the reverse steps
	if (isServer)
	{
		debug_out("FILE %s LINE %d SERVER state %d\n",__FILE__,__LINE__,state);
		if (state >= START_CONNECTING)
		{
			debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
			mBluetooth->DisconnectDevices();
		}
		debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
		if (state >= START_DISCOVERING_SERVICES)
		{
			debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
			mBluetooth->StopDiscoverServices();
		}
		debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
		if (state >= START_DISCOVERING_DEVICES)
		{
			debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
			mBluetooth->StopDiscoverDevices();
		}

		
	}
	else
	{
		debug_out("FILE %s LINE %d CLIENT state %d\n",__FILE__,__LINE__,state);
		if (state >= START_CONNECTING)
			mBluetooth->DisconnectDevices();
		debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
		/*if(state == STARTS_SLAVE)
			mBluetooth->StopSlave();*/
		debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
	}
	debug_out("FILE %s LINE %d\n",__FILE__,__LINE__);
	isConnected = 0;
	state = IDLE;
	queueTail = 0;
	queueHead = 0;

	mBTDeviceDataList.Reset();
	mDeviceDataList = NULL;
	discoveredDev = 0;
	discoveredServ = 0;
	state = IDLE;
	devicesAnswered = 0;

}


bool CBluetoothNgage::IsConnectionAvailable()
{
	Comms::IsConnectionAvailable();
}

	// power
bool CBluetoothNgage::GetPowerState()
{
	if(!mBluetooth) return false;
	char16* localname = new char16[64];
	memset(localname,0,64*sizeof(char16));
	mBluetooth->GetLocalName(localname);
	bool bl = localname[0]!=0;
	SAFE_DEL(localname);
	return bl;
	//return localname[0]!=0;
	//return mBluetooth != NULL;
}

void CBluetoothNgage::SetPowerState(bool state)
{
	return ;
}

//because we must not make another bluetooth object, we gonna use a reinitialize function instead of the
//destructor - we will delete the bluetooth object only at the end of the application
void CBluetoothNgage::Reinitialize()
{
	debug_out("FILE %s LINE %d REINITING\n",__FILE__,__LINE__);
//do a disconnect
	if( mBluetooth )
	{
		Disconnect();

		//mBluetooth->Release();
		//mBluetooth = NULL;		
	}

	//ReturnCode retVal = CApplication::GetArenaObject()->ArenaFramework()->CreateBluetoothService(mBluetooth, *this );
	//debug_out("mBluetooth: %p, ret: %d", mBluetooth, retVal);

	//reset any registered devices
	for(int i=0; i<m_iDevicesCount; i++)
	{
		if (m_pDevices[i]->iAddress)
			SAFE_DELETE((INAFBTDeviceData*)(m_pDevices[i]->iAddress));
	}
	ClearDevicesList();

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	RemoveDeviceNamePrefix();
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING
	SAFE_DELETE(m_pLocalDeviceName);
	SAFE_DELETE(m_pPrefix);

//reinitialize everything
	isConnected = false;
    isServer	= false;

	queue_start = queue_end = 0;
	m_iDevicesCount		= 0;
	m_iDevicesIdx		= 0;
	m_iDevicesConnected = 0;

	m_pLocalDeviceName = NEW char[LOCAL_DEVICE_NAME_SIZE];
	memset(m_pLocalDeviceName, 0, LOCAL_DEVICE_NAME_SIZE);

	memset(m_bDevicesCnx, 0, DEVICE_MAX * sizeof(bool));

	m_iConnectionType = CONNECTION_BLUETOOTH;

	for (int i=0; i<DEVICE_MAX; i++)
		m_pDevices[i] = NULL;

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	m_bHostnamePrefixed = false;
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING

	m_pPrefix = NEW char[10];
	sprintf(m_pPrefix, "%04d", KUidGame & 0xFFFF);
	m_pPrefix[4] = 0;


	mBTDeviceDataList.Reset();
	mDeviceDataList = NULL;
	isConnected = 0;
	discoveredDev = 0;
	discoveredServ = 0;
	state = IDLE;
	devicesAnswered = 0;

	queueHead = 0;
	queueTail = 0;


}

CBluetoothNgage::~CBluetoothNgage()
{
//only at the end of the application distroy mBluetooth - first disconnect and after that release it
	Comms::~Comms();
	if( mBluetooth )
	{
		Disconnect();
		mBluetooth->Release();
		mBluetooth = NULL;		
	}

	for(int i=0; i<m_iDevicesCount; i++)
	{
		if (m_pDevices[i]->iAddress)
			SAFE_DELETE((INAFBTDeviceData*)(m_pDevices[i]->iAddress));
	}
	ClearDevicesList();

}

/*int Comms::AddDevice(DeviceDetails* d)
{
	int pos = Comms::AddDevice(d);

}*/

#endif //BLUETOOTH_NGAGE
