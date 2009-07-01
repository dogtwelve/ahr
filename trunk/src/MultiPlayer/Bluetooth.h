// Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_BLUETOOTH_H_INCLUDED_)
#define _BLUETOOTH_H_INCLUDED_

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include <es_sock.h>

#include "Comms.h"

#define DEVICE_MAX				8
#define DATA_BUFF_SIZE			(64*DEVICE_MAX)	// was 32 bytes max, bigger for names

class CAsyncDisc;
class Series60Comms;

class CBluetooth: public Comms
{
private:	
	CAsyncDisc				*iResolver;

public:
	Series60Comms			*iComms;

	CBluetooth();
	virtual ~CBluetooth();

    // hosts search
	bool DiscoverServers(); // fills the device list with servers
	bool DiscoveryFinished();
	void FoundDevice(/*const*/ DeviceDetails* pDevice, /*TInt*/int aError);

    // connections
    bool StartServer();
    bool StartClient();

	// temp virtual void AddDevice(const DeviceDetails* d);
	// void Disconnected(TInt aDeviceID);

    // send/receive data
	bool SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId = CLIENT_ID_ALL);

    // local device name
	virtual char* GetLocalDeviceName();
	virtual void SetLocalDeviceName(char* name);

	// connection
	bool Connect(int serverIdx);
	void Disconnect(int iDeviceID);

	virtual bool IsConnectionAvailable();

	virtual void RemoveDevice(unsigned int deviceId);

	// power
	virtual bool GetPowerState();
	virtual void SetPowerState(bool state);

	virtual void StopListening();

	void Reinitialize();

};

#endif  //HAS_MULTIPLAYER

#endif // _BLUETOOTH_H_INCLUDED_

