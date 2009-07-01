#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "A4_3d_platform_def.h"
#include "project_settings.h"

/////////////////////////////////////////////////////////////////////
// Platform defines

//#define DEBUG_TEX_MEM
#define USE_ACCELEROMETER_SENSITIVITY_SLIDER

//new physics engine
#define TIME_UNIT		256
#define TIME_UNIT_SHIFT	8

//#define USE_ARABIC_LANGUAGE
#define USE_JAPANESE_LANGUAGE
#define USE_FONT_BIG_BLACK_IN_MENUS		// HQ request

#define USE_HIGH_PACKAGE
#define USE_12_TRACKS
#define USE_TOUCH_SCREEN
#define USE_IPHONE_SOUNDS

//#define USE_BONUS_CARS	// nope, HQ req

#define USE_RENDER_INTO_TEXTURE
#define USE_MULTITEXTURE

#ifdef IPHONE
	#define USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
#endif

// before fw 2.1 a hack was needed when loading the sounds
#define SYSTEM_VERSION_SOUND_BUG		0x02010000

#define IPHONE_RELEASE

#define USE_TEXTURES_SHORT_FORMATS

// rax - removed. For some reason (sounds ?) I get a memory warning when entering IGM
//#define DEBUG_MEMORY_WARNING

#ifdef USE_TOUCH_SCREEN
	
	#define DRAW_TOUCH_ZONES

	#ifdef IPHONE		
		//only on IPHONE SDK
		#define USE_SYNCHRONIZE_TOUCH
		#define CAR_CONTROL_ACCELEROMETER
	#endif

	#define TOUCH_CAR_CONTROL_1
//	#define TOUCH_CAR_CONTROL_2 // rax
	#define TOUCH_CAR_CONTROL_EDGE_TAP
	#define TOUCH_INGAME_BUTTON_MENU
	#define HAS_MENU_BUTTONS
	
	#define USE_FANCY_MENUS
	
	#ifdef USE_FANCY_MENUS
		#define MENUS_LOAD_ALL_CARS
//#ifndef IPHONE_RELEASE
		#define USE_CHEAT_ZONES	
//#endif // IPHONE_RELEASE
	#endif // USE_FANCY_MENUS
	
#endif /*USE_TOUCH_SCREEN*/

#define USE_SHOW_STATUS_BAR

#define CAR_CONTROL_TYPE_OPTION

#define INVERT_SOFTKEYS


#define USE_LENS_FLARE								1

#define USE_ROAD_FX									0

#define USE_CAR_SHADOW								1

#define USE_COPTER_SHADOW							0

#define USE_CAR_AUTO_START							1

#define USE_MOTION_BLUR								0

#define USE_PHONE_DEBUG								0

#if USE_CAR_SHADOW==1
	#define USE_OPPONENTS_SHADOW					1
	
	#ifdef USE_RENDER_INTO_TEXTURE
		//#define USE_PROJECT_SHADOW_INTO_TEXTURE
	#endif
#endif

#define USE_DEBUG_CHEATS							0


#define USE_ATTRACT_MODE							0	// have the race in background in main menu (N-Gage)

//#define USE_VERTEX_COLOR_TEST

//#define SHOW_FPS
//#define DEBUG_SPRITE_GLTEXTURE_ALLOC

#undef USE_NITRO_BLUR

#define NEW_PROGRESS_BAR

#define PROGRESS_BAR_EDGE_TOP			0xFFb4b4b4
#define PROGRESS_BAR_EDGE2_TOP			0xFF83888e
#define PROGRESS_BAR_EDGE_BOTTOM		0xFF565d65
#define PROGRESS_BAR_INT_TOP			0xFFff2d04
#define PROGRESS_BAR_INT_BOTTOM			0xFF730000
#define PROGRESS_BAR_INT_EMPTY_TOP		0xFFf6f6f6
#define PROGRESS_BAR_INT_EMPTY_BOTTOM	0xFF868686
#define PROGRESS_BAR_SHADOW				0xFF2b2b2b

//#define USE_DRM						 		// used on N-Gage
//#define USE_INITIAL_SPLASH_SCREEN				// used on N-Gage

/////////////////////////////// SOUNDS //////////////////////////////
#define ENABLE_VIBRATION

#define ENABLE_SOUND

#define DISABLE_SFX     0
#define DISABLE_MUSIC   0


//only on IPHONE SDK
#ifdef IPHONE
	#define USE_MUSIC_WRAPPER
#endif /* IPHONE*/

//#define	SOUND_ENGINE_WAV
//#define USE_MIDI
//#define ENABLE_MENU_SOUNDS

////////////////////////////// ~SOUNDS //////////////////////////////

#ifdef __MARM__
//	#define USE_ARM_ASM			// to use optimized ARM ASM code
#endif

#if WIN_DEBUG
	#define DEBUG_PHYSIC 0
	#define CHECK_MATH 0	
#endif


#ifdef IPHONE
	#define USE_IPHONE_INTRO_VIDEO
#endif


#define SERIES_60			//BBucur.17.09.2004.
#define RETRY_CONNECT		//BBucur.20.09.2004

#define HAS_MULTIPLAYER

#ifdef HAS_MULTIPLAYER

#define USE_RACE_SINCRONIZATION // debug only - pb on symbian bt ...

#define USE_MULTIPLAYER_FRAME_SICRONIZE

#define HAS_MULTIPLAYER_CAR_COLISIONS // collisions with other multiplayer cars

// #define __FAKE_SYMBIAN32__ // for debugging mp symb version on windows ... different sincroniations

// #define DEBUG_MULTIPLAYER

#define USE_MULTIPLAYER_REDUCED_SINCRO // when the a max time for a message has passed -> skip the message and go 
									   // further and aproximate the position then skip it when we receive it ... 

#define USE_MP_MSG_LOW_SIZE // on za device

#ifdef __SYMBIAN32__
//	#define HAS_BLUETOOTH
//	#define WLAN
#endif // __SYMBIAN32__
#ifdef IPHONE
	#define HAS_WLAN
#endif // IPHONE
#endif // HAS_MULTIPLAYER
////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(NGI) || defined (__WIN32__)
	#define MAX_PATH_LENGTH 257
#endif

#ifdef NGI
	#define HIGHGEAR_UID				0x2000AFE5
	#define HIGHGEAR_DATA_PATH			"private\\2000AFE5\\"
#else
	#define HIGHGEAR_UID				0x20017535
	#define HIGHGEAR_DATA_PATH			"private\\20017535\\A4\\"
#endif

// [3/10/2008 ROvidenie]
#define HAS_ONLINE
#ifdef IPHONE
	#define USE_IPHONE_KEYBOARD
#endif

#if (defined(WIN32) || defined(__SYMBIAN32__) || defined(IPHONE))  && !defined(__BREW__)

	// screen size
	//#define m_dispX 176
	//#define m_dispY 208
	#define PORTRAIT_WIDTH				DISP_X
	#define PORTRAIT_HEIGHT				DISP_Y
	#define LANDSCAPE_WIDTH				DISP_Y
	#define LANDSCAPE_HEIGHT			DISP_X

	// #define TEMP_LANDSCAPE
	#if defined(TEMP_LANDSCAPE)
		#define INITIAL_ORIENTATION	2
		#define INITIAL_DISP_X		LANDSCAPE_WIDTH
		#define INITIAL_DISP_Y		LANDSCAPE_HEIGHT
	#else
		#define INITIAL_ORIENTATION	1
		#define INITIAL_DISP_X		PORTRAIT_WIDTH
		#define INITIAL_DISP_Y		PORTRAIT_HEIGHT
	#endif


#elif defined(__BREW__)
	#define m_dispX DISP_X
	#define m_dispY DISP_Y

	#define RGB444TO565( c )		(((c << 1) & 0x1F) | ((c << 3) & 0x7E0) | ((c << 4) & 0xF800))


#else

	ERROR !!! NO PLATEFORM DEFINED !

#endif

#if CHECK_MATH
	// ---------------------------------------------------------------------------
	//	Check integer arithmetics overflow
	// ---------------------------------------------------------------------------
	void _Check_Int_Limit(double);	// Defined in IMath.cpp
	void _Check_Short_Limit(double);

	#define CHECK_SHORT_LIMIT(x)	_Check_Short_Limit(x);
	#define CHECK_LIMIT(x)				_Check_Int_Limit(x);
    void _Check_Precision(double fResult, int nResult, double fTolerancePercentage);

	#define CHK_MULT(x,y)		CHECK_LIMIT(double(x)*double(y))
	#define CHK_ADD(x,y)		CHECK_LIMIT(double(x)+double(y))
    #define CHK_PRECISION(fResult, nResult, fTolerancePercentage) _Check_Precision(fResult, nResult, fTolerancePercentage);

#else
	#define CHECK_SHORT_LIMIT(x)
	#define CHECK_LIMIT(x)
	#define	CHK_MULT(x,y)
	#define CHK_ADD(x,y)
    #define CHK_PRECISION(fResult, nResult, fTolerancePercentage)
#endif



#if WIN_DEBUG
	#define ACTIVATE_REC_MINMAX 1
#endif

#if ACTIVATE_REC_MINMAX
	#ifndef WIN_DEBUG
		#error// Only on Windows
	#endif

	#include <stdio.h>
	#include <limits.h>

	class _CMinMax
	{
	public:
		_CMinMax(const char* name):m_name(name),m_min(INT_MAX),m_max(INT_MIN){};

		~_CMinMax()
		{
			FILE* f = fopen("d:\\minmax.txt","at");
			if(f)
			{
				fprintf(f,"%s min: %d, max: %d max\n",m_name,m_min,m_max);
				fclose(f);
			}
		}

		void operator()(int i)	{	if(i < m_min)m_min=i; if(i>m_max)m_max=i;}
	private:
		const char*	m_name;
		int					m_min;
		int					m_max;
	};
#define REC_MINMAX(x,i)			if(1){static _CMinMax _var(#x); _var(i);};
#else
	#define REC_MINMAX(x,i)
#endif// REC_MINMAX

#ifdef IPHONE
	//The frame rate of the UpdateGame() call
	#define UPDATE_FRAME_RATE 24 
	#define GAMEFPS_CONST 20
	#define UPDATE_FRAME_MS (1000/UPDATE_FRAME_RATE)
	#define UPDATE_FRAME_MS_256 (1000*256/UPDATE_FRAME_RATE)
	//#define k_nUpdateRate UPDATE_FRAME_RATE
#else
	extern const int UPDATE_FRAME_RATE;
	extern const int GAMEFPS_CONST;
	extern const int UPDATE_FRAME_MS;
	extern const int UPDATE_FRAME_MS_256;
	//extern const int k_nUpdateRate;
#endif

#define DEBUG_OUT_ENABLED

#if USE_PHONE_DEBUG
	#define LOG_MACROS_ENABLED
#endif // USE_PHONE_DEBUG

#if defined(__SYMBIAN32__)
     #if defined(LOG_MACROS_ENABLED)      
		#include "stdio.h"
		
		#define DEBUG_FILE_NAME		"c:\\data\\a3log_ngi.txt"
		#define DEBUG_RESET {FILE* file = fopen(DEBUG_FILE_NAME,"w");fclose(file);}

      	#define DEBUG_PRINTF(txt) {FILE* file = fopen(DEBUG_FILE_NAME,"a+");fprintf(file,txt);fclose(file);}
      	#define DEBUG_PRINTF_1P(txt,param) {FILE* file = fopen(DEBUG_FILE_NAME,"a+");fprintf(file,txt,param);fclose(file);}
      	#define DEBUG_PRINTF_2P(txt,param1,param2) {FILE* file = fopen(DEBUG_FILE_NAME,"a+");fprintf(file,txt,param1,param2);fclose(file);}

	#else

	   #define DEBUG_PRINTF(txt)  {}
	   #define DEBUG_RESET   {} 
 	   #define DEBUG_PRINTF_1P(txt,param)  debug_out(txt, param)
 	   #define DEBUG_PRINTF_2P(txt,param1,param2) debug_out(txt, param1, param2)

 	#endif
     	
#else
	#define DEBUG_RESET   {} 
	#define DEBUG_PRINTF(txt)  {} 
	#define DEBUG_PRINTF_1P(txt,param)  TRACE2(txt, param)
    #define DEBUG_PRINTF_2P(txt,param1,param2) TRACE2(txt, param1, param2)
#endif


#if DISABLE_SFX!=0 && DISABLE_MUSIC!=0
//		#ifndef WIN32 //We need the menus in the win mode
#define DISABLE_SOUND_OPTIONS
//		#endif
#endif


//	#define SHOW_AI_DEBUG

//////////////////// FOR PROFILER ////////////////////////
#if USE_PHONE_DEBUG
//	#define ENABLE_PROFILER_TIMER_VIEW
#endif // USE_PHONE_DEBUG
		
	#ifdef ENABLE_PROFILER_TIMER_VIEW

		#define PROFILER_BEGIN( txt )				CHighGear::GetInstance()->ProfileBegin( txt );
		#define PROFILER_FROM_BEGIN( txt )			CHighGear::GetInstance()->ProfilePrintTime( txt, true );
		#define PROFILER_FROM_LAST_PRINT( txt )		CHighGear::GetInstance()->ProfilePrintTime( txt, false );

	#else

		#define PROFILER_BEGIN(txt)				
		#define PROFILER_FROM_BEGIN( txt )		
		#define PROFILER_FROM_LAST_PRINT( txt )

	#endif

///////////////////////////////////////////////////////////

#if defined(__SYMBIAN32__) || defined (IPHONE)

#define S_OK									0
#define S_FALSE									1
//#define FALSE									0
//#define TRUE									1
#define E_FAIL									(0x80004005L)

#endif // __SYMBIAN32__

#ifdef DEBUG_OUT_ENABLED
	extern void debug_out(const char* fmt, ...);
	#if !defined(IPHONE)
		extern void dpf(const char* czLog, ...);
	#else //!IPHONE
		#define dpf	
	#endif
#else //DEBUG_OUT_ENABLED
	#define debug_out
		#define dpf
#endif //DEBUG_OUT_ENABLED


#endif
