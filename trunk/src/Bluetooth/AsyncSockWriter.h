// AsyncSockWriter.h: interface for the CAsyncSockWriter class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_ASYNCSOCKWRITER_H__444FF8A9_5A92_4A16_859D_D0DCBC8FA8FF__INCLUDED_
#define AFX_ASYNCSOCKWRITER_H__444FF8A9_5A92_4A16_859D_D0DCBC8FA8FF__INCLUDED_

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

class CAsyncSockWriter;

#include "bluetooth/Asyncbase.h"
#include "MultiPlayer/Bluetooth.h"
#include "HG/HighGear.h"

#define MAX_MESSAGES	8 // make it bigger -> 64 !!
#define MAX_MSGLEN		512

class CAsyncSockWriter : public CAsyncBase
{
public:
	CAsyncSockWriter(RSocket *openedSocket, CBluetooth *pBluetooth);
	virtual ~CAsyncSockWriter();

	// from CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

	int write(char *buf, int len);
protected:
	int avoided;
	CBluetooth  *m_pComms;
	RSocket		*sock;

	//for now no messages are queued
	char rawBuf[512];
	int length;

	TBuf8<1> buf1;
	TBuf8<512> bufVar;

	bool writing;
	int state;

	char outgoing[MAX_MESSAGES][MAX_MSGLEN];
	int outgoingLen[MAX_MESSAGES];
	int outStart;
	int outEnd;
};

#endif

#endif // !defined(AFX_ASYNCSOCKWRITER_H__444FF8A9_5A92_4A16_859D_D0DCBC8FA8FF__INCLUDED_)

