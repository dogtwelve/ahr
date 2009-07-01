
#include "Lib3DGL/Lib3DGL.h"

#include "GameEngine.h"

#include "config.h"
#include "Gapi.h"
extern CGapi Gapi;
#include "HighGear.h"
#include "Random.h"

#ifdef USE_TOUCH_SCREEN
#include "TouchScreen/TouchZones.h"
#endif // USE_TOUCH_SCREEN

#include "TextureFBO.h"
#include "ScreenBufferWrapper.h"

#ifdef DEBUG_MEMORY_WARNING
extern bool m_bAppGotAnMemoryWarning;
u8 * g_tempBuffer[100];
#endif
extern bool m_bIsRunning;
extern volatile bool m_bIsAppPaused;
volatile bool s_bRecreateSoundEngineAfterInterrupt = false;
//extern int g_isTouch;

//use this to trigger a SwapBuffer
bool			g_bSkipRendering = false;

#ifdef CHECK_MEMORY_LEAKS
#include "MemoryManager.h"
#endif

//#define __USE_32BITS__		// FOR 32 bits screen  EMULATION

CGapi Gapi;
static unsigned int SysTickLast;       // used to fix framerate on PC
unsigned int g_uiFrameDuration; // last frame duration in miliseconds


//#ifdef __USE_32BITS__
//static unsigned int* VScreen = new unsigned int[DISP_X*DISP_Y];
//#else
//static unsigned short* VScreen = new unsigned short[DISP_X * DISP_Y];
//#endif

//removed unused buffers
static unsigned short* VScreen = NULL;

//@refresh ogl window
GLvoid OGLRefresh3D()
{
	::glEnable(GL_DEPTH_TEST);
	::glDepthMask(true);
	::glDepthFunc(GL_LEQUAL);
	::glClearDepthf(1.0f);
	::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//g_lib3DGL->GLEnableFog(GL_LINEAR, 0.0f, 0.0f, 22000.0f, 0.3f, 0.3f, 0.3f, 1.0f);
	//g_lib3DGL->GLEnableFog(GL_LINEAR, 0.0f, 8000.0f, 42000.0f, 0.73f, 0.65f, 0.57f, 1.0f);
}


//specific to game engine

void debug_out(const char* x, ...)
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

//temporary buffer ... needed to build the screen texture
u8 g_ScreenBuffer[ 3 * MAX_TEX_WIDTH * MAX_TEX_HEIGHT];

//tex coord container
float g_ScreenTextureUV[4];

GLuint g_glScreenTextureName = 0;

int g_FPSLastTime = 0;

void PCDisplayUpdate(void)
{	
	g_lib3DGL->Flush2D();
	return;
}



//@initialize application
extern "C" void GameInit()
{
	//m_bIsRunning = false;
	//m_bIsAppPaused = false; //avoid overriding the value from GamePause() ... if an interrupt occurs just before thread starts
	//g_isTouch = false;

	Gapi.mRunFlags = 0;
	Gapi.ScreenPtr = (unsigned short*)VScreen;

	#ifdef __USE_32BITS__
		Gapi.m_displayMode = VIDEO_MODE_32BPP;
	#else
		Gapi.m_displayMode = VIDEO_MODE_16BPP;
	#endif

	strcpy(Gapi.DataPath, ".\\");

	Gapi.PathLen = strlen(Gapi.DataPath);          // size of path in DataPath

	SysTickLast = OS_GetTime();

	Gapi.TickCount256 = OS_GetTime() << 8;   // in ms * 256

	if (!Gapi.InitGame(0))
		return;

	Gapi.m_nSignalStrength = 7; // 0 to 7... 0 means there's no GSM network available

	g_lib3DGL = new Lib3DGL();
	g_lib3DGL->Init3D();
	
	InitScreenBufferIndices();

	g_texScreenBuffer = new TextureFBO(TEX_SCREEN_BUFFER_W, TEX_SCREEN_BUFFER_H, false);

	m_bIsRunning = true;
}


static int g_blurSteps = 0;
//frame update
extern "C" void GameLoop()
{
	if(Gapi.m_bQuit)
	{
		m_bIsRunning = false;
		return;
	}

	//for sync with applicationWillResignActive, applicationDidBecomeActive which get will be call on different thread
	if(m_bIsAppPaused)
	{
		g_bSkipRendering = true;
		return;
	}		

	//get game instance
//	CHighGear* hg = CHighGear::GetInstance();
//	
//	if(s_bRecreateSoundEngineAfterInterrupt ||
//	   (hg->m_gState == CHighGear::gs_menu && (MenuContainer::GetInstance()->canRecreateSoundEngine()) 
//		&& !hg->m_AudioManager.m_soundWrap->m_bIsSoundAndMusicInitialized && !hg->m_nosound) )
//	{		
//		hg->m_AudioManager.initialize();		
//		//restart the music that was stoped on interrupt
//		
//		if(hg->m_gState == CHighGear::gs_menu && !hg->m_nosound)
//		{
//			hg->m_currentMusicIndex = MUSIC_MUSIC_M_MENU;
//			//restart the music ... if an interrupt occurs when sound is enabled ( from OFF to ON)
//			hg->m_bShouldRestartMusicAfterInterrupted = true;
//		}
//		
//		if(hg->m_currentMusicIndex != -1 )
//			hg->CreateMusic(hg->m_currentMusicIndex);
//		
//		hg->m_AudioManager.loadAllEffect();		
//		
//		s_bRecreateSoundEngineAfterInterrupt = false;		
//		
//		//fix restarting the sound after interrupt on screens where the music was only loaded, not playing
//		if(hg->m_bShouldRestartMusicAfterInterrupted)
//		{
//			hg->ResumeAudio();			
//		}
//				
//		//restore master volume
//		if(!hg->m_nosound)
//		{
//			hg->SetMasterVolume( hg->m_Profile.sound_master );
//		}
//	}

	
#ifdef DEBUG_MEMORY_WARNING

	if(m_bAppGotAnMemoryWarning)
	{
		glViewport(0, 0, DISP_X, DISP_Y);
		OGLRefresh3D();
		
		g_lib3DGL->fillRect(0, 0, CHighGear::GetInstance()->m_dispX, CHighGear::GetInstance()->m_dispY, 0xFFFF0000);
		g_lib3DGL->fillRect(50, 50, CHighGear::GetInstance()->m_dispX - 100, CHighGear::GetInstance()->m_dispY - 100, 0xFF0000FF);
		g_lib3DGL->Flush2D();
		return;
	}
	
	//simulate memory warning
//	for(int i = 0; i < 100; i++)
//		g_tempBuffer[i] = new u8[(1<<10)];
//
//	for(int i = 0; i < 100; i++)
//	{
//		g_tempBuffer[i][ OS_GetTime() & ((1<<10) - 1)] = 100;
//	}
	
//	for(int i = 0; i < 100; i++)
//	{
//		Gapi.TickCount256 += g_tempBuffer[i][0];
//	}
	
#endif	
	
	unsigned int SysTickCurrent, dt;

	SysTickCurrent = OS_GetTime();
	if (SysTickCurrent < SysTickLast)        // loop occured
	{
		SysTickLast = 0;
		return;
	}
	
	//limit fps
	if((SysTickCurrent - SysTickLast ) < (unsigned int)(1000.0f / UPDATE_FRAME_RATE) )
	{
		usleep(( (unsigned int)(1000.0f / UPDATE_FRAME_RATE) - SysTickCurrent + SysTickLast) * 1000);
		SysTickCurrent = OS_GetTime();
	}	

	g_uiFrameDuration = SysTickCurrent - SysTickLast;
	SysTickLast = SysTickCurrent;

	Gapi.TickCount256 = SysTickCurrent << 8;   // in ms * 256

	int viewportWidth = DISP_X;
	int viewportHeight = DISP_Y;					
	
	g_sceneViewportW = viewportWidth;
	g_sceneViewportH = viewportHeight;
	
	#ifdef USE_RENDER_INTO_TEXTURE							
		//u32 screenBufferMixColor = 0xFFFFFFFF; // opaque
		bool renderingWithBlur = g_bIsBlurActivated;
		if(renderingWithBlur)// && CHighGear::GetInstance()->m_gState != CHighGear::gs_ingame_menu)
		{
			g_sceneViewportW = viewportWidth;
			g_sceneViewportH = viewportHeight;
			g_bSaveInScreenBuffer = true;						
		}	
		if(g_bSaveInScreenBuffer)					
		{						
			LinkGLToScreenBuffer( 0, 0, g_sceneViewportW, g_sceneViewportH);					
			OGLRefresh3D();
			g_bSaveInScreenBuffer = false;						
		}
		else
	#endif //USE_RENDER_INTO_TEXTURE
		{						
			::glViewport( 0, 0, g_sceneViewportW, g_sceneViewportH );
			OGLRefresh3D();
		}
		g_bSkipRendering = false;					

		Gapi.LoopGame();                       // update game	
	
	#ifdef USE_RENDER_INTO_TEXTURE
		if(IsGLLinkedToScreenBuffer())
		{						
			RestoreGLLinkage();	
			
			if(renderingWithBlur)
			{
			
//				if(CHighGear::GetInstance()->m_PlayingGame != NULL && (CHighGear::GetInstance()->m_PlayingGame->GetPlayerCar()->turbo_time <  ( (CAR_TURBO_TIME * 5 ) / 6)) )
//				   g_blurSteps++;
//				   
//				if(g_blurSteps > 8)
//					g_blurSteps = 8;				
				   
				DrawBlurEffect(0x4FFFFFF, 0x4FFFFFFF, 5, 3, g_blurSteps>>1);
				//DrawBlurEffect(0x8FF0000, 0x8FFF0000, 5, 5, 1);
				//DrawBlurEffect(0xFFFFFFFF, 0xFFFFFFFF, 5, 5, 1);
				FlushScreenBuffer();
			}
			else
			{
				g_blurSteps = 0;
			}				
		}
	#endif //USE_RENDER_INTO_TEXTURE	

	{
		PCDisplayUpdate();
	}

//	if(g_isTouch)
//	{
//		LOGDEBUG("pressed");
//		Gapi.KeyEvent(true, 0x2e);
//		g_isTouch = false;
//	}

}

//close application
extern "C" void GameEnd()
{
	//Gapi.EndGame();
	delete g_texScreenBuffer;
}

static volatile bool s_bExecutingGamePause = false;
//paused game
extern "C" void GamePause()
{	
	if(s_bExecutingGamePause)
		return;
	
	s_bExecutingGamePause = true;
	
	if(m_bIsAppPaused)
	{
		s_bExecutingGamePause = false;
		return;
	}	

	m_bIsAppPaused = true;
	
	CHighGear* hg = CHighGear::GetInstance();
	
	if(m_bIsRunning)//game init done ... game thread start
	{
	
//		hg->m_bShouldRestartMusicAfterInterrupted = false;
//	
//		//suspend game		
//		hg->Suspend();
//	
//		if(hg->m_AudioManager.m_soundWrap->m_bIsSoundAndMusicInitialized)
//		{
//			//check if a music should resume ( on resume reload the currentMusicIndex)
//			//fix restarting the sound after interrupt on screens where the music was only loaded, not playing
//			if(!hg->m_nosound)
//			{
//				if(hg->IsMusicPlaying())
//					hg->m_bShouldRestartMusicAfterInterrupted = true;
//			
//				s_bRecreateSoundEngineAfterInterrupt = true;						
//			}
//		
//			hg->m_AudioManager.destroy();
//		}				
	}	
	
	s_bExecutingGamePause = false;
	
//	printf("GamePause\n");

}

static volatile bool s_bExecutingGameResume = false;

extern "C" void GameResume()
{
	
	if(s_bExecutingGameResume)
		return;	
	s_bExecutingGameResume = true;
	
	if(!m_bIsAppPaused)
	{
		s_bExecutingGameResume = false;
		return;
	}

	if(m_bIsRunning)//game init done ... game thread start
	{
		//resume game
//		CHighGear::GetInstance()->Resume();
//		m_bIsAppPaused = false;	
	}
	
	s_bExecutingGameResume = false;
	//printf("GameResume\n");
}


extern "C" void UpdateTouchPress( unsigned char touchId, int x, int y)
{
	short tmp;
	switch (Gapi.mCurrentOrientation)
	{
		case ORIENTATION_LANDSCAPE_90:
			tmp = x;
			x = 480 - y;
			y = tmp;
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			tmp = y;
			y = 320 - x;
			x = tmp;
			break;
	}
	
#ifdef USE_TOUCH_SCREEN
	CTouchZones::GetInstance()->TouchPressed(touchId, x, y);
#endif // USE_TOUCH_SCREEN
}

extern "C" void UpdateTouchRelease( unsigned char touchId, int x, int y)
{
	short tmp;
	switch (Gapi.mCurrentOrientation)
	{
		case ORIENTATION_LANDSCAPE_90:
			tmp = x;
			x = 480 - y;
			y = tmp;
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			tmp = y;
			y = 320 - x;
			x = tmp;
			break;
	}
	
#ifdef USE_TOUCH_SCREEN
	CTouchZones::GetInstance()->TouchReleased(touchId, x, y);
#endif // USE_TOUCH_SCREEN
}

extern "C" void UpdateTouchMove( unsigned char touchId, int x, int y)
{
	short tmp;
	switch (Gapi.mCurrentOrientation)
	{
		case ORIENTATION_LANDSCAPE_90:
			tmp = x;
			x = 480 - y;
			y = tmp;
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			tmp = y;
			y = 320 - x;
			x = tmp;
			break;
	}
	
#ifdef USE_TOUCH_SCREEN
	CTouchZones::GetInstance()->TouchMoved(touchId, x, y);
#endif // USE_TOUCH_SCREEN
}


//////////////////////////////////////////////////////////////////////////////////

extern "C" bool IsCharacterSupported(unsigned short c)
{
	if (c >= 256)
		return false;

	if (c == ' ')
		return true;

//	if (MenuContainer::GetInstance()->m_FontNormal &&
//		MenuContainer::GetInstance()->m_FontNormal->spriteFont->m_NormalFontMap[c] != 0)
//		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////////////

extern "C" unsigned int GetVersionNumber(char *versionString)
{
	unsigned int version = 0;
	int n = 0;
	char *s = versionString;
	int nDots = 0;

	while (*s)
	{
		if (*s == '.')
		{
			version <<= 8;
			version |= (n & 0xFF);

			n = 0;
			++nDots;
		}
		else if (*s >= '0' && *s <= '9')
		{
			n *= 10;
			n += (*s - '0');
		}

		++s;
	}

	while (nDots < 4)
	{
		version <<= 8;
		if (n != 0)
		{
			version |= (n & 0xFF);
			n = 0;
		}
		++nDots;
	}

	return version;
}