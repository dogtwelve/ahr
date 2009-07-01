#ifndef _THROW_H_
#define _THROW_H_

#include "config.h"

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#if (defined WIN32) 
#ifdef IPHONE
//...
#else
#include <assert.h>
#endif
	template<class T>
	void _Throw(const T& e,const char* file,int line)
	{
#ifdef IPHONE
//...
#else
		assert(!"Abnormal Condition");
#endif

		#ifdef ASEMANIP_PARSERONLY
			throw e;
		#endif
	}
	#define Throw(x) _Throw(x,__FILE__,__LINE__)

#else	// SYMBIAN

	// No exceptions on Symbian, 
	extern void _Throw(const char*,int line);
	#define Throw(x) _Throw(__FILE__,__LINE__)

#endif// WINDOWS/SYMBIAN




#ifndef ASEERR_MEMALLOC
	#define ASEERR_MEMALLOC 0
#endif

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<class T>
static void SafeNew(T** ptr)
{
	*ptr = STATIC_NEW T;
	if(*ptr==((void*)0))
		Throw(ASEERR_MEMALLOC);	
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<class T>
static T* SafeNew()
{
	T *ptr;
	SafeNew(&ptr);
	return ptr;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<class T>
static void SafeNew(T** ptr,int nbr)
{
	*ptr = STATIC_NEW T[nbr];
	if(*ptr==((void*)0))
		Throw(ASEERR_MEMALLOC);	
}

#endif // _THROW_H_
