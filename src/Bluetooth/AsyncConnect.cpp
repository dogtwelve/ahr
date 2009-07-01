// AsyncClientConnect.cpp: implementation of the CAsyncClientConnect class.
//
//////////////////////////////////////////////////////////////////////

// #include "config.h"

#include "bluetooth/AsyncConnect.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include "bluetooth/smiledef.h"
#include "bluetooth/AsyncAccept.h"
#include "bt_sock.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define S_NONE				0
#define S_CONNECTING		1
#define S_SENDINGNAMELEN	2
#define S_SENDINGNAME		3

//extern void resolve(TSockAddr addrToResolve, char *out);

#define DBG_C debug_out

CAsyncConnect::CAsyncConnect(Series60Comms *s60)
{
	// //debug_out(">>-->>>>> CAsyncConnect::CAsyncConnect 00 \n");
	this->s60 = s60;
	error0 = false;
	inProgress = false;
	connected = false;
	state = S_NONE;
	if (ss.Connect() != KErrNone)
		error0 = true;

	////debug_out(">>-->>>>> CAsyncConnect::CAsyncConnect 01: error:%d \n", error0);
}

CAsyncConnect::~CAsyncConnect()
{
	//debug_out(">>-->>>>> CAsyncConnect::~CAsyncConnect 00 \n");
	Cancel();
	//debug_out(">>-->>>>> CAsyncConnect::~CAsyncConnect 01 \n");
}

void CAsyncConnect::RunL()
{
	//debug_out(">>-->>>>> CAsyncConnect::RunL 00 \n");

	if (iStatus != KErrNone)
	{
		debug_out("\n >>-->>>>> CAsyncConnect::RunL ERROR CAsyncCC %d ", iStatus);
		TSockAddr dummyAddr;
		connected  = false;
		inProgress = false;
		if (!s60->OnConnectComplete(NULL, dummyAddr))
		{
			iStatus = KErrNone;
			state = S_NONE;
			Cancel();
			DBG_C("II: s60Comm will not recover");
			debug_out(">>-->>>>> CAsyncConnect::RunL --- s60Comm will not recover \n");
			return;
		}

		User::After(500);
		startConnect(addr);
		debug_out(">>-->>>>> CAsyncConnect::RunL 02 \n");
		return;
	}

	//debug_out("\n CAsyncConnect::RunL state = %d", state);
	switch(state)
	{
		case S_NONE:
		{
			break;
		}

		case S_CONNECTING:
		{
			debug_out("\nII: CAsyncCC S_CONNECTING: con OK");

			// if (strlen(s60->hg->m_BtName) == 0)
			if (strlen(s60->m_pBluetooth->m_pLocalDeviceName) == 0)
			{
				debug_out("\n EE: CAsyncCC no bt name");
				// strcpy(s60->hg->m_BtName, "???");
				strcpy(s60->m_pBluetooth->m_pLocalDeviceName, "???");				
			}

			connected = true;
			badBuf.SetLength(0);
			// badBuf.Append((char)strlen(s60->hg->m_BtName));
			badBuf.Append((char)strlen(s60->m_pBluetooth->m_pLocalDeviceName));

			debug_out("\n II:sending namelen %d", badBuf.Length());
			state = S_SENDINGNAMELEN;
			sock->Write(badBuf, iStatus);
			SetActive();
			break;
		}

		case S_SENDINGNAMELEN:
		{
			debug_out("\nII: S_SENDINGNAMELEN - CAsyncCC namelen %d", iStatus);

			if (iStatus != KErrNone)
			{
				DBG_C("EE: breaked");
				sock = NULL;
				state = S_NONE;
				break;
			}

			badBuf.SetLength(0);
			// if (strlen(s60->hg->m_BtName) == 0)
			if (strlen(s60->m_pBluetooth->m_pLocalDeviceName) == 0)			
			{
				debug_out("\n EE: CAsyncCC no bt name 1");
				// strcpy(s60->hg->m_BtName, "???");
				strcpy(s60->m_pBluetooth->m_pLocalDeviceName, "???");
			}

			// badBuf.Append((TUint8*)s60->hg->m_BtName, strlen(s60->hg->m_BtName));
			badBuf.Append((TUint8*)s60->m_pBluetooth->m_pLocalDeviceName, strlen(s60->m_pBluetooth->m_pLocalDeviceName));

			debug_out("\n II:sending name %d", badBuf.Length());
			state = S_SENDINGNAME;
			sock->Write(badBuf, iStatus);
			SetActive();
			break;
		}

		case S_SENDINGNAME:
		{
			DBG_C("II:sent name %d", iStatus);

			connected = false;
			inProgress = false;
			state = S_NONE;

			if (iStatus == KErrNone)
			{
				//also sets the value of the next sock if needs another
				//connect. So sock = NULL after is not good
				s60->OnConnectComplete(&sock, addr);
			}

//			sock = NULL;

		}
	}
	//debug_out(">>-->>>>> CAsyncConnect::RunL 03 \n");

}


void CAsyncConnect::DoCancel()
{
	debug_out(">>-->>>>> CAsyncConnect::DoCancel 00 \n");
	DBG_C("II: cancel con");
	if (!inProgress){
//		if (sock)
			//DBG_C("EE: sock2?");
		debug_out(">>-->>>>> CAsyncConnect::DoCancel 02 \n");
		return;
	}

	if (!sock){
		//DBG_C("EE: sock1?");
		debug_out(">>-->>>>> CAsyncConnect::DoCancel 03 \n");
		return;
	}

	sock->CancelConnect();
	sock->CancelWrite();
	debug_out(">>-->>>>> CAsyncConnect::DoCancel 04 \n");
}


TInt CAsyncConnect::RunError(TInt aError)
{
	//debug_out(">>-->>>>> CAsyncConnect::RunError 00 \n");
	//DBG_C("EE: CAsyncCC %d[%d]", aError, state);
	inProgress = false;
	error0 = true;
	state = S_NONE;

	return KErrNone;
}

void CAsyncConnect::startConnect(TSockAddr addr)
{
	debug_out(">>-->>>>> CAsyncConnect::startConnect 00 \n");
	if (inProgress)
	{
		DBG_C("EE: cntr busy");
		debug_out(">>-->>>>> CAsyncConnect::startConnect 01 \n");
		return;
	}

	this->addr = addr;

	connected = false;

	_LIT(KL2Cap, "L2CAP");
	if (error0)
	{
		DBG_C("EE:CAsyncCC error0");
		debug_out(">>-->>>>> CAsyncConnect::startConnect 02 \n");
		return;
	}

	SAFE_CLOSEDEL(sock);
	sock =  new RSocket();

	if (sock->Open(ss, KL2Cap) != KErrNone)
	{
		error0 = true;
		DBG_C("EE: CAsyncCC");
		SAFE_DEL(sock);
		debug_out(">>-->>>>> CAsyncConnect::startConnect 03 \n");
		return;
	}
	addr.SetPort(7);

	CAsyncAccept::disableSecurity((TBTSockAddr*)&addr);

//	char tmp[128];
//	resolve(addr, tmp);
//	//DBG_C("II: !con to [%s]", tmp);

	inProgress = true;
	state = S_CONNECTING;
	sock->Connect(addr, iStatus);
	//DBG_C("II: wait con ...");
	debug_out("sock->Connect: %d", iStatus);
	SetActive();
	//debug_out(">>-->>>>> CAsyncConnect::startConnect 04 \n");
}

void CAsyncConnect::closeSockets()
{
	//debug_out(">>-->>>>> CAsyncConnect::closeSockets 00 \n");
	SAFE_CLOSEDEL(sock);
	//debug_out(">>-->>>>> CAsyncConnect::closeSockets 01 \n");
}


#endif // HAS_BLUETOOTH_SYMBIAN
