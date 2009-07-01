// AsyncClientConnect.h: interface for the CAsyncClientConnect class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ASYNCCLIENTCONNECT__
#define __ASYNCCLIENTCONNECT__

#include "config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

class CAsyncConnect;
class Series60Comms;

#include "bluetooth/AsyncBase.h"
#include "bluetooth/Series60Comms.h"

class CAsyncConnect : public CAsyncBase
{
public:
	void closeSockets();
	CAsyncConnect(Series60Comms *s60);
	virtual ~CAsyncConnect();

	// from CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

	void startConnect(TSockAddr addr);

	bool inProgress;
	bool connected;

protected:
	Series60Comms *s60;
	RSocketServ ss;
	RSocket *sock;
	char rawBuf[128];
	TPtr8 *buf;

	bool error0;
	int state;

	TBuf8<64> badBuf;

	TSockAddr addr;
};

#endif // HAS_BLUETOOTH_SYMBIAN


#endif
