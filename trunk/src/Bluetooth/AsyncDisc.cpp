
// #include "config.h"

#include "bluetooth/asyncdisc.h"

#include "hg/File.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#define RESOLVER 0

#include <DeviceDetails.h>

#include "MultiPlayer/Bluetooth.h"

extern void	TxtUniConv( char* in, int in_len, char* out );
DeviceDetails* TDeviceDetailsToDeviceDetails(TDeviceDetails devDetails);
TDeviceDetails DeviceDetailsToTDeviceDetails(DeviceDetails devDetails);

CAsyncDisc::CAsyncDisc(CBluetooth *pBluetooth)
{
	//debug_out("-->>-->>>>> CAsyncDisc::CAsyncDisc 00 \n");
	m_pBluetooth = pBluetooth;
	error0 = false;
	inProgress = false;
	if (ss.Connect() != KErrNone)
	{
		error0 = true;
		//DBG_BT("EE:CASyncDisc connect");
	}
	if (!error0)
	{
		if (ss.FindProtocol(_L("BTLinkManager"), info) != KErrNone)
		{
			//DBG_BT("EE:CASyncDisc fp");
			ss.Close();
			error0 = true;
		}
	}
	//debug_out("-->>-->>>>> CAsyncDisc::CAsyncDisc 01: error:%d \n", error0);
}

CAsyncDisc::~CAsyncDisc(void)
{
	//debug_out("-->>-->>>>> CAsyncDisc::~CAsyncDisc 00 \n");
	Cancel();
	//debug_out("-->>-->>>>> CAsyncDisc::~CAsyncDisc 01 \n");
}


void CAsyncDisc::RunL()
{
	//debug_out("-->>-->>>>> CAsyncDisc::RunL 00 \n");
	if (!inProgress)
	{
		//DBG_BT("EE:CAsyncDisc !inP");
		//debug_out("-->>-->>>>> CAsyncDisc::RunL 01 \n");
		return;
	}
	if (iStatus == KErrNone)
	{
		if( nameEntry().iName.Ptr() == NULL )
		{
			debug_out("\n nameEntry() is empty!");
			return;
		}

		char tmp[128];
		TxtUniConv((char*)nameEntry().iName.Ptr(), nameEntry().iName.Length(), tmp);
		debug_out("\n DEVICE NAME : II:-> %s <-", tmp);
				
		TDeviceDetails newDev((TBTSockAddr)nameEntry().iAddr, nameEntry().iName);
		
		// convert from TDeviceDetails to DeviceDetails !		
		m_pBluetooth->FoundDevice(TDeviceDetailsToDeviceDetails(newDev), iStatus.Int());

		hr.Next(nameEntry, iStatus);

		SetActive();
	}
	else if (iStatus == KRequestPending)
	{
		//debug_out("\n CAsyncDisc::RunL II:CASyncDisc pending");
	}	
	else if (iStatus == KErrEof)
	{
		debug_out("\n CAsyncDisc::RunL :CASyncDisc search ended %d", iStatus);

		inProgress = false;

		/*// ROV - TODO 
		TDeviceDetails none;

		// a little tricky
		m_pBluetooth->FoundDevice(none, iStatus.Int());
		*/
	}
	else
	{
		debug_out("\n CAsyncDisc::RunL EE:CASyncDisc %d", iStatus);
		inProgress = false;
	}

	debug_out("-->>-->>>>> ... CAsyncDisc::RunL() \n");
}

TInt CAsyncDisc::RunError(TInt aError)
{
	//debug_out("-->>-->>>>> CAsyncDisc::RunError 00 \n");
	//DBG_BT("II:!CASyncDisc search ended %d", aError);
	inProgress = false;
	return KErrNone;
}
// cancel pending send operation
void CAsyncDisc::DoCancel()
{
	debug_out("-->>-->>>>> CAsyncDisc::DoCancel 00 \n");
	hr.Cancel();
	inProgress = false;
	error0 = false;
}

void CAsyncDisc::startDiscovering()
{
	//debug_out("CAsyncDisc::startDiscovering 00 \n");
	if (CActiveScheduler::Current() == NULL)
	{
		DBG_BT("EE: no schd");
		//debug_out("CAsyncDisc::startDiscovering 01 \n");
		return;
	}
	if (inProgress)
	{
		DBG_BT("EE: allready discovering");
		//debug_out("CAsyncDisc::startDiscovering 02 \n");
		return;
	}


	//debug_out("CAsyncDisc::startDiscovering 03 \n");
	if (hr.Open(ss,info.iAddrFamily,info.iProtocol) != KErrNone)
	{
		DBG_BT("EE:CASyncDisc hr open");
		//debug_out("CAsyncDisc::startDiscovering 04 \n");
		ss.Close();
		//debug_out("CAsyncDisc::startDiscovering 05 \n");
		error0 = true;
	}
	else
	{
		//debug_out("CAsyncDisc::startDiscovering 06 \n");
		inqAddr.SetIAC(KGIAC);
		//debug_out("CAsyncDisc::startDiscovering 07 \n");
		inqAddr.SetAction(KHostResInquiry|KHostResName|KHostResIgnoreCache);
		//debug_out("CAsyncDisc::startDiscovering 08 \n");
		hr.GetByAddress(inqAddr, nameEntry, this->iStatus);
		inProgress = true;
		//debug_out("CAsyncDisc::startDiscovering 09 \n");
		SetActive();
		debug_out("CAsyncDisc::startDiscovering 10 \n");
	}
	debug_out("CAsyncDisc::startDiscovering 11 \n");

}

#endif // HAS_BLUETOOTH_SYMBIAN
