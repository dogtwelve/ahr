////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Comms.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __WIN32__
#include <memory.h>
#endif // __WIN32__

#include <string.h>
#include <stdio.h>

#include "memoryallocation.h"
#include "Comms.h"
#include "DevUtil.h"

//DEBUG!!!!!!
//#include "HighGear.h"

#ifdef HAS_MULTIPLAYER

//SEFU 8 MOVED int Comms::KUidGame = HIGHGEAR_UID;

// constructor
Comms::Comms()
{
//SEFU 8
	KUidGame = HIGHGEAR_UID;

	isConnected = false;
    isServer	= false;

	queue_start = queue_end = 0;
	m_iDevicesCount		= 0;
	m_iDevicesIdx		= 0;
	m_iDevicesConnected = 0;

	m_pLocalDeviceName = NEW char[LOCAL_DEVICE_NAME_SIZE];
	memset(m_pLocalDeviceName, 0, LOCAL_DEVICE_NAME_SIZE);

	memset(m_bDevicesCnx, 0, DEVICE_MAX * sizeof(bool));

	m_iConnectionType = CONNECTION_NONE;

	for (int i=0; i<DEVICE_MAX; i++)
		m_pDevices[i] = NULL;

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	m_bHostnamePrefixed = false;
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING

	m_pPrefix = NEW char[10];
	sprintf(m_pPrefix, "%04d", KUidGame & 0xFFFF);
	m_pPrefix[4] = 0;
}

//--------------------------------------------------------
//--------------------------------------------------------

bool Comms::update()
{
	return true;
}

//--------------------------------------------------------
//--------------------------------------------------------

bool Comms::DiscoverServers()
{
	return true;
}

bool Comms::DiscoveryFinished()
{
	return false;
}

bool Comms::StartServer()
{
	return true;
}

void Comms::StopListening()
{
}

bool Comms::StartClient()
{
	return true;
}

//--------------------------------------------------------
//--------------------------------------------------------

bool Comms::SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId)
{
	return true;
}

//--------------------------------------------------------
//--------------------------------------------------------

char* Comms::GetLocalDeviceName()
{
	return m_pLocalDeviceName;
}

void Comms::SetLocalDeviceName(char* name)
{
}

//--------------------------------------------------------
//--------------------------------------------------------

unsigned char* Comms::RecvData(/*unsigned char* data,*/ unsigned int& dataLen, unsigned char& clientId)
{
	return QueueGet(/*data,*/ dataLen, clientId);
}

// called when a data recive event occures
bool Comms::OnDataRecv(unsigned char* data, unsigned int  dataLen, unsigned char  clientId)
{
//	debug_out("\n Comms::OnDataRecv()... MSG = %d", data[0]);

	bool bRet = true;

	// there are cases where some messages can be concatenated into one ... so 
	// parse them here
	int offset  = 0;
	int msgSize = 0;

#ifdef _DEBUG
	int nbMsg = 0;
#endif // _DEBUG

	do
	{
		// verify msg type
		if ((data[ offset ] <= 0) || ( data[ offset ] >= BT_MESSAGES_NUMBER))
		{
			// unknown message received
			//A_ASSERT(data[ offset ] == 0);
			break;
		}

		msgSize = MP_MESSAGES_LENGTHS[ data[ offset ] ];

		if (msgSize == 0)
		{
			// unknown message received
			//A_ASSERT(0);
			break;
		}

		if (!QueuePut(data + offset, msgSize, clientId))
			bRet = false;
		
#ifdef _DEBUG
		nbMsg++;
#endif // _DEBUG

		offset += msgSize;

	} while (offset < dataLen);


//#ifdef _DEBUG
//	if (nbMsg > 1)
//	{
//		dpf("\n ------- MORE MESSAGES IN ONE !!! nbMsg = %d", nbMsg);
//	}
//
//#endif // _DEBUG

	return bRet;
}

//--------------------------------------------------------
//--------------------------------------------------------

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
void Comms::AddDeviceNamePrefix()
{	
	char* devName = GetLocalDeviceName();
	debug_out("\n Comms::AddDeviceNamePrefix() ... devName = %s", devName);

	debug_out("\n Comms::AddDeviceNamePrefix() devName = %s", devName);
	debug_out("\n Comms::AddDeviceNamePrefix() m_pPrefix = %s", m_pPrefix);
	debug_out("\n Comms::AddDeviceNamePrefix() strstr(devName, m_pPrefix) = %s", strstr(devName, m_pPrefix));

	if (strstr(devName, m_pPrefix) != devName)	// add prefix only if not already prefixed
	{
		debug_out("\n NOT PREFIXED -> ADD PREFIX !");
		char name[LOCAL_DEVICE_NAME_SIZE];

		strcpy(name, m_pPrefix);
		strcat(name, devName);

		debug_out("\n NOT PREFIXED -> ADD PREFIX name = %s", name);

		SetLocalDeviceName(name);
	}
	else
	{
		debug_out("\n PREFIXED -> DO NOT ADD PREFIX!");
	}

	m_bHostnamePrefixed = true;

	//debug_out("\n ... Comms::AddDeviceNamePrefix()");
}

//--------------------------------------------------------
//--------------------------------------------------------

void Comms::RemoveDeviceNamePrefix()
{
	char* devName = GetLocalDeviceName();

	debug_out("\n Comms::RemoveDeviceNamePrefix() ... m_bHostnamePrefixed = %d", m_bHostnamePrefixed);
	debug_out("\n Comms::RemoveDeviceNamePrefix() ... devName = %s", devName);

	if (m_bHostnamePrefixed && (strlen(devName) >= strlen(m_pPrefix)))
	{
		SetLocalDeviceName(devName + strlen(m_pPrefix));
	}

	m_bHostnamePrefixed = false;
}

//--------------------------------------------------------
//--------------------------------------------------------

bool Comms::MatchDeviceNamePrefix(const DeviceDetails &aDevice)
{
	debug_out("MatchDeviceNamePrefix: looking for %s in %s\n", m_pPrefix, aDevice.iName);
	return strstr(aDevice.iName, m_pPrefix) == aDevice.iName;
}

#endif // HAS_BLUETOOTH_DISCOVER_FILTERING

//--------------------------------------------------------
//--------------------------------------------------------

bool Comms::Connect(int serverIdx)
{
	return true;
}

void Comms::Disconnect(int iDeviceID)
{
	m_bDevicesCnx[iDeviceID] =  false;

	if (m_iDevicesConnected > 0)
	{
		m_iDevicesConnected--;
	}

	if (m_iDevicesConnected <= 0)
		isConnected = false;
}

void Comms::Connected(int iDeviceID)
{
	if (isServer)
		m_bDevicesCnx[iDeviceID] = true;

	isConnecting = false;
	isConnected = true;	// not really true for server, means we have at least one connection !!!
	m_iDevicesConnected++;

	//debug_out("Comms::Connected iDeviceID = %d, isServer = %d, m_iDevicesConnected = %d, m_bDevicesCnx[iDeviceID] = %d\n", iDeviceID, isServer, m_iDevicesConnected, m_bDevicesCnx[iDeviceID]);
}

bool Comms::IsConnectionAvailable()
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// power
///////////////////////////////////////////////////////////////////////////////

bool Comms::GetPowerState()
{
	return true;
}

void Comms::SetPowerState(bool state)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// devices list implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int Comms::GetDevicesNo()
{
	return m_iDevicesCount;
}

unsigned int Comms::GetConnectedDevicesNo()
{
	int cnt = 0;
	
	for (unsigned int i = 0; i < m_iDevicesCount; i++)
		if (m_bDevicesCnx[i])
			cnt++;
	
	return cnt;
}

void Comms::AddDevice(DeviceDetails* d)
{
	if (m_iDevicesCount < DEVICE_MAX - 1)
	{
		m_pDevices[m_iDevicesCount]		= d;		
		m_iDevicesCount++;
	}
}

void Comms::SelectFirstDevice()
{
	m_iDevicesIdx = 0;
}

DeviceDetails* Comms::GetNextDevice()
{
	if (m_iDevicesIdx < m_iDevicesCount)
		return m_pDevices[m_iDevicesIdx++];
	else
		return 0;
}

void Comms::RemoveDevice(unsigned int deviceId)
{
//	debug_out("\n Comms::RemoveDevice()... deviceId = %d", deviceId);
	SAFE_DELETE(m_pDevices[deviceId]);

	if (deviceId < m_iDevicesCount)
	{
		for(int i = deviceId; i < m_iDevicesCount-1; i++)
			m_pDevices[i] = m_pDevices[i+1];

		m_iDevicesCount--;
		m_pDevices[m_iDevicesCount] = NULL;
	}
	
	debug_out("\n ... Comms::RemoveDevice()\n");
}

void Comms::RemoveUnconnectedDevices()
{
	for(int i = 0; i < DEVICE_MAX; i++)
		if (!m_bDevicesCnx[i])
			RemoveDevice(i);
}

void Comms::ClearDevicesList()
{
	while(m_iDevicesCount)
		RemoveDevice(m_iDevicesCount - 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// receive queue implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Comms::QueueIsEmpty()
{
	return (queue_start == queue_end);
}

bool Comms::QueueIsFull()
{
	return (((queue_end + 1) & QUEUE_MASK) == queue_start);
}

bool Comms::QueuePut(unsigned char* data, unsigned int  dataLen, unsigned char  clientId)
{

//DEBUG ONLY !!!!!
//generate network LAG
	//if(CHighGear::GetInstance()->m_frameCounter % 3)
	//{
	//	debug_out("LOSSSSSS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	//	return false;
	//}

	if (QueueIsFull())
	{
		debug_out("\n Comms::QueuePut QUEUE is FULL!");
		return false;
	}

	if (dataLen > QUEUE_DATA_BUFF_SIZE)
	{
		A_ASSERT(0);
		dataLen = QUEUE_DATA_BUFF_SIZE;
	}
	
	queue_clientId[queue_end] = clientId;
	memcpy(queue_data[queue_end], data, dataLen);
	queue_dataLen[queue_end] = dataLen;

	queue_end++;
	queue_end &= QUEUE_MASK;

	return true;
}

unsigned char *Comms::QueueGet(/*unsigned char* data,*/ unsigned int& dataLen, unsigned char& clientId)
{
	if (QueueIsEmpty())
	{
//		debug_out("\n QueueGet is EMPTY ()!!! queue_start = %d ", queue_start);
		return NULL;
	}

	clientId	= queue_clientId[queue_start];
	unsigned char *pRetData	= queue_data[queue_start];
	dataLen		= queue_dataLen[queue_start];

	// debug_out("\n Comms::QueueGet() - data[0] = %d", pRetData[0]);

	queue_start++;
	queue_start &= QUEUE_MASK;

	return pRetData;
}

void Comms::ResetQueue()
{
	queue_start = queue_end = 0;
}

Comms::~Comms()
{
	// debug_out("\n ~Comms()...");
#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	RemoveDeviceNamePrefix();
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING

	SAFE_DELETE(m_pLocalDeviceName);
	SAFE_DELETE(m_pPrefix);

	// debug_out("\n ... ~Comms()");
}

#endif // HAS_MULTIPLAYER
