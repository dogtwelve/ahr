// Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WINNETWORK_H_INCLUDED_)
#define _WINNETWORK_H_INCLUDED_

#include <winsock2.h>

//#include "Config.h"

#include "MultiPlayer/Comms.h"

#ifndef WINDOWS_MOBILE
#define LISTEN_PORT "7890"
#define DISCOVER_PORT "7891"
#else
#define LISTEN_PORT "7890"
#define DISCOVER_PORT "7891"
#endif

enum t_netw_state
{
	STATE_NONE = 0,
	STATE_ACCEPT,
	STATE_DISCOVER,
	STATE_RECV_DATA,
};

class CWinNetwork : public Comms
{
public:
	static const int	DATA_BUFF_SIZE = QUEUE_DATA_BUFF_SIZE;

	CWinNetwork();
	virtual ~CWinNetwork();

	// update
	virtual bool update();

    // hosts search
	virtual bool DiscoverServers(); // fills the device list with servers

    // connections
	virtual bool StartServer();
    virtual void StopListening();
    virtual bool StartClient();

    // send/receive data
	virtual bool SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId = CLIENT_ID_ALL);

    // local device name
	virtual char* GetLocalDeviceName();
	virtual void SetLocalDeviceName(char* name);

	// connection
	virtual bool Connect(int serverIdx);
	virtual void Disconnect(int idx);
	virtual bool IsConnectionAvailable();

	// devices
	virtual void RemoveDevice(unsigned int deviceId);

private:
	void CleanUp();
	bool InitDiscover(bool server);

	WSADATA wsaData;
	SOCKET	DataSocket[DEVICE_MAX];
	SOCKET	ListenSocket;
	SOCKET	DiscoverSocket;

	unsigned char dataBuff[DATA_BUFF_SIZE];

	int		state;
};

#endif // _WINNETWORK_H_INCLUDED_

