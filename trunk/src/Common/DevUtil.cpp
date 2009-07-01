#include "config.h"
#include "DevUtil.h"
#include "File.h"

#include <stdio.h>
#include <string.h>

#ifdef __SYMBIAN32__
	#include "application.h"
#endif

#ifdef IPHONE
//...
#else /* IPHONE */
	#ifdef _WINDOWS
	#include <windows.h>
	#include <winbase.h>
	#endif
#endif /* IPHONE */

#ifdef _DEBUG

#if defined _WINDOWS
static char	m_czWork[1024];

#if defined(DEBUG_OUT_ENABLED) && !defined (IPHONE)
void dpf(const char* czLog, ...)
{
	va_list va;

	va_start(va, czLog);
	vsprintf(m_czWork, czLog, va);
	va_end(va);
	OutputDebugString(m_czWork);
}
#endif

#endif
void Trace(const char* in_str)
{ 
	#if defined _WINDOWS && !defined(__BREW__)
		dpf("%s\n", in_str);


	#else//_WINDOWS

	#ifdef BREW
		DBGPRINTF( "%s\n", in_str );
	#endif


		#ifdef __WINS__
			A_IFile* file = A_IFile::Open("c:\\debug.txt", A_IFile::OPEN_APPEND | A_IFile::OPEN_TEXT | A_IFile::OPEN_STANDALONE,false,false);
		#else
			A_IFile* file = A_IFile::Open("c:\\debug.txt", A_IFile::OPEN_APPEND | A_IFile::OPEN_TEXT | A_IFile::OPEN_STANDALONE,false,false); 
		#endif
			A_ASSERT(file);
			if(file)
			{
				file->Write(in_str, strlen(in_str));
				const char * eol = "\r\n";
				file->Write(eol, strlen(eol));
		//        file->Flush();
				A_IFile::Close(file);
			}


	#endif	//_WINDOWS
}

#else//_DEBUG

#ifdef IPHONE

extern "C" void LOGDEBUG(const char *error, ...);

void Trace(const char* in_str)
{ 
	LOGDEBUG(in_str);
}

#else /* IPHONE */

void Trace(const char* in_str)
{ 
	#ifdef BREW
		DBGPRINTF( "%s\n", in_str );
	#endif

	A_IFile* file = A_IFile::Open("debug.txt", A_IFile::OPEN_APPEND | A_IFile::OPEN_TEXT | A_IFile::OPEN_STANDALONE,false,false);
	//A_ASSERT(file);
	if(file)
	{
        file->Write(in_str, strlen(in_str));
//        file->Flush();
        A_IFile::Close(file);
	}
}

#endif /* IPHONE */

#endif//_DEBUG



void TraceClear( )
{ 
#ifndef _WINDOWS
	A_IFile::Delete("c:\\debug.txt", false);
#endif
}

void Trace1(const char* in_str, int in_arg)
{
    char str[256];
    sprintf(str, in_str, in_arg);
    Trace(str);
}