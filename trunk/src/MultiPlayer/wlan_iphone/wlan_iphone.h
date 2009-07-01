// Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WINNETWORK_H_INCLUDED_)
#define _WINNETWORK_H_INCLUDED_

#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>

#include "Config.h"

#include "MultiPlayer/Comms.h"

#define LISTEN_PORT "7890"
#define DISCOVER_PORT "7891"

#define CLOSE(skt) if (skt >= 0) close(skt);

enum t_netw_state
{
	STATE_NONE = 0,
	STATE_ACCEPT,
	STATE_DISCOVER,
	STATE_RECV_DATA,
};

class CWlanIPhone : public Comms
{
public:
	static const int	DATA_BUFF_SIZE = QUEUE_DATA_BUFF_SIZE;
	static const long	DISCOVER_TIMER = 3000;
	static const int	DISCOVER_BROADCAST_COUNT = 100;
	
	typedef int SOCKET;
	static const int INVALID_SOCKET = -1;
	static const int SOCKET_ERROR = -1;

	CWlanIPhone();
	virtual ~CWlanIPhone();

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
	virtual void Disconnect();
	virtual bool IsConnectionAvailable();
	
	// power
    virtual bool GetPowerState();
	virtual void SetPowerState(bool state);
	
	// devices
	virtual void RemoveDevice(unsigned int deviceId);
	int FindDevice(void* address);

private:
	void ConnectionClosedWith(unsigned int deviceId);
	void CleanUp();
	bool InitDiscover(bool server);

	SOCKET	DataSocket[DEVICE_MAX];
	SOCKET	ListenSocket;
	SOCKET	DiscoverSocket;

	unsigned char dataBuff[DATA_BUFF_SIZE];

	int		state;
	
	unsigned long timer;
	unsigned int counter;
};

#endif // _WINNETWORK_H_INCLUDED_

