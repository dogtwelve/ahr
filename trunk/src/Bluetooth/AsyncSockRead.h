// AsyncSockRead.h: interface for the CAsyncSockRead class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_ASYNCSOCKREAD_H__B46316C8_A374_4A56_98E3_C5117673AE83__INCLUDED_
#define AFX_ASYNCSOCKREAD_H__B46316C8_A374_4A56_98E3_C5117673AE83__INCLUDED_

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

class CAsyncSockRead;

#include "bluetooth/AsyncBase.h"
#include "MultiPlayer/Bluetooth.h"

class CAsyncSockRead  : public CAsyncBase
{
public:
	CAsyncSockRead(RSocket *sock, CBluetooth *pComms);
	virtual ~CAsyncSockRead();

	// from CActive
	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

	void startReading();

protected:	
	CBluetooth  *m_pComms;
	RSocket		*sock;

	char rawBuf[512];
	int len;
	TPtr8 *buf;
	int state;
};

#endif // HAS_BLUETOOTH_SYMBIAN

#endif // !defined(AFX_ASYNCSOCKREAD_H__B46316C8_A374_4A56_98E3_C5117673AE83__INCLUDED_)

