#ifdef IPHONE
//#import <UIKit/UIKit.h>
#endif

#include "IphoneUtil.h"


// get current time in milliseconds
extern "C" unsigned long OS_GetTime(void)
{
    struct timeval	tp;
    gettimeofday(&tp, 0);
    return (tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

//debug info
extern "C" void LOGDEBUG(const char *x, ...)
{
#if USE_PHONE_DEBUG
	va_list	argptr;
	va_start (argptr, x);
	vfprintf (stderr, x, argptr);
	fprintf(stderr, "\n");
	va_end (argptr);
	fflush(stderr);
#endif // USE_PHONE_DEBUG
}


