// Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_COMMS_H_INCLUDED_)
#define _COMMS_H_INCLUDED_

#include "config.h"
#ifdef __SYMBIAN32__
#include "Application.h"
#endif // __SYMBIAN32__

#ifdef HAS_MULTIPLAYER

#include "MultiPlayer/MultiplayerConstants.h"
#include "MultiPlayer/LocalDeviceDetails.h"

const int  MP_MESSAGES_LENGTHS [ BT_MESSAGES_NUMBER ]  = {
													0,	 // NONE
													4,	 // BT_MESSAGE_ACCEPTED		= 1,	// from server to client : client has been accepted (&connected) with idx
													600, // BT_MESSAGE_STARTGAME,			// from server to client : start the game with the parameters
													4, // BT_MESSAGE_STARTCARINFO,		// from client to server : car selected info
													1, // BT_MESSAGE_READY,				// from client to server : game ready to start
													1, // BT_MESSAGE_START,				// from server to client : game start
													MP_MSG_UPDATE_CAR_LENGTH, // BT_MESSAGE_UPDATECAR,			// all : update of a car
													0, // BT_MESSAGE_UPDATERANK,			// from server to client (2 player only) : the rank of the client
													1, // BT_MESSAGE_END,					// from server to clients : race ended -> go to result page and wait
													1, // BT_MESSAGE_RESTART,				// from server to clients : race restart
													0, // BT_MESSAGE_NEXTRACE,			// from server to clients : next race restart
													1, // BT_MESSAGE_QUIT,				// from server to clients : quit mp
													0, // BT_MESSAGE_NEWRACE,				// from server to clients : new race restart
													0, // BT_MESSAGE_LOCKDATA,			// from server to clients : lock informations for mp game
													1, // BT_MESSAGE_ABORT,				// from server to clients : abort game (menu or game)
													2, // BT_MESSAGE_HASQUIT,				// from server to clients : client has quitted (idx)
													2, // BT_MESSAGE_HORN,				// all : horn + car idx (server should forward)
													3, // BT_MESSAGE_PAUSE,				// all : pause / unpause
													0, // BT_MESSAGE_KEEPALIVE,			// all : to tell we're still alive :)
													0, // BT_MESSAGE_SERVER_DISCONNECT,	// server to clients : server disconnected (for the case where the BT lib doesn't detect deconnection)
													MP_MSG_UPDATE_CRUSH_LENGTH // BT_MESSAGE_UPDATECAR_CRUSH		// from server to clients : since the server is the only one to compute the collisions betoween cars -> send the crush resulting offsets														
										  };

class Comms
{
public:


	static const unsigned char	CLIENT_ID_ALL = 255;
//SEFU 8	static int KUidGame;
	int KUidGame;

	Comms();
	virtual ~Comms();

	// update
	virtual bool update();

    // hosts search
	virtual bool DiscoverServers(); // fills the device list with servers
	virtual bool DiscoveryFinished();

    // connections
    virtual bool StartServer();
    virtual void StopListening();
    virtual bool StartClient();

    // send/receive data
	virtual bool SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId = CLIENT_ID_ALL);
	unsigned char* RecvData(/*unsigned char* data,*/ unsigned int& dataLen, unsigned char& clientId);

    // local device name
	virtual char* GetLocalDeviceName();
	virtual void SetLocalDeviceName(char* name);

#ifdef HAS_BLUETOOTH_DISCOVER_FILTERING
	bool	m_bHostnamePrefixed;

	void	AddDeviceNamePrefix();
	void	RemoveDeviceNamePrefix();
	bool	MatchDeviceNamePrefix(const DeviceDetails &aDevice);
#endif // HAS_BLUETOOTH_DISCOVER_FILTERING

	// connection
	virtual bool Connect(int serverIdx);
	virtual void Disconnect(int iDeviceID);

	void Connected(int iDeviceID);
	virtual bool IsConnectionAvailable();

	// power
    virtual bool GetPowerState();
	virtual void SetPowerState(bool state);

	//salcideon - crash was here - needs to have same value as AVAILABLE_SERVERS_MAX_NUMBER
	static const unsigned char     DEVICE_MAX = 50; // 8;

    // devices list
	DeviceDetails*		    m_pDevices[DEVICE_MAX];
	bool					m_bDevicesCnx[DEVICE_MAX];	// for server only	// connection state (false= not connected, true connected, false & id -1 == disconected)
	unsigned int			m_iDevicesCount;
	unsigned int			m_iDevicesIdx;
	unsigned int			m_iDevicesConnected;

	static const int		LOCAL_DEVICE_NAME_SIZE = 100;
	char* 					m_pLocalDeviceName;



protected:
	char*					m_pPrefix;

public:
	
	unsigned int GetDevicesNo();
	unsigned int GetConnectedDevicesNo();

	virtual void AddDevice(DeviceDetails* d);
	virtual void RemoveDevice(unsigned int deviceId);

	void SelectFirstDevice();
	DeviceDetails* GetNextDevice();
	void RemoveUnconnectedDevices();
	void ClearDevicesList();

	// connection
    bool                    		isConnecting;
    bool                    		isConnected;
    bool                    		isServer;

	static const unsigned int		CONNECTION_NONE = 0;
	static const unsigned int		CONNECTION_BLUETOOTH = 1;
	static const unsigned int		CONNECTION_WIFI = 2;

	int								m_iConnectionType; // 0 = none; 1 = bluetooth; 2 = wi-fi

	bool OnDataRecv(unsigned char* data, unsigned int  dataLen, unsigned char  clientId);

protected:

	// queue
    static const unsigned int		QUEUE_RANK = 5;// 7; - TESTING !
    static const unsigned int		QUEUE_SIZE = 1 << QUEUE_RANK;
    static const unsigned int		QUEUE_MASK = QUEUE_SIZE - 1;
    static const unsigned int		QUEUE_DATA_BUFF_SIZE = 600;

    // receive queue
    unsigned char					queue_clientId[QUEUE_SIZE];
    unsigned char					queue_data[QUEUE_SIZE][QUEUE_DATA_BUFF_SIZE];
    unsigned int					queue_dataLen[QUEUE_SIZE];
	unsigned int					queue_start;
	unsigned int					queue_end;

    bool QueueIsEmpty();
    bool QueueIsFull();
    bool QueuePut(unsigned char* data, unsigned int  dataLen, unsigned char  clientId);
    unsigned char* QueueGet(/*unsigned char* data,*/ unsigned int& dataLen, unsigned char& clientId);
public:
	void ResetQueue();
};

#endif  //HAS_MULTIPLAYER

#endif // _COMMS_H_INCLUDED_

