

#include "config.h"

#ifdef HAS_BLUETOOTH_SYMBIAN

#include "bluetooth/asyncbase.h"
#include "hg/File.h"

#ifdef __SYMBIAN32__	// BT emulation

CAsyncBase::CAsyncBase():CActive(CActive::EPriorityStandard)
{
	CActiveScheduler::Add(this);
}

CAsyncBase::~CAsyncBase()
{
	// cancel pending operation if any
	Cancel();
	// remove from scheduler
	Deque();
}
#endif //__SYMBIAN32__	// BT emulation

#endif // HAS_BLUETOOTH_SYMBIAN
