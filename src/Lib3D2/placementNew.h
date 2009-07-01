#ifndef _PLACEMENTNEW_H_
#define _PLACEMENTNEW_H_


#if defined WIN32 && !defined(__BREW__)

//	#include <new>

#else


	inline void *operator new(unsigned int , void* aBase)
		{return(aBase);}


	inline void operator delete(void *, void *)
		{return; }

#endif


#endif // _PLACEMENTNEW_H_
