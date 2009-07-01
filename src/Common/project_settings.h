// Common defines to all sources
#ifndef __PROJECT_SETTINGS__
#define __PROJECT_SETTINGS__

////////////////////////////////////////////////////////////////////////////
#include "memoryallocation.h"

#if (defined(_WINDOWS) || defined(WIN32))
#pragma warning(disable : 4786)
#pragma warning(disable : 4503)
#pragma warning(disable : 4985)
#endif

#define MAX_INT     0x7FFFFFFF
#define MAX_UINT    0xFFFFFFFF
#define MAX_SHORT   0x7FFF
#define MAX_USHORT  0xFFFF

////////////////////////////////////////////////////////////////////////////

#define A2_PREVIEW_VERSION
#define USE_TRACK_REVERSED							0
#define USE_REPLY									0
#define USE_CAMERA_FOLLOW_CAR_WHEN_TURNING			0
#define USE_LOADING_TIPS_IMAGES						0

#ifdef USE_OGL
	#define USE_Z_BUFFER								0
	#define USE_NITRO_BLUR								0 // rax: do not define it on OpenGL
	#define USE_OGL_MOTION_BLUR							0
#else
	#define USE_Z_BUFFER								1
	#define USE_NITRO_BLUR								1
	#define USE_OGL_MOTION_BLUR							0
#endif // USE_OGL

#define USE_STENCIL_BUFFER							0 // actually not used

#define USE_CHEAT_CODES								0

#define DISABLE_VINYLS

#define SHOW_TWO_CARS_IN_GARAGE						1

#define RESUME_AUDIO_FRAMES							4
#define GAMELOFT_LOGO_DURATION						(3000 << 8)

#define CHINESE_LANG_INDEX_START					7

//#define USE_GLOBE
//#define INSTANT_PLAY_CHEAT

//#define DISABLE_SUN
//#define SKIP_MENUS

// enable minimap
//#define DISABLE_MINIMAP
#define DISPLAY_MINIMAP_ALWAYS
#define DRAW_FULLSCREEN					// draw the 3D game window to the entire screen (no letterbox) -cmm
#define TOP_LETTERBOX_BAR_ONLY			// draw the letterbox bar on the top only (bar will be twice as thick as in letterbox) -cmm
#define REMOVE_UNUSED_SPEEDLINES_CODE	// Removes some old code that appears to be deprecated -cmm
#define DRAW_NEW_NITRO_FX				// Allows easy switching between the old and new Nitro FX -cmm
#define DISABLE_BIKE_COCKPIT			// dont display the bike cockpit in "bumper" (first person) cameras

#define _ENVMAP_

#define CAR_SHADOW_COLOR			0x0C


////////////////////////////////////////////////////////////////////////////
// Display

#define IS_PORTRAIT				(m_dispX<=m_dispY)
#define IS_PORTRAIT_REF(ref)	(ref.m_dispX<=ref.m_dispY)
#define IS_LANDSCAPE			(m_dispX>m_dispY)


////////////////////////////////////////////////////////////////////////////
// Rendering

#define DEBUG_PHYSIC 0

#define __565_RENDERING__	// FOR 565 instead of 444 rendering

#ifdef __565_RENDERING__
	#define RGB444TO565( c )		(((c << 1) & 0x1F) | ((c << 3) & 0x7E0) | ((c << 4) & 0xF800))
	#define RGB444TO565ALPHA( c )	(((c << 1) & 0x1F) | ((c << 3) & 0x7E0) | ((c << 4) & 0xF800) | ((((unsigned int)c) <<4) & 0xF0000))
#else
	#define RGB444TO565( c )		(c)
	#define RGB444TO565ALPHA( c )	(c)
#endif

#define RGB888TO565( c )		(((c >> 3) & 0x1F) | ((c >> 5) & 0x7E0) | ((c >> 8) & 0xF800))
#define RGB8888TO4444( c )		(((c >> 16) & 0xF000) | ((c >> 12) & 0x0F00) | ((c >> 8) & 0x00F0) | ((c >> 4) & 0x000F))

#define PACK4444(a ,r ,g ,b) ((((b)&0x0f)<<0) | (((g)&0x0f)<<4) | (((r)&0x0f)<<8) | (((a)&0x0f)<<12))

#define CONVERT_565_TO_888(x)	( \
								( ( (x) & 0xF800 ) << 8 ) | \
								( ( (x) & 0x07E0 ) << 5 ) | \
								( ( (x) & 0x001F ) << 3 ) \
								) 

#define CONVERT_444_TO_888(x)	((((x) & 0x0F00) << 12) | (((x) & 0x0F00) << 8) | (((x) & 0x00F0) << 8) | (((x) & 0x00F0) << 4) | (((x) & 0x000F) << 4) | (((x) & 0x000F)))
#define CONVERT_4444_TO_8888(x)	((((x) & 0xF000) << 16) | (((x) & 0xF000) << 12) | (((x) & 0x0F00) << 12) | (((x) & 0x0F00) << 8) | (((x) & 0x00F0) << 8) | (((x) & 0x00F0) << 4) | (((x) & 0x000F) << 4) | (((x) & 0x000F)))


////////////////////////////////////////////////////////////////////////////
// Screen modes 

#define VIDEO_MODE_12BPP      12
#define VIDEO_MODE_16BPP      16
#define VIDEO_MODE_24BPP      24
#define VIDEO_MODE_32BPP      32

////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	#if defined(WIN32) && !defined(IPHONE)
		#ifndef __BREW__
			#define WIN_DEBUG 1
		#endif
	#endif
#endif

////////////////////////////////////////////////////////////////////////////
// Multiplayer

#ifdef __SYMBIAN32__

	#define GETTIMEMS  User::TickCount

#elif defined IPHONE

	extern "C" unsigned long OS_GetTime (void);
	#define GETTIMEMS	OS_GetTime

#else  // win 

	#define GETTIMEMS GetTickCount

#endif

#endif// __PROJECT_SETTINGS__

		

