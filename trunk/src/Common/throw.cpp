
#ifndef WIN32

#include "Throw.h"



#include "File.h"
#include <stdio.h>



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void _Throw(const char* file,int line)
{
	A_IFile* log = A_IFile::Open("debug.txt", A_IFile::OPEN_TEXT | A_IFile::OPEN_WRITE);
	if(log)
	{
		char buffer[64];
		sprintf(buffer,"Error: %s:%d\n",file,line);
		log->Write(buffer, 64);
		A_IFile::Close(log);
	}	
}



#endif // Win32
