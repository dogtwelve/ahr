
#ifdef IPHONE
#include <string.h>
#endif

#ifdef __WIN32__
#include <memory.h>
#endif // __WIN32__

#include <stdio.h>

#include "MultiPlayer/LocalDeviceDetails.h"

#ifdef __SYMBIAN32__
#ifndef SYMBIAN8
	#include "e32cmn.h"
#endif // SYMBIAN8
#endif // __SYMBIAN32__

DeviceDetails::DeviceDetails()
{
	memset(iName, 0, 256 * sizeof(char));
	iAddress = NULL;
}

DeviceDetails::~DeviceDetails()
{
	// Do not free iAddress here! 
	// SAFE_DELETE(iAddress);
}
