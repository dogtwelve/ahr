// AsyncAccept.cpp: implementation of the CAsyncAccept class.
//
//////////////////////////////////////////////////////////////////////

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include "bluetooth/AsyncAccept.h"
#include "bluetooth/smiledef.h"

#include "btmanclient.h"
#include "bluetooth/Async_btmanclient.h"

#include <bt_sock.h>

#define DBG_ACC debug_out

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define S_NONE		0
#define S_ACCEPT	1
#define S_NAMELEN	2
#define S_NAME		3

//#define //DBG_ACC ;//debuglog

#ifdef NOKIA_6630
	#define BT_6630
#endif


CAsyncAccept::CAsyncAccept(Series60Comms *s60)
{
	// debug_out(">>>>> CAsyncAccept::CAsyncAccept 00 \n");
	sockAccept = NULL;
	client = NULL;
	s60Comm = s60;
	state = S_NONE;
	tmpBuf = NULL;
	TInt error = ss.Connect();
	if (error != KErrNone)
		error0 = true;
	inProgress = false;
	debug_out(">>>>> OPA --- CAsyncAccept::CAsyncAccept 01: error:%d \n", error);
}

CAsyncAccept::~CAsyncAccept()
{
	debug_out("\n CAsyncAccept::~CAsyncAccept ...");
	// cancel pending operation if any
	Cancel();

	SAFE_CLOSEDEL(sockAccept);
	SAFE_CLOSEDEL(client);
	ss.Close();

	debug_out("\n ... CAsyncAccept::~CAsyncAccept");
}

void CAsyncAccept::RunL()
{
	debug_out("\n CAsyncAccept::RunL() ... state = %d", state);
	if (iStatus != KErrNone)
	{
		debug_out(">>>>> CAsyncAccept::RunL 01_0 ... iStatus:%d\n",iStatus);
		DBG_ACC("EE: accept %d", iStatus);
		SAFE_CLOSEDEL(client);
		DBG_ACC("EE: deleted client");
		inProgress = false;
		debug_out(">>>>> CAsyncAccept::RunL 01_1 \n");
		return;
	}

	// debug_out("\n>>>>> CAsyncAccept::RunL 01 state = %d \n", state);

	switch(state)
	{
		case S_NONE:
		{
			debug_out("\nEE: active S_NONE?");
			break;
		}

		case S_ACCEPT:
		{
			debug_out("\nII:Got a client");			
			client->RemoteName(addr);
			SAFE_DEL(tmpBuf);

			debug_out(">>>>> CAsyncAccept::RunL 02_1 \n");
			//reading name len
			tmpBuf = new TPtr8((TUint8*)rawBuf, 1);

			DBG_ACC("II: read len");
			state = S_NAMELEN;
			client->Read(*tmpBuf, iStatus);			
			SetActive();			

			break;
		}

		case S_NAMELEN:
		{			
			DBG_ACC("II: %d!", tmpBuf->Length());
			nameLen = rawBuf[0];
			SAFE_DEL(tmpBuf);
			
			tmpBuf = new TPtr8((TUint8*)rawBuf, nameLen);
			DBG_ACC("II: read name %d[%d]", nameLen,tmpBuf->MaxLength());
			state = S_NAME;
			client->Read(*tmpBuf, iStatus);

			SetActive();			
			break;
		}

		case S_NAME:
		{		
			memcpy(name, rawBuf, nameLen);
			name[nameLen] = 0;
			DBG_ACC("II: hndsk [%s]", name);
			
			if (s60Comm->OnClientAccepted(client, name, addr))
			{
				client = NULL;
				serverDoAccept();
				break;
			}
			else
			{
				debug_out("\nII: i'm done acpt");
				client = NULL;
				SAFE_CLOSEDEL(sockAccept);
				debug_out("\nII: acpt del");
			}

			state = S_NONE;
			inProgress = false;
			break;
		}
	}	

	debug_out("\n ... CAsyncAccept::RunL()");
}


TInt CAsyncAccept::RunError(TInt aError)
{
	//DBG_ACC("EE: CAsyncAccept %d[%d]", aError, state);
	SAFE_CLOSEDEL(client);
	SAFE_CLOSEDEL(sockAccept);
	state = S_NONE;
	return KErrNone;
}

// cancel pending send operation
void CAsyncAccept::DoCancel()
{
	// debug_out(">>>>> CAsyncAccept::DoCancel 00 \n");

	if (!inProgress)
	{
		if (client)
			// debug_out(">>>>> CAsyncAccept::DoCancel 01 ... client:%d\n", &client);
			//DBG_ACC("EE: client should be NULL");

		return;
	}

	if (!client)
	{
		// debug_out(">>>>> CAsyncAccept::DoCancel 02 \n");
		//DBG_ACC("EE: client NULL!!");
		return;
	}

	//DBG_ACC("II: canceling 00");
	sockAccept->CancelAccept();

	//DBG_ACC("II: canceling 02");
	debug_out(">>>>> CAsyncAccept::DoCancel 03 \n");
}


void CAsyncAccept::startAccepting()
{
	// debug_out("\nCAsyncAccept::startAccepting ... 00\n");
	_LIT(KL2Cap, "L2CAP");

	DBG_ACC("II:CAsyncAccept start");

	TBTSockAddr addr;
//		//DBG_ACC("I:sla 0");

	addr.SetPort(7);

	if (!allowPort(7))
	{
		debug_out("CAsyncAccept::startAccepting 01\n");
		DBG_ACC("E: allow port 7");
		return ;
	}

	CAsyncAccept::disableSecurity((TBTSockAddr*)&addr);
	//paranoia check

	SAFE_CLOSEDEL(sockAccept);

	sockAccept = new RSocket();
	if (sockAccept == NULL)
	{
		//DBG_ACC("E: cannot alloc server");
		debug_out("CAsyncAccept::startAccepting 07\n");
		return ;
	}
//		//DBG_ACC("I:sla 1.5");
	if (sockAccept->Open(ss, KL2Cap) != KErrNone)
	{
		//DBG_ACC("EE:CAsyncAccept cannot open");
		// debug_out("CAsyncAccept::startAccepting 09\n");
		SAFE_DEL(sockAccept);
		debug_out("CAsyncAccept::startAccepting 10\n");
		return ;
	}
//		//DBG_ACC("I:sla 2");
	TInt result = sockAccept->Bind(addr);
	if (result != KErrNone)
	{
		DBG_ACC("EE:Could not bind %d", result);

		SAFE_CLOSEDEL(sockAccept);
		debug_out("CAsyncAccept::startAccepting 14\n");
		return;
	}

	if(sockAccept->Listen(2) != KErrNone)
	{
		//DBG_ACC("EE:Could not listen");
		SAFE_CLOSEDEL(sockAccept);
		debug_out("CAsyncAccept::startAccepting 17\n");
		return ;
	}
//		//DBG_ACC("I:sla 4");
	inProgress = true;
	serverDoAccept();

	// debug_out("\n CAsyncAccept::startAccepting 19\n");
}

void CAsyncAccept::stopAccepting()
{

}

void CAsyncAccept::serverDoAccept()
{
	// debug_out("CAsyncAccept::serverDoAccept 00\n");
	TUint8 buf[128];
	char *bufC = (char*)buf;
	//paranoia check
	if (client != NULL)
	{
// 		// debug_out("CAsyncAccept::serverDoAccept ---- AICI CRAPA!!!! --- \n");
		//DBG_ACC("not nul 1234");
		SAFE_CLOSEDEL(client);
	}

	client = new RSocket();

	if (client == NULL)
	{
		// debug_out("CAsyncAccept::serverDoAccept 07\n");
		//DBG_ACC("E:SOCK_CLIENT_TMP = NULL");
		return;
	}

	if (client->Open(ss) != KErrNone)
	{
		debug_out("EE:CAsyncAccept");
		SAFE_DEL(client);
		return;
	}

	//DBG_ACC("II:start accept %x");
	//
	state = S_ACCEPT;
	sockAccept->Accept(*client, iStatus);
	//DBG_ACC("II: accepting");
	SetActive();
	// debug_out("CAsyncAccept::serverDoAccept 15\n");
}

void CAsyncAccept::disableSecurity(TBTSockAddr *addr)
{
//#ifdef BT_6630
	const TUid KCobainServerUid3 = { 0x03F5C4A9 };

	TBTServiceSecurity addr_settings;
	addr_settings.SetUid(KCobainServerUid3);
	addr_settings.SetAuthentication(EFalse);
	addr_settings.SetEncryption(EFalse);
	addr_settings.SetAuthorisation(EFalse);

	if (addr)
		((TBTSockAddr*)addr)->SetSecurity(addr_settings);
}

#ifdef SYMBIAN8

bool CAsyncAccept::allowPort(int port)
{
	// define the security settings
	const TUid KCobainServerUid3 = { 0x03F5C4A9 };

	// register the service
	TRequestStatus status;

	RBTMan sec_manager;

	if (sec_manager.Connect() != KErrNone)
	{
		//DBG_ACC("E: allowPort");
		return FALSE;
	}

	RBTSecuritySettingsB sec_session;

	if (sec_session.Open(sec_manager) != KErrNone)
	{
		//DBG_ACC("E: session open");
		sec_manager.Close();
		//DBG_ACC("E: session man close");
		return FALSE;
	}

	TBTServiceSecurityB sec_settings(KCobainServerUid3, KSolBtL2CAP, port);

	sec_settings.SetAuthentication(EFalse);
	sec_settings.SetEncryption(EFalse);
	sec_settings.SetAuthorisation(EFalse);

	sec_session.RegisterService(sec_settings, status);
	User::WaitForRequest(status);

	sec_session.Close();
	sec_manager.Close();

	if (status != KErrNone)
		return FALSE;

	return TRUE;
}

#else // ! SYMBIAN8

bool CAsyncAccept::allowPort(int port)
{
	// needed (?)
	return TRUE;
}

#endif // ! SYMBIAN8

#endif // HAS_BLUETOOTH_SYMBIAN
