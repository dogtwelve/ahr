// Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WinBlue_H_INCLUDED_)
#define _WinBlue_H_INCLUDED_

#include <winsock2.h>
#include <Ws2bth.h>
#include <bthapi.h>

//#include "Config.h"

#include "MultiPlayer/Comms.h"

enum t_netw_state_blue
{
	STATE_NONE_B = 0,
	STATE_ACCEPT_B,
	STATE_RECV_DATA_B,
	STATE_DISCOVER_B,
};

class CWinBlue : public Comms
{
public:
	static const int	DATA_BUFF_SIZE = QUEUE_DATA_BUFF_SIZE;

	CWinBlue();
	virtual ~CWinBlue();

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
	virtual void Disconnect(int);
	virtual bool IsConnectionAvailable();

	// devices
	virtual void RemoveDevice(unsigned int deviceId);

	//start bluetooth
	virtual void SetPowerState(bool state);
	virtual bool GetPowerState();

private:
	void CleanUp();

	WSADATA wsaData;
	SOCKET	DataSocket[DEVICE_MAX];
	SOCKET	ListenSocket;

	unsigned char dataBuff[DATA_BUFF_SIZE];

	int RegisterService(BYTE *rgbSdpRecord, int cSdpRecord, int iChannelOffset, UCHAR channel);
	int GetGUID(WCHAR *psz, GUID *pGUID);

	static DWORD WINAPI ScanForNextDevice(LPVOID lpParameter);
	DWORD dwSize;
	LPWSAQUERYSET pwsaResults;
	int scanResult;
	int scanState;  //0 start new scan, 1 - got a result, -1 ended

	int		state;

	HANDLE hLookup;
};

#endif // _WinBlue_H_INCLUDED_

