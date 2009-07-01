

#if !defined(AFX_MEMORYMANAGER_MALLOC_H__83A5DC65_A4FF_4A75_8531_38447B1F4A73__INCLUDED_)
#define AFX_MEMORYMANAGER_MALLOC_H__83A5DC65_A4FF_4A75_8531_38447B1F4A73__INCLUDED_

#pragma warning (disable : 4100)

#include "A4_3d_platform_def.h"

#define MM_MALLOC    malloc
#define MM_FREE      free
#define MM_REALLOC   realloc
#define MM_STATIC_MALLOC malloc

#define NEW new
#define STATIC_NEW new
#define MM_DELETE delete
#define DELETE_ARRAY delete[]

#define SAFE_DELETE(p) if(p) { MM_DELETE (p); (p) = 0; }
#define SAFE_DELETE_ARRAY(p) if(p) { DELETE_ARRAY (p); (p) = 0; }

#endif // !defined(AFX_MEMORYMANAGER_MALLOC_H__83A5DC65_A4FF_4A75_8531_38447B1F4A73__INCLUDED_)
