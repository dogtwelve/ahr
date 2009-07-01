// Series60Comms.cpp: implementation of the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#include "config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include <btmanclient.h>
#include "bluetooth/Async_btmanclient.h"
#include "bluetooth/Series60Comms.h"
#include "bluetooth/smiledef.h"

#ifdef SYMBIAN8
#include <settinginfo.h>
#endif
#include "hg/HighGear.h"

#include "MultiPlayer/Bluetooth.h"

DeviceDetails *TDeviceDetailsToDeviceDetails(TDeviceDetails devDetails);
TDeviceDetails DeviceDetailsToTDeviceDetails(DeviceDetails devDetails);

int	UniTxtConv( char* in, char* out );

#ifdef SYMBIAN_UIQ

#include <e32property.h>
#include <qbeamingproperty.h>
#include <internal/qbeaming/qbtoperationmode.hrh>

#else

class CBTMCMSettings : public CBase
{

public:	
	IMPORT_C static class CBTMCMSettings *  NewL(class MBTMCMSettingsCB *);
	IMPORT_C static class CBTMCMSettings *  NewLC(class MBTMCMSettingsCB *);

#ifdef SYMBIAN8
	IMPORT_C int  SetPowerStateL(int, int);
#else // ! SYMBIAN8
	IMPORT_C int  SetPowerState(TBool);
	IMPORT_C static TInt GetPowerState (TBool &aValue);
#endif // ! SYMBIAN8

};

#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//for this index sockets can be used as clients
#define SOCK_BASE	0


//state for client
#define C_NONE			0
#define C_ACCEPTING		1
#define C_CONNECTING	2
#define C_CONNECTED		3 //the server has connected to the client

#define S_NONE			0
#define S_ACCEPTING		1 //accepting more connections
#define S_CONNECTING	2 //server connects back to the clients
#define S_CONNECTED		5 //server has connected to all the clients

#define FLAG_CURRENT_CONNECTING 0

extern void	TxtUniConv( char* in, int in_len, char* out );

Series60Comms::Series60Comms(CBluetooth *pBluetooth)
{
	ss		  = NULL;
	acceptor  = NULL;
	connector = NULL;

	m_pBluetooth = pBluetooth;

	//flags
	client = server = false;

	//create socket server
	TProtocolDesc info;
	ss = new RSocketServ();

	if (ss)
	{
		if (ss->Connect() == KErrNone)
		{
			if (ss->FindProtocol(_L("BTLinkManager"), info) != KErrNone)
			{
				//DBG_BT("EE:ss fp");
				SAFE_CLOSEDEL(ss);
			}
		}
		else
		{
			//DBG_BT("EE:ss connect");
			SAFE_DEL(ss);
		}
	}
	
	// devicesCount	  = 0;
	socksClientsCount = 0;

	int i;
	for (i = 0; i < S60MAXCON; i++)
	{
		socks[i]  = NULL;
		reader[i] = NULL;
		writer[i] = NULL;
	}

	for (i = 0; i < S60MAXFLAGS; i++)
		flag[i] = -1;

	//
	acceptor  = NEW CAsyncAccept(this);
	connector = NEW CAsyncConnect(this);
}

Series60Comms::~Series60Comms()
{
	// debug_out("<<    ******   >>   Series60Comms::~Series60Comms 00\n");
	destroyWhenCan();

//	if (acceptor)
//		acceptor->Cancel();

	//DBG_BT("II:del acceptor");
	SAFE_DEL(acceptor);

//	if (connector)
//		connector->Cancel();
	//DBG_BT("II:del connector");
	SAFE_DEL(connector);
	// debug_out("<<    ******   >>   Series60Comms::~Series60Comms 01\n");
}

void Series60Comms::destroyWhenCan()
{
	// debug_out("<<    ******   >>   Series60Comms::destroyWhenCan 00\n");
	//DBG_BT("II:start destroy");

	for (int i = 0; i < S60MAXCON; i++){
		if (socks[i]){
			//DBG_BT("II:   sock[%d]", i);
			SAFE_DEL(reader[i]);
			SAFE_DEL(writer[i]);
			SAFE_CLOSEDEL(socks[i]);
			//DBG_BT("II:		ok[%d]", i);
		}
	}
	//
	//DBG_BT("II:destroy ss");
	if (ss){
		SAFE_CLOSEDEL(ss);
	}

	// debug_out("<<    ******   >>   Series60Comms::destroyWhenCan 01\n");
}

#ifdef SYMBIAN8
int Series60Comms::BluetoothPowerState(TBool &state)
{
	TInt iState;

	CSettingInfo *ci = CSettingInfo::NewL(NULL);

	if (!ci)
	{
		// debug_out("s60::BluetoothPowerState 02\n");
		return 1;
	}
	CleanupStack::PushL(ci);

	ci->Get(SettingInfo::EBluetoothPowerMode, iState);

	if (iState == 0)
	{
		state = false;
		// debug_out("\n Series60Comms::BluetoothPowerState IS NOT ACTIVE !!! ");
	}
	else
	{
		state = true;
		// debug_out("\n Series60Comms::BluetoothPowerState IS ACTIVE !!! ");
	}

	CleanupStack::PopAndDestroy();
	// /debug_out("s60::BluetoothPowerState 09\n");

	return 0;
}

void Series60Comms::SetBluetoothPowerState(TBool state)
{
	// DBG_BT("II: setting BT 0");
	CBTMCMSettings *tmp;
	tmp = CBTMCMSettings::NewL(NULL);

	if (!tmp)
	{
		return;
	}

	CleanupStack::PushL(tmp);

	tmp->SetPowerStateL(state,0);

	CleanupStack::PopAndDestroy();
}

#else // !SYMBIAN8

int Series60Comms::BluetoothPowerState(TBool &state)
{	
#ifdef SYMBIAN_UIQ
	TInt mode = 0;
	// ad: todo: try to get state with KPropertyKeyBluetoothOnOff
	TInt err = RProperty::Get(KPropertyUidQBeamingCategory, KPropertyKeyGetOperationMode, mode);
	if( mode == EQBTOperationModeDiscoverable )
		state = true;
	else
		state = false;
#else
	CBTMCMSettings *tmp;


	tmp = CBTMCMSettings::NewL(NULL);

	if (!tmp)
	{
//GCCE added -1
		return -1;
	}


	CleanupStack::PushL(tmp);

	tmp->GetPowerState(state);

	CleanupStack::PopAndDestroy();
#endif
	return 0;
}

void Series60Comms::SetBluetoothPowerState(TBool state)
{
#ifdef SYMBIAN_UIQ
	TInt err;
	if( state )
		err = RProperty::Set(KPropertyUidQBeamingCategory, KPropertyKeySetOperationMode, EQBTOperationModeDiscoverable);
	else
		err = RProperty::Set(KPropertyUidQBeamingCategory, KPropertyKeySetOperationMode, EQBTOperationModeOff);
#else
	CBTMCMSettings *tmp;
	tmp = CBTMCMSettings::NewL(NULL);

	if (!tmp)
	{
		return;	
	}

	CleanupStack::PushL(tmp);

	TInt error = tmp->SetPowerState(state);

	CleanupStack::PopAndDestroy();
#endif
}

#endif // !SYMBIAN8

TInt Series60Comms::SendData(const TDesC8& aData )
{
	for (int i = 0; i < socksClientsCount; i++)
		if (writer[i])
			writer[i]->write((char*)aData.Ptr(), aData.Length());
//		else
			//DBG_BT("EE: no writer??");

	return 1;
}

TInt Series60Comms::SendData(TInt aClientId, const TDesC8& aData)
{
	if (writer[aClientId] == NULL)
	{
		//DBG_BT("EE: writer[%d] NULL");
		return 0;
	}

	return writer[aClientId]->write((char*)aData.Ptr(), aData.Length());
}

TInt Series60Comms::Connect(int idx)
{
	debug_out("\n Series60Comms::Connect() ...");

	retries = 3;

	if (acceptor)
		acceptor->Cancel();

	//DBG_BT("II: connect() 0");

	// if (devicesCount <=0)
	if (m_pBluetooth->m_iDevicesCount <= 0)
	{
		return -1;
	}

	if( server )
		flag[FLAG_CURRENT_CONNECTING] = 0;
	else
		flag[FLAG_CURRENT_CONNECTING] = idx;

	DBG_BT("II: connect() 1 [%d]",flag[FLAG_CURRENT_CONNECTING]);

	// TODO: cast devices address
	// connector->startConnect(devices[flag[FLAG_CURRENT_CONNECTING]].iAddress);
	TDeviceDetails device = DeviceDetailsToTDeviceDetails(*m_pBluetooth->m_pDevices[flag[FLAG_CURRENT_CONNECTING]]);

	connector->startConnect(device.iAddress);

	debug_out("\n ... Series60Comms::Connect()");
	return 0;
}

void Series60Comms::StartHostL()
{
	// debug_out("\n II:starthostl");
	server		 = true;
	client		 = false;
	// rov devicesCount = 0;

	acceptor->Cancel();
	acceptor->startAccepting();
}

void Series60Comms::StopListening()
{
	debug_out("\n Series60Comms::StopListening() ... ");
	if (acceptor)
	{
		acceptor->Cancel();
	}

	SAFE_DELETE(acceptor);
	debug_out("\n ... Series60Comms::StopListening()");
}

void Series60Comms::GetLocalDeviceName(THostName &name)
{
	// debug_out("\n<<    ******   >>   Series60Comms::GetLocalDeviceName");
	RSocketServ		tmpServ;
	TProtocolDesc	tmpInfo;
	RHostResolver	tmpHr;

	if (tmpServ.Connect() != KErrNone)
	{
		//DBG_BT("EE: gldn 0");
		goto GLDN_out;
	}

	if (tmpServ.FindProtocol(_L("BTLinkManager"),tmpInfo) != KErrNone)
	{
		//DBG_BT("EE: gldn 1");
		tmpServ.Close();
		goto GLDN_out;
	}

	if (tmpHr.Open(tmpServ,tmpInfo.iAddrFamily,tmpInfo.iProtocol) != KErrNone)
	{
		//DBG_BT("EE: gldn 2");
		tmpServ.Close();
		goto GLDN_out;
	}

	tmpHr.GetHostName(name);
	tmpHr.Close();
	tmpServ.Close();

GLDN_out:
	// debug_out("\n<<    ******   >>   Series60Comms::GetLocalDeviceName out");
	return;
}

TInt Series60Comms::SetLocalDeviceName(THostName name)
{
	debug_out("\n Series60Comms::SetLocalDeviceName ...");
	//DBG_BT("II: SLHN 0");
	RSocketServ tmpServ;
	TProtocolDesc tmpInfo;
	RHostResolver tmpHr;

	TInt status = tmpServ.Connect();
	if (status != KErrNone)
	{
		DBG_BT("EE: slhn %d", status);
		return status;
	}

	status = tmpServ.FindProtocol(_L("BTLinkManager"),tmpInfo);
	if (status != KErrNone)
	{
		DBG_BT("EE: shn find %d", status);
		tmpServ.Close();
		return status;
	}

	status = tmpHr.Open(tmpServ,tmpInfo.iAddrFamily,tmpInfo.iProtocol);

	if (status != KErrNone)
	{
		DBG_BT("EE: shn open %d", status);
		tmpServ.Close();
		return status;
	}

	status = tmpHr.SetHostName(name);

	DBG_BT("EE: shn %d", status);
	tmpHr.Close();
	tmpServ.Close();
	DBG_BT("II: SLHN 1");

	return KErrNone;

}

void Series60Comms::StartClientL()
{
	server 			  = false;
	client 			  = true;
	socksClientsCount = 0;
}

void Series60Comms::Disconnect()
{
	if( socksClientsCount > 0 )
	{
		socksClientsCount--;
		SAFE_DEL(reader[socksClientsCount]);
		SAFE_DEL(writer[socksClientsCount]);
		SAFE_CLOSEDEL(socks[SOCK_BASE+socksClientsCount]);
	}
	if( connector )
		connector->Cancel();
}

void Series60Comms::Disconnected(RSocket *sock)
{
	debug_out("\n Series60Comms::Disconnected - sock !");
	for (int i = 0; i < socksClientsCount; i++)
		if (socks[SOCK_BASE+i] == sock)
		{
			//DBG_BT("II: deleting %d 0", i);
			SAFE_DEL(reader[i]);
			//DBG_BT("II: deleting %d 1", i);
			SAFE_DEL(writer[i]);
			//DBG_BT("II: deleting %d 2", i);
			SAFE_CLOSEDEL(socks[SOCK_BASE+i]);
			//DBG_BT("II: deleting %d 3", i);

			m_pBluetooth->Disconnect(i);
			return;
		}
	//DBG_BT("EE: sock !match");

}

// Pseudo Callbacks

/**
 * Responsible with deletion or storing the socket.
 * returns 1 if wants more clients, 0 if not.
 */
int Series60Comms::OnClientAccepted(RSocket *sock, char *name, TSockAddr addr)
{
	DBG_BT("II: OCA [%s]", name);

	if (server)
	{
		//server may want more clients
		//but does not store the socket
		SAFE_CLOSEDEL(sock);

		DBG_BT("II: OCA 0");

		THostName hn;
		hn.SetLength(0);

		DBG_BT("II: OCA 1");

		for (int i = 0; i < strlen(name); i++)
		{
			TChar tmpC(name[i]);
			hn.Append(tmpC);
		}

		DBG_BT("II: OCA 2");

		TDeviceDetails newDev(addr, hn);
		DeviceDetails  *pDevice = TDeviceDetailsToDeviceDetails(newDev);

		//hg->Declared(hg->device_nb, newDev, KErrNone);
		// hg->ConfirmRemoteDevice(newDev);
		CHighGear::GetInstance()->ConfirmRemoteDevice(pDevice);

		DBG_BT("II: OCA 3");

		return 1;
	}
	else if (client)
	{
		//delete the fist socket, that connected us to the server
		//(and wich the server has closed it after reading our name)
		SAFE_CLOSEDEL(socks[SOCK_BASE]);
		//
		if (socksClientsCount != 1)
		{
			//DBG_BT("EE: *** != 1 ***");
		}
		socksClientsCount = 0;
		//client is happy with the first client
		socks[SOCK_BASE+(socksClientsCount++)] = sock;

		reader[socksClientsCount-1] = NEW CAsyncSockRead(sock, m_pBluetooth);
		reader[socksClientsCount-1]->startReading();
		writer[socksClientsCount-1] = NEW CAsyncSockWriter(sock, m_pBluetooth);

		m_pBluetooth->Connected(0);
		// debug_out("<<    ******   >>   Series60Comms::OnClientAccepted 04\n");
		return 0;
	}
	else
	{
		//DBG_BT("EE: comm in wrong state");
		// debug_out("<<    ******   >>   Series60Comms::OnClientAccepted 05\n");
		return 0;
	}
}

/**
 * Called when a connection is completed
 * It is responsible to store or to delete the socket
 * if sock == NULL, return 1 means it should still try connect
 *
 * This is triggered on the connection initiator side
 */

int Series60Comms::OnConnectComplete(RSocket **sock, TSockAddr addr)
{
	// debug_out("<<    ******   >>   Series60Comms::OnConnectComplete 00\n");

//	if (!server && !client)
		//DBG_BT("II:**%d %d**", server, client);

	if (sock == NULL)
	{
		debug_out("<<    ******   >>   Series60Comms::OnConnectComplete 01\n");
		if( retries-- > 0 )
			return 1;
		
		m_pBluetooth->isConnecting = false;
		return 0;
	}

	m_pBluetooth->isConnecting = false;

	socks[SOCK_BASE+(socksClientsCount++)] = *sock;

	if (server)
	{
		// debug_out("<<    ******   >>   Series60Comms::OnConnectComplete 02\n");
		//try to connect to next one
		flag[FLAG_CURRENT_CONNECTING]++;
		//DBG_BT("II: OCC %d %d", devicesCount, flag[FLAG_CURRENT_CONNECTING]);

		reader[socksClientsCount-1] = NEW CAsyncSockRead(*sock, m_pBluetooth);
		reader[socksClientsCount-1]->startReading();
		writer[socksClientsCount-1] = NEW CAsyncSockWriter(*sock, m_pBluetooth);

		//connector will not delete the connected socket
		*sock = NULL;

		// hg->Connected(socksClientsCount-1, KErrNone);
		m_pBluetooth->Connected(socksClientsCount-1);

		// if (devicesCount > flag[FLAG_CURRENT_CONNECTING])
		if (m_pBluetooth->m_iDevicesCount > flag[FLAG_CURRENT_CONNECTING])
		{
			// debug_out("<<    ******   >>   Series60Comms::OnConnectComplete 03\n");
			// connector->startConnect(devices[flag[FLAG_CURRENT_CONNECTING]].iAddress);
			TDeviceDetails device = DeviceDetailsToTDeviceDetails(*m_pBluetooth->m_pDevices[flag[FLAG_CURRENT_CONNECTING]]);
			connector->startConnect(device.iAddress);
		}
	}

	if (client)
	{
		//debug_out("<<    ******   >>   Series60Comms::OnConnectComplete 04\n");
		*sock = NULL;
		//DBG_BT("II: client acc cancel");
		acceptor->Cancel();
		//DBG_BT("II: ->ok");
		acceptor->startAccepting();
	}

	// debug_out("<<    ******   >>   Series60Comms::OnConnectComplete 05\n");
	return 0;
}

void Series60Comms::DoCancel()
{
	if (acceptor)
	{
		acceptor->Cancel();
	}
	if (connector)
	{
		connector->Cancel();
	}
}

// convert from TDevideDetails to DeviceDetails
DeviceDetails *TDeviceDetailsToDeviceDetails(TDeviceDetails devDetails)
{
	DeviceDetails* pRetValue =  NEW DeviceDetails();

	TxtUniConv((char*)devDetails.iName.Ptr(), devDetails.iName.Length(), pRetValue->iName);

	pRetValue->iAddress = (void*) NEW TBTSockAddr(devDetails.iAddress);

	return pRetValue;
}

// convert from TDevideDetails to DeviceDetails
TDeviceDetails DeviceDetailsToTDeviceDetails(DeviceDetails devDetails)
{		
	int len = strlen(devDetails.iName);

	char *uni = NEW char[len << 1];
		
	UniTxtConv( devDetails.iName, uni);
	
	THostName hostName; // ();

	//  hostName.Copy(static_cast<TUint16*>(uni), len);
	hostName.Copy((TUint16*)uni, len);

	TDeviceDetails retValue( *((TBTSockAddr*)devDetails.iAddress), (THostName)(*uni) );

	memcpy(&retValue.iAddress, devDetails.iAddress, sizeof(TBTSockAddr));
	
	return retValue; // newDevDetails();
}

#endif //HAS_BLUETOOTH_SYMBIAN
