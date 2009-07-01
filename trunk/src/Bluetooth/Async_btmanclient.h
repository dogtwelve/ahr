

#include "Config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

class TBTAccessRequirementsB
	/** The access requirements set up by a bluetooth service.

	Note: This class is provided ONLY for BC with versions prior to v8.0
	@removed
	@internalTechnology
	*/
	{
public:
	IMPORT_C TBTAccessRequirementsB();
	IMPORT_C void SetAuthentication(TBool aPreference);
	IMPORT_C void SetAuthorisation(TBool aPreference);
	IMPORT_C void SetEncryption(TBool aPreference);
	IMPORT_C void SetDenied(TBool aPreference);
	IMPORT_C TBool AuthenticationRequired() const;
	IMPORT_C TBool AuthorisationRequired() const;
	IMPORT_C TBool EncryptionRequired() const;
	IMPORT_C TBool Denied() const;
	IMPORT_C TBool operator==(const TBTAccessRequirementsB& aRequirements) const;
private:
	TInt iRequirements;
private:
	enum TBTServiceSecuritySettings
		{
		EAuthenticate = 0x01,
		EAuthorise = 0x02,
		EEncrypt = 0x04,
		EDenied = 0x08
		};
	};

class TBTServiceSecurityB
	/** The security settings of a bluetooth service.

	Note: This class is provided ONLY for BC with versions prior to v8.0.
	@removed
	@internalTechnology
	*/
	{
public:
	IMPORT_C TBTServiceSecurityB(TUid aUid, TInt aProtocolID, TInt aChannelID);
	IMPORT_C TBTServiceSecurityB(const TBTServiceSecurityB& aService);
	IMPORT_C TBTServiceSecurityB();
	IMPORT_C void SetUid(TUid aUid);
	IMPORT_C void SetProtocolID(TInt aProtocolID);
	IMPORT_C void SetChannelID(TInt aChannelID);
	IMPORT_C void SetDenied(TBool aPreference);
	IMPORT_C TBool AuthorisationRequired() const;
	IMPORT_C TBool EncryptionRequired() const;
	IMPORT_C TBool AuthenticationRequired() const;
	IMPORT_C TBool Denied() const;
	IMPORT_C TUid Uid() const;
	IMPORT_C TInt ProtocolID() const;
	IMPORT_C TInt ChannelID() const;

#ifdef SYMBIAN8
	IMPORT_C void SetAuthentication(TBool aPreference);
	IMPORT_C void SetEncryption(TBool aPreference);
	IMPORT_C void SetAuthorisation(TBool aPreference);
#endif // !SYMBIAN8

private:
	TUid iUid;	///<The UID of the service.  Will be used by the UI to work out the name of the service when prompting the user.
	TInt iProtocolID;	///<The protocol layer below this service.
	TInt iChannelID;	///<The port number on which the service is sitting
	TBTAccessRequirementsB iSecurityRequirements;	///<Whether the service requires authentication, authorisation, or encryption.
	};

typedef TPckgBuf<TBTServiceSecurityB> TBTServiceSecurityPckgBufB; /*! removed*/


class RBTManSubSessionB : public RSubSessionBase
	/** Subsession base class.

	Note: This class is provided ONLY for BC with versions prior to v8.0
	@removed
	@internalTechnology
	*/
	{
public:
	virtual TInt Open(RBTMan& aSession) = 0;
	virtual void Close() = 0;
	IMPORT_C void CancelRequest(TRequestStatus& aStatus);	//THIS IS NOT ASYNCHRONOUS!!!!
	void LocalComplete(TRequestStatus& aStatus, TInt aErr);
	};

class RBTSecuritySettingsB : public RBTManSubSessionB
	/** Security Settings Subsession.

	Note: This class is provided ONLY for BC with versions prior to v8.0
	@removed
	@internalTechnology
	*/
	{
public:
	IMPORT_C TInt Open(RBTMan& aSession);
	IMPORT_C void Close();
	IMPORT_C void RegisterService(const TBTServiceSecurityB& aService, TRequestStatus& aStatus);
	IMPORT_C void UnregisterService(const TBTServiceSecurityB& aService, TRequestStatus& aStatus);
private:
	TBTServiceSecurityPckgBufB iRegPckg;	///<Stores the parameters passed in during a service registration.
	TBTServiceSecurityPckgBufB iUnregPckg;	///<Stores the parameters passed in during a service unregistration.
	};



_LIT(K70sBCStubPanicName,"BTManClntBC");

#endif // HAS_BLUETOOTH
