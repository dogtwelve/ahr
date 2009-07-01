#ifndef __ASYNCDISC__
#define __ASYNCDISC__

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include <es_sock.h>
#include <bt_sock.h>

#include "bluetooth/asyncbase.h"

class CHighGear;
class CBluetooth;

// class CAsyncDisc;

class CAsyncDisc : 	public CAsyncBase
{

public:
	CAsyncDisc(CBluetooth *pBluetooth);
	~CAsyncDisc(void);

	// from CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

	void startDiscovering();

	bool inProgress;

protected:
	CBluetooth *m_pBluetooth;
	bool error0;

	TProtocolDesc	 info;	
	RSocketServ		 ss;
	RHostResolver	 hr;

	TInquirySockAddr inqAddr;

	TNameEntry nameEntry;
};


#endif // HAS_BLUETOOTH_SYMBIAN

#endif
