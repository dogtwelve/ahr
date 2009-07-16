#include <windows.h>

#ifdef USE_OGL
	#include "Lib3DGL/Lib3DGL.h"
#endif /* USE_OGL */

#include "config.h"
#include "Gapi.h"
#include "HighGear.h"
#include "Random.h"

#include "TextureFBO.h"
#include "ScreenBufferWrapper.h"

#define FPS_LIMIT		20

////////////////////////////////////////////////////////////////////////////////////////////////////

extern const int UPDATE_FRAME_RATE = 24;
extern const int GAMEFPS_CONST = 20;
extern const int UPDATE_FRAME_MS = (1000/UPDATE_FRAME_RATE);
extern const int UPDATE_FRAME_MS_256 = (1000*256/UPDATE_FRAME_RATE);
//extern const int k_nUpdateRate = UPDATE_FRAME_RATE;

//#define VLD_MAX_DATA_DUMP 10
#include "vld.h"

////////////////////////////////////////////////////////////////////////////////////////////////////


CGapi Gapi;
static unsigned int SysTickLast;       // used to fix framerate on PC
unsigned int g_uiFrameDuration; // last frame duration in miliseconds
bool m_bNoFocus = false;

LRESULT CALLBACK MainProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

#ifndef SCREEN_SCALE_FACTOR
#define SCREEN_SCALE_FACTOR 1
#endif
#define WINDOW_DISP_X (DISP_X * SCREEN_SCALE_FACTOR)
#define WINDOW_DISP_Y (DISP_Y * SCREEN_SCALE_FACTOR)

#define WINDOW_RESIZE_DISP_X (m_dispX * SCREEN_SCALE_FACTOR)
#define WINDOW_RESIZE_DISP_Y (m_dispY * SCREEN_SCALE_FACTOR)

static HINSTANCE hInst;
static HDC       GameWindowDC;
static HGLRC	 GameWindowGLRC = NULL;
static HWND      MainWindow  = NULL;
static TCHAR     szAppName[] = TEXT("App");
static TCHAR     szTitle[]   = TEXT("App");

#ifdef __USE_32BITS__
	static unsigned int* VScreen;
#else
	static unsigned short* VScreen;
#endif // __USE_32BITS__


#ifdef USE_OGL
	GLfloat			g_aspectRatio = 1.0f;

	//use this to trigger a SwapBuffer
	bool			g_bSkipRendering	= false;
#else /* USE_OGL */
	static BITMAP  OffscreenBitmap;
	static HBITMAP hOffscreenBitmap;
	static HDC hDCMem;
#endif /* USE_OGL */

bool m_Paused=false;
bool m_Step=false;

int nZoom = 1;
int m_dispX;
int m_dispY;

//Mouse coordinates info
bool m_bMousePressed;
int m_iMouseX;
int m_iMouseY;

bool IsInBox(int x, int y, int x1, int y1, int x2, int y2)
{
	return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

//salcideon used by LZMA
unsigned char temp_buff[16 * 1024];

#ifdef BLACK_SCREEN
bool LandscapeMode;
void UpdateOrientation( HWND hWnd )
{
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	if (width > height)
	{
		if ( LandscapeMode == false ) //last frame i was in portrait
		{
		    LandscapeMode = true;
			Gapi.m_pHighGear->Suspend();

			RECT rc;
			rc.left = rc.top = 0;
			rc.right = width;
			rc.bottom = height;
			MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);

			HDC hDC = GetDC( hWnd );
			FillRect( hDC, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

			HGDIOBJ oldFont = SelectObject( hDC, GetStockObject( SYSTEM_FONT ) );
			SetTextColor( hDC, RGB( 255, 0, 0 ) );
			SetBkMode( hDC, TRANSPARENT );

			rc.top = rc.bottom / 2 - 30;

			// "en", "fr", "de", "sp", "it"
			if (CHighGear::GetInstance()->m_Profile.language == 0)
			{
				DrawText( hDC, L"This game cannot run in\nlandscape mode.\nPlease switch your screen\nback to portrait mode.", 125 - 34 - 3, &rc, DT_CENTER );
			}
			if (CHighGear::GetInstance()->m_Profile.language == 1)
			{
				DrawText( hDC, L"Ce jeu ne peut fonctionner\nen mode paysage. Veuillez\nchanger la configuration de votre\n?ran pour le mode portrait.", 152 - 34 - 3, &rc, DT_CENTER );
			}
			if (CHighGear::GetInstance()->m_Profile.language == 2)
			{
				DrawText( hDC, L"Dieses Spiel funktioniert\nnicht im Querformat.\nStelle das Display\nauf Hochformat um.", 121 - 34 - 3, &rc, DT_CENTER );
			}
			if (CHighGear::GetInstance()->m_Profile.language == 3)
			{
				DrawText( hDC, L"El juego no funciona\nen modo horizontal.\nPor favor, pon tu pantalla\nen modo vertical.", 122 - 34 - 3, &rc, DT_CENTER );
			}
			if (CHighGear::GetInstance()->m_Profile.language == 4)
			{
				DrawText( hDC, L"Questo gioco non pu?nfunzionare in mod. laterale.\nUsa il telefono in modalit?npieghevole aperto.", 133 - 34 - 3, &rc, DT_CENTER );
			}

			SelectObject( hDC, oldFont );
			ReleaseDC( hWnd, hDC );
		}
	}
	else
	{
		if (LandscapeMode == true)		//last frame i was in landscape
		{
			Gapi.m_pHighGear->Resume();
			SHFullScreen(MainWindow, (SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON));
		}
		LandscapeMode = false;
	}

}
#endif

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef AUTODETECT_ORIENTATION
DWORD initialDeviceOrientation = -1;

void SetDefaultOrientation()
{
	DEVMODE devMode;

	memset(&devMode, 0x00, sizeof(devMode));
	devMode.dmSize = sizeof(devMode);
	devMode.dmFields = DM_DISPLAYORIENTATION;

	ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_TEST, NULL);

	//int width = GetSystemMetrics(SM_CXSCREEN);
	//int height = GetSystemMetrics(SM_CYSCREEN);

	if (DMDO_DEFAULT != devMode.dmDisplayOrientation /*|| width > height*/)
	{
		//Currently not at 0 degrees
		initialDeviceOrientation = devMode.dmDisplayOrientation;
		devMode.dmDisplayOrientation = DMDO_DEFAULT;

		ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_RESET, NULL);
	}
}

void SetInitialOrientation()
{
	DEVMODE devMode;

	memset(&devMode, 0x00, sizeof(devMode));
	devMode.dmSize = sizeof(devMode);
	devMode.dmFields = DM_DISPLAYORIENTATION;

	ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_TEST, NULL);

	if (initialDeviceOrientation != -1)
	{
		devMode.dmDisplayOrientation = initialDeviceOrientation;
		ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_RESET, NULL);
	}
}
#endif
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_OGL

void Zoom(int zoom){}

#else /* USE_OGL */

void Zoom(int zoom)
{
	if(zoom < 1)
		return;

	nZoom = zoom;
	RECT rc, client;
	GetWindowRect(MainWindow, &rc);
	GetClientRect(MainWindow, &client);
	int gapX = (rc.right-rc.left) - (client.right-client.left);
	int gapY = (rc.bottom-rc.top) - (client.bottom-client.top);
	float ratio = (float)WINDOW_DISP_X / WINDOW_DISP_Y;

	SetWindowPos(
		MainWindow,
		0,
		0,
		0,
		(WINDOW_DISP_X * zoom) + gapX,
		(int)(WINDOW_DISP_X * zoom / ratio) + gapY,
		SWP_NOMOVE);
}

// ----------------------------------
// Create graphic display for Windows
// ----------------------------------
int GXOpenDisplay(HWND hWnd, int width, int height)
{
  long          Data[256];
  unsigned long *ColorMask;
  BITMAPINFO    *BitmapInfo;
  HDC           hDC = GetDC(hWnd);

  // For Win32, create a DIB section compatible with original main game window DC.
  BitmapInfo                            = (BITMAPINFO *)Data;
  BitmapInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
#ifdef __USE_32BITS__
  BitmapInfo->bmiHeader.biBitCount      = 32;
#else
  BitmapInfo->bmiHeader.biBitCount      = 16;
#endif
  BitmapInfo->bmiHeader.biWidth         = width;
  BitmapInfo->bmiHeader.biHeight        = -height;
  BitmapInfo->bmiHeader.biPlanes        = 1;
  BitmapInfo->bmiHeader.biCompression   = BI_BITFIELDS;

#ifdef __USE_32BITS__
  BitmapInfo->bmiHeader.biSizeImage     = width * height * 4;
#else
  BitmapInfo->bmiHeader.biSizeImage     = width * height * 2;
#endif
  BitmapInfo->bmiHeader.biClrUsed       = 0;
  BitmapInfo->bmiHeader.biClrImportant  = 0;
  BitmapInfo->bmiHeader.biXPelsPerMeter = 0;
  BitmapInfo->bmiHeader.biYPelsPerMeter = 0;
  ColorMask                             = (unsigned long *)&BitmapInfo->bmiColors[0];
#ifdef __USE_32BITS__
  ColorMask[0]                          = 0xFF0000;
  ColorMask[1]                          = 0xFF00;
  ColorMask[2]                          = 0xFF;
#else
  ColorMask[0]                          = 0xF800;
  ColorMask[1]                          = 0x7E0;
  ColorMask[2]                          = 0x1F;
#endif
  hOffscreenBitmap                      = CreateDIBSection(hDC, BitmapInfo, DIB_RGB_COLORS, &OffscreenBitmap.bmBits, NULL, 0);
  OffscreenBitmap.bmWidth               = width;
  OffscreenBitmap.bmHeight              = height;
#ifdef __USE_32BITS__
  OffscreenBitmap.bmBitsPixel           = 32;
#else
  OffscreenBitmap.bmBitsPixel           = 16;
#endif
  OffscreenBitmap.bmPlanes              = 1;

  // And create a memory DC used to blit offscreen bitmap to main window.
  hDCMem = CreateCompatibleDC(hDC);
  ReleaseDC(hWnd, hDC);
  return 1;
}

// -------------------------------
// Display close, free everything.
// -------------------------------
int GXCloseDisplay(void)
{
  DeleteDC(hDCMem);
  DeleteObject(hOffscreenBitmap);
  return 1;
}

#endif /* USE_OGL */


// --------------------------------------------------------
// Resize window
// --------------------------------------------------------

void Resize(int dispX, int dispY)
{
	m_dispX = dispX;
	m_dispY = dispY;

	RECT rc, client;
	GetWindowRect(MainWindow, &rc);
	GetClientRect(MainWindow, &client);
	int gapX = (rc.right-rc.left) - (client.right-client.left);
	int gapY = (rc.bottom-rc.top) - (client.bottom-client.top);
	float ratio = (float)WINDOW_RESIZE_DISP_X / WINDOW_RESIZE_DISP_Y;

	SetWindowPos(
		MainWindow,
		0,
		0,
		0,
		(WINDOW_RESIZE_DISP_X * nZoom) + gapX,
		(int)(WINDOW_RESIZE_DISP_X * nZoom / ratio) + gapY,
		SWP_NOMOVE);

#ifndef USE_OGL
	GXCloseDisplay();
	GXOpenDisplay(MainWindow, WINDOW_RESIZE_DISP_X, WINDOW_RESIZE_DISP_Y);
#endif /* !USE_OGL */
}

// --------------------------------------------------------
// Display update, display next frame composed into VScreen
// --------------------------------------------------------
#ifdef USE_OGL
//...
//EGLContext g_EGLContext;
//EGLDisplay g_EGLDisplay;
//EGLSurface g_EGLSurface;

GLint InitOpenGLESWindow(GLvoid);
GLvoid CloseOpenGLESWindow(GLvoid);
GLvoid OGLRefresh3D();

//temporary buffer ... needed to build the screen texture
u8 g_ScreenBuffer[ 3 * MAX_TEX_WIDTH * MAX_TEX_HEIGHT];
float g_ScreenTextureUV[4];

GLuint g_glScreenTextureName = 0;

// rax: moved in project_settings.h
//	 0xF800	 0x07E0		0x1F
//	(1111 1)(111 111)(1 1111)
//#define CONVERT_565_TO_888(x)	( \
//								( ( (x) & 0xF800 ) << 8 ) | \
//								( ( (x) & 0x07E0 ) << 5 ) | \
//								( ( (x) & 0x001F ) << 3 ) \
//								) 
//
//#define CONVERT_444_TO_888(x)	( ( ( (x) & 0x0F00 ) << 12 ) | ( ( (x) & 0x00F0 ) << 8 ) | ( ( (x) & 0x000F ) << 4 ) ) 


void PCDisplayUpdate(void)
{	
	g_lib3DGL->Flush2D();

	return;
}

#else /* USE_OGL */

#ifdef __USE_32BITS__

void PCDisplayUpdate(void)
{
  SelectObject(hDCMem, hOffscreenBitmap);        // Blit from memory DC to game window DC.
  unsigned int *VirtualScreen  = (unsigned int *)OffscreenBitmap.bmBits;

  for (int y=0; y<DISP_Y; y++)
  {
    for (int x=0; x<DISP_X; x++)
    {
        unsigned int s = VScreen[y*DISP_X+x];

        for(int nVertScale = 0; nVertScale < SCREEN_SCALE_FACTOR; ++nVertScale)
        {
            for(int nHorizScale = 0; nHorizScale < SCREEN_SCALE_FACTOR; ++nHorizScale)
            {
                VirtualScreen[(y*SCREEN_SCALE_FACTOR+nVertScale)*WINDOW_DISP_X+(x*SCREEN_SCALE_FACTOR+nHorizScale)] = s;//((s & 0xF800) << 8) | ((s & 0x7E0) << 5) | ((s & 0x1F) << 3);;
            }
        }
    }
  }

/*  if (Gapi.m_nStartVibra > 0)
  {
	  Gapi.m_nStartVibra -= 29;
  }*/
  	RECT rect;
	GetClientRect(MainWindow, &rect);

	StretchBlt(GameWindowDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hDCMem, 0, 0, WINDOW_DISP_X, WINDOW_DISP_Y, SRCCOPY);
  //BitBlt(GameWindowDC, 0, 0, WINDOW_DISP_X, WINDOW_DISP_Y, hDCMem, 0, 0, SRCCOPY);
}

#else

void PCDisplayUpdate(void)
{
	int disp_x = CHighGear::GetInstance()->m_dispX;
	int disp_y = CHighGear::GetInstance()->m_dispY;

	if (m_dispX != disp_x || m_dispY != disp_y)
	{
		Resize(disp_x, disp_y);
	}
  
	unsigned short *VirtualScreen  = (unsigned short *)OffscreenBitmap.bmBits;
	memset(VirtualScreen,0,2 * WINDOW_DISP_X * WINDOW_DISP_Y);

	memcpy(VirtualScreen, VScreen, disp_y * disp_x * 2);

  	RECT rect;
	GetClientRect(MainWindow, &rect);

	SelectObject(hDCMem, hOffscreenBitmap);        // Blit from memory DC to game window DC.
	StretchBlt(GameWindowDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hDCMem, 0, 0, WINDOW_RESIZE_DISP_X, WINDOW_RESIZE_DISP_Y, SRCCOPY);
	//SetTextAlign(GameWindowDC, TA_UPDATECP);


  //BitBlt(GameWindowDC, 0, 0, WINDOW_DISP_X, WINDOW_DISP_Y, hDCMem, 0, 0, SRCCOPY);
}
#endif /* __USE_32BITS__ */

#endif /* USE_OGL */

// -----------------------------------------
static int g_blurSteps = 0;

// Main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    /* Initialize defaults, Video and Audio */
    //if((SDL_Init(SDL_INIT_AUDIO)==-1)) {
    //    printf("Could not initialize SDL: %s.\n", SDL_GetError());
    //    exit(-1);
    //}

  MSG  msg;
  WNDCLASS WinClass;

  /*HWND h = FindWindow( szAppName, szTitle );
  if(h)
  {
    	SetForegroundWindow(h);
	    return 0;
  }*/


  hInst = hInstance;                        // Store hInstance for future use.

  if (!hPrevInstance)                       // Window class init and registration.
  {
    WinClass.style         = CS_HREDRAW | CS_VREDRAW;
    WinClass.lpfnWndProc   = MainProc;
    WinClass.cbClsExtra    = 0;
    WinClass.cbWndExtra    = 0;
    WinClass.hInstance     = hInstance;
    WinClass.hIcon         = NULL;
	WinClass.hCursor       = LoadCursor(NULL, IDC_ARROW);//NULL;
    WinClass.hbrBackground = NULL;
    WinClass.lpszMenuName  = NULL;
    WinClass.lpszClassName = szAppName;

    if (!RegisterClass(&WinClass))   // if can't register window class then quit program.
     return 0;
  }

  //set the portrait orientation
  #ifdef AUTODETECT_ORIENTATION
  SetDefaultOrientation();
  #endif

  // ---------------------------------
  // Create window centered on desktop

  // ---------------------------
  // get size of Windows desktop
  int dx, dy;
  RECT DeskRect;
  HWND Desktop = GetDesktopWindow();
  if (!GetClientRect(Desktop, &DeskRect))
    return 0;
  dx = DeskRect.right - DeskRect.left;
  dy = DeskRect.bottom - DeskRect.top;

  // ----------------------------------------
  // create Ui app window centered on desktop
  int CenterX = (dx - WINDOW_DISP_X)/2;
  int CenterY = (dy - WINDOW_DISP_Y)/2 - 100;

  DWORD WStyle = WS_VISIBLE;//WS_SIZEBOX;

  m_dispX = DISP_X;
  m_dispY = DISP_Y;

  // -------------
  // Create window
  MainWindow = CreateWindow(
     szAppName,
     szTitle,
     WStyle,
     CenterX,
     CenterY,
     WINDOW_DISP_X,                                    // default orientation is horizontal
     WINDOW_DISP_Y,
     NULL,
     NULL,
     hInstance,
     NULL);

  if (!MainWindow)
    return 0;

  // get drawable size (depend of window border width)
  RECT WinRect;
  if (!GetClientRect(MainWindow, &WinRect))
    return 0;

  // get size of created window
  dx = WinRect.right - WinRect.left;
  dy = WinRect.bottom - WinRect.top;

  // adjust size
  if ((dx < WINDOW_DISP_X) || (dy < WINDOW_DISP_Y))
  {
    DestroyWindow(MainWindow);

    MainWindow = CreateWindow(
       szAppName,
       szTitle,
       WStyle,
       CenterX,
       CenterY,
       WINDOW_DISP_X + (WINDOW_DISP_X - dx),
       WINDOW_DISP_Y + (WINDOW_DISP_Y - dy),
       NULL,
       NULL,
       hInstance,
       NULL);

    if (!MainWindow)
      return 0;
  }

#ifdef USE_OGL
  InitOpenGLESWindow();
#endif

  Gapi.mRunFlags = 0;

  char *p = (char *) lpCmdLine;
  char arg[128];
  while (*p)
  {
	  sscanf(p, "%s", arg);
	  if (!stricmp(arg, "ni"))
		  Gapi.mRunFlags |= Gapi.RUN_NO_INTERFACE;
	  if (!stricmp(arg, "nbg"))
		  Gapi.mRunFlags |= Gapi.RUN_NO_BG_OBJECTS;

	  p += strlen(arg);
  }

  Zoom(1);

  ShowWindow(MainWindow, SW_SHOWNORMAL);             // Show window

  // -------------
  // Gapi Init
  //memset(&Gapi, 0, sizeof(CGapi));

#ifdef USE_OGL
	VScreen = NULL;
#else //USE_OGL
	#ifdef __USE_32BITS__
		VScreen = new unsigned int[DISP_X*DISP_Y];
	#else
		VScreen = new unsigned short[DISP_X * DISP_Y];
	#endif
#endif //USE_OGL

  Gapi.ScreenPtr = (unsigned short*)VScreen;
#ifdef __USE_32BITS__
	Gapi.m_displayMode = VIDEO_MODE_32BPP;
#else
	Gapi.m_displayMode = VIDEO_MODE_16BPP;
#endif

	strcpy(Gapi.DataPath, ".\\");

  Gapi.PathLen = strlen(Gapi.DataPath);          // size of path in DataPath
#ifdef SUPPORT_PROGRAM_ARGUMENT
  Gapi.m_strProgramArgument = lpCmdLine;
#endif

#ifdef SUPPORT_CLIPBOARD
  Gapi.m_hWnd = reinterpret_cast<void*>(MainWindow);
#endif

  Gapi.TickCount256 = GetTickCount() << 8;   // in ms * 256
  if (Gapi.InitGame(0))                           // Main game init
  {
	  //TEST
	//Gapi.ShowNokia();
	Gapi.m_nSignalStrength = 7; // 0 to 7... 0 means there's no GSM network available

    SysTickLast = GetTickCount();
    while (!Gapi.m_bQuit)
    {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        if (msg.message == WM_QUIT)
          Gapi.Quit();
        else
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      else
      {
#if defined(HAS_MULTIPLAYER) && defined (__FAKE_SYMBIAN32__)
		if ((Gapi.m_pHighGear) && (!Gapi.m_pHighGear->m_bWaitingForMpMessages))
		{
#endif // HAS_MULTIPLAYER && __FAKE_SYMBIAN32__

        // update time counters
        unsigned int SysTickCurrent;//, dt;

        SysTickCurrent = GetTickCount();
        if (SysTickCurrent < SysTickLast)        // loop occured
        {
          SysTickLast = 0;
          continue;
        }

		if((SysTickCurrent - SysTickLast ) < (unsigned int)(1000.0f / FPS_LIMIT) )
		{
			Sleep((unsigned int)(1000.0f / FPS_LIMIT) - SysTickCurrent + SysTickLast);
			SysTickCurrent = GetTickCount();
		}

		g_uiFrameDuration = SysTickCurrent - SysTickLast;
        SysTickLast = SysTickCurrent;
        Gapi.TickCount256 = SysTickCurrent << 8;   // in ms * 256

#if defined(HAS_MULTIPLAYER) && defined (__FAKE_SYMBIAN32__)	
		}	
#endif // HAS_MULTIPLAYER && __FAKE_SYMBIAN32__
		if (!m_bNoFocus)
		{
		#ifdef BLACK_SCREEN
			UpdateOrientation(MainWindow);
			if (!LandscapeMode)
		#endif
			{
				if((!m_Paused) || m_Step)
				{	
					
					int disp_x = CHighGear::GetInstance()->m_dispX;
					int disp_y = CHighGear::GetInstance()->m_dispY;

					if (m_dispX != disp_x || m_dispY != disp_y)
					{
						Resize(disp_x, disp_y);						
					}	
				#ifdef USE_OGL
					
					int viewportWidth = CHighGear::GetInstance()->m_dispX;
					int viewportHeight = CHighGear::GetInstance()->m_dispY;					

					g_sceneViewportW = viewportWidth;
					g_sceneViewportH = viewportHeight;

				#ifdef USE_RENDER_INTO_TEXTURE										
					bool renderingWithBlur = g_bIsBlurActivated;
					//TEST
					//if(renderingWithBlur && CHighGear::GetInstance()->m_state != CHighGear::gs_ingame_menu)
					if(renderingWithBlur)
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
				#endif /* USE_OGL */

					Gapi.LoopGame();                       // update game	
				
				#ifdef USE_RENDER_INTO_TEXTURE
					if(IsGLLinkedToScreenBuffer())
					{						
						RestoreGLLinkage();	
			
						if(renderingWithBlur)
						{
						
							//TEST
							//if(CHighGear::GetInstance()->m_PlayingGame != NULL && (CHighGear::GetInstance()->m_PlayingGame->GetPlayerCar()->turbo_time <  ( (CAR_TURBO_TIME * 5 ) / 6)) )
							//   g_blurSteps++;
							//else
							//   g_blurSteps--;
							   
							if(g_blurSteps > 8)
								g_blurSteps = 8;
							if(g_blurSteps < 0)
							   g_blurSteps = 0;
							   
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

					PCDisplayUpdate();                     // blit screen PC						

				#ifdef USE_OGL
					if(!g_bSkipRendering)
					{
						//eglSwapBuffers(g_EGLDisplay, g_EGLSurface);
						SwapBuffers(GameWindowDC);
					}
				#endif /* USE_OGL */
					m_Step=false;
				}
			}	
		}

		//Sleep(20);		//need this here so phone can catch intrerupts
	  }
    }
    Gapi.EndGame();
//	MM_DELETE CMemoryManager::GetInstance();
  }

#ifdef USE_OGL
  //...
  CloseOpenGLESWindow();

  g_lib3DGL->Clean3D();
  SAFE_DELETE(g_lib3DGL);

#else /* USE_OGL */
  GXCloseDisplay();                              // Close GAPI display driver
#endif /* USE_OGL*/

  ReleaseDC(MainWindow, GameWindowDC);
  DestroyWindow(MainWindow);
  InvalidateRect(NULL, NULL, TRUE);
#ifdef AUTODETECT_ORIENTATION
  SetInitialOrientation();
#endif

  delete[] VScreen;

  return(msg.wParam);                            // Exit
}

// ------------------------------
// Main window message procedure.
//-------------------------------
LRESULT CALLBACK MainProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_TOUCH_SCREEN
	static CTouchZones* touchZones = CTouchZones::GetInstance();
#endif //USE_TOUCH_SCREEN

	static bool mouseLButtHeld = false;
	switch (Message)
	{
		//salcideon use mouse
		case WM_LBUTTONDOWN:
		{
			short x = (0xFFFF & lParam);
			short y = (lParam>>16);

#ifdef USE_TOUCH_SCREEN
			//TEST
			//if (Gapi.m_pHighGear->m_state != CHighGear::gs_suspended)
				touchZones->TouchPressed(0, x, y);
#endif // USE_TOUCH_SCREEN

			mouseLButtHeld = true;
			break;
		}

		case WM_LBUTTONUP:
		{
			short x = (0xFFFF & lParam);
			short y = (lParam>>16);

#ifdef USE_TOUCH_SCREEN
			//TEST
			//if (Gapi.m_pHighGear->m_state != CHighGear::gs_suspended)
				touchZones->TouchReleased(0, x, y);
#endif // USE_TOUCH_SCREEN

			mouseLButtHeld = false;
			break;
		}

		case WM_MOUSEMOVE:
		{
			short x = (0xFFFF & lParam);
			short y = (lParam>>16);

#ifdef USE_TOUCH_SCREEN
			if (mouseLButtHeld)
				touchZones->TouchMoved(0, x, y);
#endif // USE_TOUCH_SCREEN
			break;
		}

  case WM_CLOSE:
		Gapi.Quit();
	break;

    case WM_CREATE:
#ifdef USE_OGL
	  //...
#else /* USE_OGL */
      GameWindowDC = GetDC(hWnd);
      GXOpenDisplay(hWnd, WINDOW_DISP_X, WINDOW_DISP_Y); // init display
#endif /* USE_OGL */
    break;
    case WM_KEYDOWN:
      wParam &= 0xff;

      Gapi.KeyEvent(true, wParam);
//      Gapi.KeyPressed[wParam] = 1;
//      Gapi.KeyPressedId = wParam;
		switch(wParam)
		{
		case VK_ESCAPE:
			Gapi.Quit();
			break;
		case VK_F1:
			m_Paused=true;
			break;
		case VK_F2:
			m_Paused=true;
			m_Step=true;
			break;
		case VK_F3:
			m_Paused=false;
			break;

		case VK_F5:
		{

			break;
		}
		case VK_F7:
		{
			/// Generate interrupt
			//TEST
			//Gapi.m_pHighGear->Suspend();

			/// Generate phone call interrupt - incoming call
/*
			CHighGear* hg = CHighGear::GetInstance();

			if( !hg->m_bInPhoneCall )
				hg->m_AudioManager.EnterPhoneCall();
*/
			break;
		}
		case VK_F8:
		{
			/// After interrupt
			//TEST
			//Gapi.m_pHighGear->Resume();

			/// Generate phone call interrupt - finished call
/*
			CHighGear* hg = CHighGear::GetInstance();

			if( hg->m_bInPhoneCall )
				hg->m_AudioManager.ExitPhoneCall();
*/
			break;
		}
		case VK_F9:
		{
			int o = Gapi.GetDisplayOrientation();
			Gapi.mDisplayOrientationPending = (o == ORIENTATION_PORTRAIT) ? ORIENTATION_LANDSCAPE_90 : ORIENTATION_PORTRAIT;
			break;
		}
		case VK_OEM_3:
		{
			//TEST
			//CHighGear* hg = CHighGear::GetInstance();

			//if( hg->m_PlayingGame )
			//{
			//	CCar* playercar = hg->m_PlayingGame->GetPlayerCar();
			//	A_ASSERT( playercar != NULL );

			//	playercar->NotifyEvent( PointSystem::k_nCopsTakeDown/*(PointSystem::EEvent)( hg->m_Random.GetNumber( PointSystem::k_nNumEvents ))*/ );
			//	hg->m_PlayingGame->SpawnCopBehind( playercar, 2, 2048, CCar::COP_TYPE_HELICOPTER );
			//}

			break;
		}
		case VK_ADD:
			Zoom(nZoom << 1);
			break;
		case VK_SUBTRACT:
			Zoom(nZoom >> 1);
			break;
		}

    break;

    case WM_KEYUP:
      wParam &= 0xff;
      Gapi.KeyEvent(false, wParam);
//      Gapi.KeyPressed[wParam] = 0;
    break;

    case WM_SIZE:
      return 0;
    break;

	case WM_KILLFOCUS:
		break;

	case WM_SETFOCUS:
		break;


    default:
      return DefWindowProc(hWnd, Message, wParam, lParam);
  }
  return 0;
}

unsigned long GRGetTickCount()
{
	return GetTickCount();
}

unsigned long GRGetTickCountPeriod()
{
	// one tick = 1000 microSeconds
	return 1000;
}

#ifdef DEBUG_OUT_ENABLED
void debug_out(const char* fmt, ...)
{
//#pragma message ("WARNING: DEBUG_OUT_ENABLED")
	FILE* s_LogFile = NULL;

	va_list arg_list;
	va_start(arg_list, fmt);

	if(s_LogFile == NULL)
	{
		s_LogFile = fopen("asphalt.txt","a");
	}

	if(s_LogFile != NULL)
	{
		vfprintf(s_LogFile, fmt, arg_list);
		fprintf(s_LogFile, "\n");
		fflush(s_LogFile);
		fclose(s_LogFile);
		s_LogFile = NULL;
	}
}

#else

void debug_out(const char* x, ...){}

#endif	//DEBUG_OUT_ENABLED


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////OpengGL specific////////////////////////////////////////////////

#ifdef USE_OGL

//@link ogl to window
GLint InitOpenGLESWindow(GLvoid)
{
	//LINK GL TO RENDERING CONTEXT
	//EGLDisplay eglDisplay = eglGetDisplay(0);

	//EGLint versMajor;
	//EGLint versMinor;
	//EGLBoolean result = eglInitialize(eglDisplay, &versMajor, &versMinor);

	//if (result)
	//{
	//	debug_out("GL version %d.%d\n", versMajor, versMinor);

	//	EGLint configAttribs[128];
	//	int i = 0;
	//	configAttribs[i++] = EGL_RED_SIZE;
	//	configAttribs[i++] = 8;
	//	configAttribs[i++] = EGL_GREEN_SIZE;
	//	configAttribs[i++] = 8;
	//	configAttribs[i++] = EGL_BLUE_SIZE;
	//	configAttribs[i++] = 8;
	//	configAttribs[i++] = EGL_ALPHA_SIZE;
	//	configAttribs[i++] = 8;
	//	configAttribs[i++] = EGL_SURFACE_TYPE;
	//	configAttribs[i++] = EGL_WINDOW_BIT;
	//	configAttribs[i++] = EGL_NONE;

	//	int numConfigs;
	//	EGLConfig eglConfig;

	//	if (!eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs) || (numConfigs != 1))
	//	{
	//		debug_out("failed to find usable config =(\n");
	//	}
	//	else
	//	{
	//		GLint configID;
	//		eglGetConfigAttrib(eglDisplay, eglConfig, EGL_CONFIG_ID, &configID);
	//	}
	//	
	//	//create elgSurface for the current display and config
	//	EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, MainWindow, NULL);

	//	if (eglSurface == EGL_NO_SURFACE)
	//		debug_out("failed to create surface =(, %04x\n", eglGetError());

	//	//obtain eglcontext
	//	EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, 0);

	//	if (eglContext == EGL_NO_CONTEXT)
	//		debug_out("failed to allocate context\n");

	//	g_EGLDisplay = eglDisplay;
	//	g_EGLContext = eglContext;
	//	g_EGLSurface = eglSurface;

	//	if (!eglMakeCurrent(g_EGLDisplay, g_EGLSurface, g_EGLSurface, g_EGLContext))
	//		debug_out("make current error: %04x\n", eglGetError());
	//		
	//	debug_out("SUCCESS InitEGL\n");
	//}

	//CHOOSE APPROPRIATE PIXEL FORMAT	
	GameWindowDC = GetDC(MainWindow);
	PIXELFORMATDESCRIPTOR	pfd;

	pfd.nSize		=	sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion	=	1;
	pfd.dwFlags		=	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType	=	PFD_TYPE_RGBA;
	pfd.cColorBits	=	32;
	pfd.cRedBits	=	0;
	pfd.cRedShift	=	0;
	pfd.cGreenBits	=	0; 
	pfd.cGreenShift	=	0; 
	pfd.cBlueBits	=	0; 
	pfd.cBlueShift	=	0; 
	pfd.cAlphaBits	=	0; 
	pfd.cAlphaShift	=	0; 
	pfd.cAccumBits		=	0; 
	pfd.cAccumRedBits	=	0; 
	pfd.cAccumGreenBits	=	0; 
	pfd.cAccumBlueBits	=	0; 
	pfd.cAccumAlphaBits	=	0; 
	pfd.cDepthBits		=	24; //Use 16 for antialiased rendering
	pfd.cStencilBits	=	8; 
	pfd.cAuxBuffers		=	0; 
	pfd.iLayerType		=	0; 
	pfd.bReserved		=	0; 
	pfd.dwLayerMask		=	0; 
	pfd.dwVisibleMask	=	0; 
	pfd.dwDamageMask	=	0; 

	int ret=::ChoosePixelFormat( GameWindowDC, &pfd );

	if(ret < 1)
		return 0;

	SetPixelFormat( GameWindowDC, ret, &pfd );

	GameWindowGLRC=wglCreateContext( GameWindowDC );
	
	wglMakeCurrent( GameWindowDC, GameWindowGLRC);

	
	//openGL specific
	::glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Black Background
	::glClearDepthf(1.0f);									// Depth Buffer Setup
	::glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	::glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	LinkOES2EXT();	
	
	g_lib3DGL = new Lib3DGL();
	g_lib3DGL->Init3D();	

#ifdef USE_RENDER_INTO_TEXTURE
	InitScreenBufferIndices();
	g_texScreenBuffer = new TextureFBO(TEX_SCREEN_BUFFER_W, TEX_SCREEN_BUFFER_H, false);
	//::glBindTexture(GL_TEXTURE_2D, g_texScreenBuffer->m_glTextureName);
	//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif //USE_RENDER_INTO_TEXTURE
	
	return 1;
}

//@detach ogl from window
GLvoid CloseOpenGLESWindow(GLvoid)
{
	//to do
	//eglMakeCurrent(g_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	//eglTerminate(g_EGLDisplay);

#ifdef USE_RENDER_INTO_TEXTURE
	delete g_texScreenBuffer;
#endif

	if (GameWindowGLRC)	
	{
		wglMakeCurrent(NULL, NULL);			
		wglDeleteContext(GameWindowGLRC);
		GameWindowGLRC=NULL;				
	}

}

//@refresh ogl window
GLvoid OGLRefresh3D()
{
	::glEnable(GL_DEPTH_TEST);
	::glDepthMask(TRUE);
	::glDepthFunc(GL_LEQUAL);
	::glClearDepthf(1.0f);
	::glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//g_lib3DGL->GLEnableFog(GL_LINEAR, 0.0f, 14000.0f, 42000.0f, 0.73f, 0.65f, 0.57f, 1.0f);
	//g_lib3DGL->GLEnableFog(GL_LINEAR, 0.0f, 8000.0f, 42000.0f, 0.73f, 0.65f, 0.57f, 1.0f);
}

#endif /* USE_OGL */
