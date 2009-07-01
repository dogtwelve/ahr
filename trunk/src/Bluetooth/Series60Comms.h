// Series60Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIES60COMMS_H__DF6486DD_5562_44C3_9FAB_1B62F291AD25__INCLUDED_)
#define AFX_SERIES60COMMS_H__DF6486DD_5562_44C3_9FAB_1B62F291AD25__INCLUDED_

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include <e32def.h>
#include <DeviceDetails.h>
#include <es_sock.h>
#include <bt_sock.h>

class CAsyncSockRead;
class CAsyncSockWriter;
class CAsyncConnect;
class CAsyncAccept;
class Series60Comms;
class CBluetooth;

#include "bluetooth/AsyncSockRead.h"
#include "bluetooth/AsyncSockWriter.h"
#include "bluetooth/AsyncConnect.h"
#include "bluetooth/AsyncAccept.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MultiPlayer/Bluetooth.h"

//max number of devices, sockets ...etc
#define S60MAXCON	16
#define S60MAXFLAGS	32

class Series60Comms
{
	RSocket				*socks[S60MAXCON];
	CAsyncSockRead		*reader[S60MAXCON];
	CAsyncSockWriter	*writer[S60MAXCON];
	int					socksClientsCount;
	int					flag[S60MAXFLAGS];
	RSocketServ			*ss;
	CAsyncAccept		*acceptor;
	CAsyncConnect		*connector;

	int					state;
	int					retries;
public:
	CBluetooth			*m_pBluetooth;
	bool				client;
	bool				server;

	Series60Comms(CBluetooth *m_pBluetooth);
	virtual ~Series60Comms();
	void destroyWhenCan();

	static int BluetoothPowerState(TBool &state);
	static void SetBluetoothPowerState(TBool state);	

	TInt SendData(const TDesC8& aData );
	TInt SendData(TInt aClientId, const TDesC8& aData);
	TInt Connect(int idx);

	int OnClientAccepted(RSocket *sock, char *name, TSockAddr addr);
	int OnConnectComplete(RSocket **sock, TSockAddr addr);

	void Disconnected(RSocket *sock);

	void GetLocalDeviceName(THostName &name);
	TInt SetLocalDeviceName(THostName name);

	// void SetHostAcceptMode();

	void StartHostL();
	void StartClientL();
	void StopListening();

	void Disconnect();

	void DoCancel();
};

#endif  //HAS_BLUETOOTH_SYMBIAN

#endif // !defined(AFX_SERIES60COMMS_H__DF6486DD_5562_44C3_9FAB_1B62F291AD25__INCLUDED_)

