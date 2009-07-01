#include "Config.h"
#ifndef  _BLUETOOTHNGAGE_H_
#define  _BLUETOOTHNGAGE_H_
#ifdef HAS_BLUETOOTH_NGAGE
//#include <f32file.h>
#include "Comms.h"

#include <fileaccess.h>
#include <ngiafbluetooth.h>

using namespace ngi;
const uint32 KBT_serviceID = 0x1050;
#define MAX_BUFFERS 5
// FORWARD DECLARATIONS
namespace ngi
{
	class IArenaFramework;
}
// CLASS DECLARATIONS

enum
{
	IDLE = 0,
	START_DISCOVERING_DEVICES = 1,
	START_DISCOVERING_SERVICES = 2,
	START_CONNECTING = 3,
	CONNECTED = 4,
	WAITING_FOR_HOST_NAME = 5,
	SENDING_HOST_NAME = 6,
	SENDING_CLIENT_NAME = 7,
	STARTS_SLAVE = 8,
};


/**
* Class CBluetoothPage.
*
* The page responsible for Bluetooth Multiplayer API in the example application.
*/
class CBluetoothNgage : public INAFBluetoothObserver, public Comms, public IArenaObserver
{
    public:
		unsigned char dataBuff[MAX_BUFFERS][QUEUE_DATA_BUFF_SIZE];
		int queueHead; //it is only read and set in the update, so will be no race conditions
		int queueTail; //it is set in callback, and if we miss it in this frame, because the callback, just incremented
					//it will take the buffer in the next frame. The only place where it is set is in callback,
					//the update only read it
		int lengths[MAX_BUFFERS];
        /**
        * CBluetoothPage.
        *
        * Default constructor.
        */
        CBluetoothNgage();

        /**
        * ~CBluetoothPage.
        *
        * Virtual destructor.
        */
        virtual ~CBluetoothNgage();

        /**
        * Initialize.
        *
        * Initialize the Bluetooth page.
        *
        */
        bool Initialize( void );

		ngi::ReturnCode CreateBluetoothInterface(IArenaFramework* aArena);
        /**
         * This method will be called when an input event occurs.
         *
         * @param aInputType type of input (key press/key released/axis moved)
         * @param aDevice the input device that generated the event
         * @param aTimeStamp time of input event
         * @param aData1 the pressed / released key (for keypad devices)
         * @param aData2 not used for keypad devices
         *
         * @return The next page that should be displayed
         */
        //virtual CPage* HandleInput( const InputType aInputType,
        //                            const IInputDevice& aDevice,
        //                            const uint64 aTimeStamp,
        //                            const uint32 aData1,
        //                            const int32 aData2 = 0 );


        /**
        * FocusChange.
        *
        * This method is called by the application class AudioEx
        * when focus is changed and playback should be paused or continued.
        *
        * @param aChange Called with TRUE if focus gained, FALSE if focus lost.
        */
       // void FocusChange( bool32 aChange );


		//INAFBluetoothObserver
		void EchoReceived(INAFBTMsgData&) NO_THROW;
		void GameTextReceived(INAFBTMsgData&) NO_THROW;
		void PrivateGameTextReceived(INAFBTMsgData&) NO_THROW;
		void GameDataReceived(INAFBTMsgData&) NO_THROW;
		void PrivateGameDataReceived(INAFBTMsgData&) NO_THROW;
		void DataToServerReceived(INAFBTMsgData&) NO_THROW;
		void HostClientDisconnectCallback(uint32) NO_THROW;
		void StartSlaveCallback(void) NO_THROW;
		void DiscoverDevicesCallback(INAFBTDeviceDataList*) NO_THROW;
		void DiscoverServicesCallback(INAFBTDeviceDataList*) NO_THROW;
		void ConnectDevicesCallback(void) NO_THROW;
		void SlaveConnectedCallback() NO_THROW;
		void SlaveDisconnectedCallback() NO_THROW;
		void HostReceiveDiscoverDeviceCallback(INAFBTDeviceData*) NO_THROW;
		void HostReceiveDiscoverServiceCallback(INAFBTDeviceData*) NO_THROW;



		// IArenaObserver
		void ProgressReport( EArenaObserverState aState ) NO_THROW;


	// update
	virtual bool update();

    // hosts search
	virtual bool DiscoverServers(); // fills the device list with servers

    // connections
    virtual bool StartServer();
    virtual void StopListening();
    virtual bool StartClient();

    // send/receive data
	virtual bool SendData(unsigned char* data, unsigned int  dataLen, unsigned char  clientId = CLIENT_ID_ALL);

    // local device name
	virtual char* GetLocalDeviceName();
	virtual void SetLocalDeviceName(char* name);

	// connection
	virtual bool Connect(int serverIdx);
	virtual void Disconnect(/*int*/);
	virtual bool IsConnectionAvailable();

	// power
    virtual bool GetPowerState();
	virtual void SetPowerState(bool state);


public:
//	virtual void AddDevice(DeviceDetails* d); 
//	virtual void RemoveDevice(unsigned int deviceId);

	void Reinitialize();





    private:
    	int connected;
		int discoveredDev;
		int discoveredServ;
		int state;
		int devicesAnswered;
		
        /**
        * Private declaration without implementation to avoid copying.
        */
        CBluetoothNgage( const CBluetoothNgage& aCopy );

        /**
        * Private declaration without implementation to avoid assignment.
        */
        CBluetoothNgage& operator=( const CBluetoothNgage& aOther );

		//pointing to bluetooth interface
		INAFBluetooth *mBluetooth;
		
		CPublicNAFBTDeviceDataList mBTDeviceDataList;
		INAFBTDeviceDataList* mDeviceDataList;
};

#endif //HAS_BLUETOOTH_NGAGE
#endif // _BLUETOOTHNGAGE_H_
