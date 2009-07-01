
#ifndef __ASYNCBASE__
#define __ASYNCBASE__

#include "config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#ifdef __SYMBIAN32__

#include <e32base.h>
#include <es_sock.h>

class CAsyncBase :	public CActive
{
public:
	CAsyncBase();
	virtual ~CAsyncBase();
protected:
	TRequestStatus *m_status;
};

#endif //__SYMBIAN32__

#endif // HAS_BLUETOOTH_SYMBIAN

#endif // __ASYNCBASE__
