// AsyncSockRead.cpp: implementation of the CAsyncSockRead class.
//
//////////////////////////////////////////////////////////////////////

#include "bluetooth/AsyncSockRead.h"
// #include "config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include "bluetooth/smiledef.h"
#include "bluetooth/Series60Comms.h"

//#define //DBG_BT ;//debuglog

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define S_NONE		2
#define S_GETLEN	0
#define S_GETMSG	1

/**
 * This class can close a socket but it cannot delete it
 */

CAsyncSockRead::CAsyncSockRead(RSocket *openedSock, CBluetooth *pComms)
{
	//debug_out("-->>-->>-->> CAsyncSockRead::CAsyncSockRead 00 \n");
	sock = openedSock;
	m_pComms = pComms;
	state	 = S_NONE;
	buf      = new TPtr8((TUint8*)rawBuf, 1);
	//debug_out("-->>-->>-->> CAsyncSockRead::CAsyncSockRead 01 \n");
}

CAsyncSockRead::~CAsyncSockRead()
{
	//debug_out("-->>-->>-->> CAsyncSockRead::~CAsyncSockRead 00 \n");
	Cancel();
	delete buf;
	//debug_out("-->>-->>-->> CAsyncSockRead::~CAsyncSockRead 01 \n");
}

void CAsyncSockRead::RunL()
{
//	debug_out("\n CAsyncSockRead::RunL() ... iStatus = %d", iStatus);
//	debug_out("\n CAsyncSockRead::RunL() ... state = %d", state);

	if (iStatus != KErrNone)
	{
		//DBG_BT("EE: read sock %d", iStatus);
		if (iStatus != KErrAbort)//interrupted
			m_pComms->iComms->Disconnected(sock);

		//debug_out("-->>-->>-->> CAsyncSockRead::RunL 01 \n");
		return;
	}

	//	debug_out("\n-->>-->>-->> CAsyncSockRead::RunL 00 state = %d", state);

	switch(state)
	{
		case S_NONE:
		{
			break;
		}

		case S_GETLEN:
		{
			len=rawBuf[0];
//			debug_out("\n CAsyncSockRead S_GETLEN read -> II:readed len %d", len);

			//TODO, cleanup the mess
			if (len <= 0 || len >=512)
			{
				//DBG_BT("EE: len = %d", len);
//				debug_out("-->>-->>-->> ERROR ERROR ERROR ERROR ERROR CAsyncSockRead::RunL 02 \n");
				return;
			}

			buf->Set((TUint8*)rawBuf, 0, len);

			state = S_GETMSG;
			sock->Read(*buf, iStatus);
			SetActive();
			break;
		}

		case S_GETMSG:
		{
			// debug_out("\n CAsyncSockRead read -> S_GETMSG !!! ");
			unsigned char clientId = 0;// -1;

			m_pComms->OnDataRecv((unsigned char*)buf->Ptr(), buf->Size(), clientId);

			state = S_GETLEN;

			buf->Set((TUint8*)rawBuf, 0, 1);
			sock->Read(*buf, iStatus);
			SetActive();

			// debug_out("\n BT----->>>> S_GETMSG MESSAGE: %d", buf[0]);
			break;
		}
	}

	//debug_out("-->>-->>-->> CAsyncSockRead::RunL 04 \n");
}

void CAsyncSockRead::DoCancel()
{
	//debug_out("-->>-->>-->> CAsyncSockRead::DoCancel 00 \n");
	state = S_NONE;
	if (sock)
		sock->CancelRead();
	//debug_out("-->>-->>-->> CAsyncSockRead::DoCancel 02 \n");
}

TInt CAsyncSockRead::RunError(TInt aError)
{
	//debug_out("-->>-->>-->> CAsyncSockRead::RunError 00 \n");
	//DBG_BT("EE:read socket");
	sock->Close();
	sock = NULL;
	state = S_NONE;
	//debug_out("-->>-->>-->> CAsyncSockRead::RunError 01 \n");

	return KErrNone;
}

void CAsyncSockRead::startReading()
{
	//debug_out("-->>-->>-->> CAsyncSockRead::startReading 00 \n");
	//DBG_BT("II:CAsyncSR start");
	state = S_GETLEN;

	buf->Set((TUint8*)rawBuf, 0, 1);

	sock->Read(*buf, iStatus);

	SetActive();
	//debug_out("-->>-->>-->> CAsyncSockRead::startReading 01 \n");
}

#endif
