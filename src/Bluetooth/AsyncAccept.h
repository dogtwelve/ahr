// AsyncAccept.h: interface for the CAsyncAccept class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ASYNCACCEPT__
#define __ASYNCACCEPT__

#include "config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include <es_sock.h>


class CAsyncBase;
class CAsyncAccept;
class Series60Comms;

#include "bluetooth/AsyncBase.h"
#include "bluetooth/AsyncAccept.h"
#include "bluetooth/Series60Comms.h"


class CAsyncAccept : public CAsyncBase
{
public:
	bool allowPort(int);
	static void disableSecurity(TBTSockAddr *addr);
	void serverDoAccept();
	CAsyncAccept(Series60Comms *s60);
	virtual ~CAsyncAccept();

	// from CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

	void startAccepting();
	void stopAccepting();

	bool inProgress;

protected:
	Series60Comms *s60Comm;
	RSocket *sockAccept;
	RSocket *client;
	RSocketServ ss;
	TSockAddr addr;

	bool error0;
	int state;

	char rawBuf[128];
	TPtr8 *tmpBuf;
	int nameLen;
	char name[64];

};

#endif // HAS_BLUETOOTH_SYMBIAN

#endif // !defined(AFX_ASYNCACCEPT_H__EE1FECA2_545E_4753_B7A3_73C7287CFEFA__INCLUDED_)

