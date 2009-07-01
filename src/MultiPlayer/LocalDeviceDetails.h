
#if !defined(_DEVICE_DETAILS_H_INCLUDED_)
#define _DEVICE_DETAILS_H_INCLUDED_

#include "memoryallocation.h"

//#ifdef HAS_BLUETOOTH
//	#include <DeviceDetails.h>
//	class DeviceDetails;
//
//	void	TxtUniConv( char* in, int in_len, char* out );
//	int		UniTxtConv( char* in, char* out );
//
//#endif // !HAS_BLUETOOTH

class DeviceDetails
{
    public:
        char		   iName[256];
		void *			iAddress;

        DeviceDetails();
        ~DeviceDetails();
};

#endif // _DEVICE_DETAILS_H_INCLUDED_
