#ifndef __DEVUTIL_H__
#define __DEVUTIL_H__

#include <stdio.h>
#include <stdarg.h>

template<class T,class V>
inline static void SetFlag(T& in_nState,const V& in_nFlag) {in_nState|=in_nFlag;}


template<class T,class V>
inline static bool GetFlag(const T& in_nState, const V& in_nFlag) { return (in_nState&in_nFlag) == T(in_nFlag);}

#define ACTIVATE_TRACE_ON_DEBUG	1

#ifdef _WINDOWS
#pragma warning(disable : 4786)
#endif

void TraceClear( );


//#ifdef _DEBUG
//
//
//	void Trace(const char* in_str);
//	void Trace1(const char* in_str, int in_arg);
//	#if ACTIVATE_TRACE_ON_DEBUG
//		#define TRACE(str)
//		#define TRACE1(str, arg)
//		#define TRACE_TEST(str) Trace(str)
//		#define TRACE1_TEST(str, arg) Trace1(str, arg)
//
///*
//		#define TRACE(str) Trace(str)
//		#define TRACE1(str, arg) Trace1(str, arg)
//*/
//		
//		#include "stdarg.h"
//		inline void TRACE2(const char* in_str, ... )
//		{
//			char str[256];
//			
//			va_list args;
//			va_start( args, in_str );
//			vsprintf( str, in_str, args );
//			Trace(str);
//			va_end( args );
//		}  
//
//	#else
//		#define TRACE(str)
//		#define TRACE1(str, arg)
//		#define TRACE2(x)
//	#endif
//
//	#ifdef _WINDOWS
//		#define BREAKPOINT _asm int 3
//	#else
//		#define BREAKPOINT
//	#endif
//
//	#ifdef _WINDOWS
//		#define PRINT_ASSERT
//		#define PRINT_ASSERT1
//	#else
//		#define PRINT_ASSERT TRACE
//		#define PRINT_ASSERT1 TRACE1
//	#endif
//
//	#define WARNING(stmnt, str) if(!(stmnt)) {PRINT_ASSERT(str); BREAKPOINT } 
//	#define WARNING1(stmnt, str, arg1) if(!(stmnt)) {PRINT_ASSERT1(str, arg1); BREAKPOINT } 
//	#undef A_ASSERT
//#ifdef __BREW__
//	#ifdef WIN32
//#include <assert.h>
//		#define A_ASSERT(arg1)	assert(arg1)
//	#else
//		#define A_ASSERT(arg1)	{ if ((arg1) == 0)	DBGPRINTF( "Assert failed in file " __FILE__ " at line %d\nExpression:\n" #arg1 "\n", __LINE__);	}
//	#endif
//#else
////salcideon
//	#ifdef WINDOWS_MOBILE
//		#define A_ASSERT(arg1) //WARNING1(arg1,"Assert failed in file " __FILE__ " at line %d\nExpression:\n" #arg1 "\n", __LINE__);
//	#else
//		#define A_ASSERT(arg1) WARNING1(arg1,"Assert failed in file " __FILE__ " at line %d\nExpression:\n" #arg1 "\n", __LINE__);
//	#endif
//#endif
//	#define HARDCODE(v) REMINDER("warning: '" #v "' : value is harcoded")
//
//#else //_DEBUG

	#define BREAKPOINT
	#define WARNING(arg1, arg2)
	#undef ASSERT
	
#include <assert.h>
#ifdef _DEBUG
	#define A_ASSERT(arg1)	assert(arg1)
#else
	#define A_ASSERT(arg1)
#endif

	#define TRACE(str)
	#define TRACE1(str, arg)
#ifdef _DEBUG
#ifdef __SYMBIAN32__
	inline void TRACE2(const char* in_str, ... ) {}
#else
//	#define TRACE2(str, ...)
	#define TRACE2				dpf
#endif // __SYMBIAN32__
#else // _DEBUG
	inline void TRACE2(const char* in_str, ... ) {}
#endif // _DEBUG

//#endif//_DEBUG

#define TRACE_EXPRESSION(arg) TRACE_IN_FILE(#arg "\n")
#define TRACE_FUNCTION TRACE_EXPRESSION

// Compile-time Assertion
#pragma warning(disable : 4094)

#if defined(__BREW__)
	#define STATIC_ASSERT(exp) 
#else
	#define STATIC_ASSERT(exp) struct Unused_struct_for_static_assert{int ASSERT_FAILED: (exp);};
#endif

#pragma warning(disable : 4786)

#ifdef _WINDOWS
	#pragma warning(3 : 4018)
#endif


#endif // ndef __DEVUTIL_H__