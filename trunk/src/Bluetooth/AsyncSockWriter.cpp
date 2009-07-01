// AsyncSockWriter.cpp: implementation of the CAsyncSockWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "bluetooth/AsyncSockWriter.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define S_NONE 0
#define S_LEN 1
#define S_MESG 2

#include "bluetooth/smiledef.h"
#include "MultiPlayer/Bluetooth.h"
#include "bluetooth/Series60Comms.h"

//#define //DBG_BT ;//debuglog

CAsyncSockWriter::CAsyncSockWriter(RSocket *openedSocket, CBluetooth *pComms)
{
	//debug_out(">-->>-->>-->> CAsyncSockWriter::CAsyncSockWriter 00 \n");
	sock	   = openedSocket;
	m_pComms   = pComms;
	state	   = S_NONE;

	outStart = outEnd = 0;

	avoided = 0;
	//debug_out(">-->>-->>-->> CAsyncSockWriter::CAsyncSockWriter 01 \n");
}

CAsyncSockWriter::~CAsyncSockWriter()
{
	//debug_out(">-->>-->>-->> CAsyncSockWriter::~CAsyncSockWriter 00 \n");
	Cancel();
	//debug_out(">-->>-->>-->> CAsyncSockWriter::~CAsyncSockWriter 01 \n");
}


void CAsyncSockWriter::RunL()
{
//	debug_out("\n CAsyncSockWriter::RunL() ... iStatus = %d", iStatus);
//	debug_out("\n CAsyncSockWriter::RunL() ... state = %d", state);	
//	debug_out("\n CAsyncSockWriter::RunL() ... outStart = %d", outStart);
//	debug_out("\n CAsyncSockWriter::RunL() ... outEnd = %d", outEnd);

	if (iStatus == KErrNone)
	{
		switch(state)
		{
			case S_LEN:
			{
//				debug_out("\n CAsyncSockWriter::RunL() ... S_LEN");
				bufVar.SetLength(0);
				if (length >= bufVar.MaxLength())
				{
//					debug_out("\n CAsyncSockWriter::RunL EE: write too much");
					return;
				}

				bufVar.Append((TUint8*)rawBuf, length);

				state = S_MESG;
				sock->Write(bufVar, iStatus);
				SetActive();
				break;
			}

			case S_MESG:
			{
				if (outEnd == outStart)
				{
					writing = false;
					break;
				}
				else
				{
					state = S_LEN;
					length = outgoingLen[outStart];
					memcpy(rawBuf, outgoing[outStart], length);

					buf1.SetLength(0);
					buf1.Append((char)length);
					sock->Write(buf1, iStatus);

					outStart++; outStart%=MAX_MESSAGES;

					SetActive();
				}
			}
		}
	}
	else
	{
		if (iStatus != KErrAbort)
		{//interrupted
//			debug_out("\n EE:\t DISCONECTED will close !!!");
			writing = false;
			//after this this class will be deleted. also the reader and the sock
			m_pComms->iComms->Disconnected(sock);
		}
	}

//	debug_out("\n ... CAsyncSockWriter::RunL() writing = %d", writing);
}


void CAsyncSockWriter::DoCancel()
{
	// debug_out(">-->>-->>-->> CAsyncSockWriter::DoCancel 00 \n");
	state = S_NONE;
	outStart = outEnd = 0;
	if (sock)
		sock->CancelWrite();
	// debug_out(">-->>-->>-->> CAsyncSockWriter::DoCancel 01 \n");
}

TInt CAsyncSockWriter::RunError(TInt aError)
{
	// debug_out(">-->>-->>-->> CAsyncSockWriter::RunError 00 \n");
	//DBG_BT("EE: write %d", aError);
	writing = false;

	return KErrNone;
}


int CAsyncSockWriter::write(char *buf, int len)
{
//	debug_out("\n CAsyncSockWriter::write, len = %d", len);

	// CHECK THIS !!!
//	debug_out("\n CAsyncSockWriter::write, writing = %d", writing);	

	if (writing)
	{
		//add to stack
		if (((outEnd+1)%MAX_MESSAGES) == outStart)
		{
			//debug_out(">-->>-->>-->> CAsyncSockWriter::write 01 \n");
			return 0;
		}

		//debug_out("\n BT----->>>> WRITE MESSAGE: %d", buf[0]);


		memcpy(outgoing[outEnd], buf, len);
		outgoingLen[outEnd] = len;

		outEnd++; outEnd%=MAX_MESSAGES;

		return 1;
	}

	length = len;
	memcpy(rawBuf, buf, len);

	writing = true;
	//write first the length
	state = S_LEN;

	buf1.SetLength(0);
	buf1.Append((char)len);

	sock->Write(buf1, iStatus);

	// rov- testing only - force it not to send but only one message once ...
	// User::WaitForRequest(iStatus);

	SetActive();
	// debug_out(">-->>-->>-->> CAsyncSockWriter::write 04 \n");
	return 1;
}

#endif
