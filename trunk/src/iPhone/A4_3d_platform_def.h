#ifndef __A4_3D_PLATFORM_DEF__
#define __A4_3D_PLATFORM_DEF__

//#define NOKIA_N95

//device specific bugs
//rax: TODO: remove and use phone UID instead
#define N93_LEFT_SOFTKEY_BUG

//#define R240x320 // in define_symbian.h

#include "define_iPhone.h"

#if defined R352x416 && defined SYMBIAN9
	#undef R352x416
	#define R176x208
	#define DOUBLESCREEN
#else
	#undef DOUBLESCREEN
#endif

#define S60_API
//#define UIQ_API


#if defined R176x208
	#define DISP_X 176//240
	#define DISP_Y 208//320
#elif defined R240x320
	#define DISP_X 240
	#define DISP_Y 320
#elif defined R352x416
	#define DISP_X 352
	#define DISP_Y 416
#elif defined R320x480
	#define DISP_X 320
	#define DISP_Y 480
#else
	#error please define one of them
#endif
#define DISP_MAX DISP_Y

#ifdef DOUBLESCREEN
	#define REAL_DISP_X (DISP_X << 1)
	#define REAL_DISP_Y (DISP_Y << 1)
#else	// ! DOUBLESCREEN
	#define REAL_DISP_X DISP_X
	#define REAL_DISP_Y DISP_Y
#endif	// ! DOUBLESCREEN

//test_memory constants
#define NUMBER_OF_CHUNKS 32
#define CHUNK_SIZE		 256
#define NUMBER_OF_TRIES  60
#define WAITING_TIME	 500000


#endif //__A4_3D_PLATFORM_DEF__
