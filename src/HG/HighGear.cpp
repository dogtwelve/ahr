#include "A4_3d_platform_def.h"

#include "HighGear.h"

#include "GenDef.h"
#include "time.h"

#include "lib3D.h"
#include "File.h"
#include "Intl.h"

#include "sprites.h"


#include "ArchivedFileManager.h"

#include "lib2d/AuroraSprite.h"
#include "lib2d/FontWrapper.h"

#include "config.h"

#ifdef CHECK_MEMORY_LEAKS
#include "MemoryManager.h"
#endif
#include "Sound/SoundManager.h"



/// NOTE: Next file was generated from 'MakeSounds.bat', which is in data directory.
#include "SoundsTable.h"


#ifdef USE_IPHONE_INTRO_VIDEO
	#include "IphoneUtil.h"
	#include "iPhone/Sound/SoundEngine.h"
#endif


#ifdef HAS_MULTIPLAYER
#include "MultiPlayer/Comms.h"

#ifdef HAS_BLUETOOTH_NGAGE
	#include "MultiPlayer/BluetoothNgage.h"
#endif // HAS_BLUETOOTH_NGAGE

#endif // HAS_MULTIPLAYER


#ifdef IPHONE
extern void GetDeviceLanguage(char *lang);
#endif

#include "ScreenBufferWrapper.h"

extern bool g_bSkipRendering;

#if (defined(WIN32) || defined(__WINS__))  && !defined(__BREW__)
# define for if (false) ; else for
#endif

#define LOGO_BACKGROUND_COLOR	0xFFFFFF
#define LOGO_ALPHA_FRAME	500
#define LOGO_START_FRAME	100
#define LOGO_TOTAL_FRAME	1200

#define LOADING_BACKGROUND_COLOR	0x888888


CHighGear::CHighGear(CGapi& gapi) :
	    m_gapi(gapi),

    m_lib3D(0),
	m_lib2D(0),


	m_isInPhoneCall(false),
	m_bStartUpOrientationOverride(0)

{

#ifdef AUTODETECT_PHONE_RESOLUTION
	m_dispX = initialDisplayOrientationX;
	m_dispY = initialDisplayOrientationY;
#else
	m_dispX = INITIAL_DISP_X;
	m_dispY = INITIAL_DISP_Y;
#endif



	m_lib2D = NEW CLib2D(gapi.ScreenPtr, m_dispX, m_dispY, *m_lib3D);

	m_screenImage2D = new Image(m_dispX, m_dispY, gapi.ScreenPtr);

	
#ifdef USE_TOUCH_SCREEN
	touchZones = CTouchZones::GetInstance();
#endif // USE_TOUCH_SCREEN
	

#ifdef USE_BACK_LIGHT
	m_pLight = NULL;
	m_bChargerConected = false;
#endif
	
	m_gState = GAME_STATE_INIT;
}


//BBucur.15.09.2004. Commented parts for debug. Uncomment all.
CHighGear::~CHighGear()
{
	MM_DELETE m_lib3D;
	MM_DELETE m_lib2D;
	MM_DELETE m_screenImage2D;


#ifdef USE_BACK_LIGHT
	SAFE_DELETE(m_pLight);
#endif
}

// Returns true when the loading is done
bool CHighGear::InitHighGear(int step)
{
	switch (step)
	{
	case 0:
		CArchivedFileManager::GetInstance()->CreateAllFileMappingTable();
		break;

	case 1:
		// Lib3D
		m_lib3D = NEW Lib3D::CLib3D;
		m_lib3D->Init();// init renderer matrix stack
		m_lib2D->SetLib3D(m_lib3D);

		m_lib3D->m_board3d->m_screen = m_lib2D->GetDestPtr();

		m_lib3D->m_screenImage3D = m_screenImage2D;
		m_screenImage3D = m_lib3D->m_screenImage3D;


		break;

	case 2:

		break;

	case 3:
		{
			/// Turn on backlight
			Gapi().TurnOnBackLight();

	//		LoadSplashImageBuffers();
		}
		break;

	case 4:

		break;


	case 5:


	case 6:

		return true;
	}


	return false;
}

#ifdef IPHONE
extern "C" void setStatusBarOrientation(int orient);
#endif

bool CHighGear::InitDisplayOrientation(int orientation)
{
	debug_out("orientation %d %d\n",orientation,Gapi().GetDisplayOrientation());

	if (orientation == Gapi().GetDisplayOrientation())
	{
		return true;
	}

//SEFU ADD
#ifdef ORINETATION_CHANGE_KEYS
	if(Gapi().m_orientationOptionsInMenus)
	{
		switch(orientation)
		{
			case ORIENTATION_PORTRAIT:
				SetKeyBoardConfig(0);
				break;
			case ORIENTATION_LANDSCAPE_90:
				SetKeyBoardConfig(1);
				break;
			case ORIENTATION_LANDSCAPE_270:
				SetKeyBoardConfig(2);
				break;
		}
	}
#endif

	if(orientation == INITIAL_ORIENTATION)
	{
#ifdef AUTODETECT_PHONE_RESOLUTION
		m_dispX = initialDisplayOrientationX;
		m_dispY = initialDisplayOrientationY;
#else
		m_dispX = INITIAL_DISP_X;
		m_dispY = INITIAL_DISP_Y;
#endif
	}
	else
	{
#ifdef AUTODETECT_PHONE_RESOLUTION
		m_dispX = initialDisplayOrientationY;
		m_dispY = initialDisplayOrientationX;
#else
		m_dispX = INITIAL_DISP_Y;
		m_dispY = INITIAL_DISP_X;
#endif
	}

	Gapi().mCurrentOrientation = orientation;


	if (m_lib3D)
	{
		// Recalc Fov, yeah it's stupid way, but it checks if the value is different
		int fov = m_lib3D->GetFoV();
		m_lib3D->SetFov(fov+1);
		m_lib3D->SetFov(fov);

		if(m_lib3D->m_board3d)
		{
			//m_lib3D->m_board3d->recalcScreenPtr();
#if USE_STENCIL_BUFFER
			m_lib3D->m_board3d->ClearStencil(true);
#endif
		}

		m_lib3D->m_screenImage3D->changeSize(m_dispX, m_dispY);
	}


	m_screenImage2D->changeSize(m_dispX, m_dispY);

	if (m_lib2D)
	{
		m_lib2D->ResetClip();
		m_lib2D->SetDestPtr(m_lib2D->GetDestPtr(), m_dispX, m_dispY);
		m_lib2D->ResetClip();
	}

#ifndef USE_OGL
//	if (m_PlayingGame && m_PlayingGame->m_Map)
//	{
//		CCar* playerCar = m_PlayingGame->GetPlayerCar();
//
//		if (playerCar)
//			playerCar->recalculateEnvMap();
////SEFU 
//		if (m_gState == gs_play && !Gapi().m_orientationOptionsInMenus)
//		{
//			m_PlayingGame->RecreateInGameMenu();
//		}
//		if (m_gState == gs_ingame_menu)
//		{
//			//LAngelov: we must recreate InGame Menu
//			m_PlayingGame->RecreateInGameMenu();
//		}
//	}
#endif // USE_OGL



// rax - moved above, needed for IGM menu arrow (its pos is changed based on m_Profile.orientation)
//	Gapi().mCurrentOrientation = orientation;
//	m_Profile.orientation = orientation;


	debug_out("IDO:2\n");
	return true;
}

// global inits
int CHighGear::GameInit(void)
{
    return 1;
}


/////////////////////////////////////////////////////////////////////////////////
extern void OGLRefresh3D();



void CHighGear::GameLoop(void)
{
	switch( m_gState )
	{
	case GAME_STATE_INIT:
		InitHighGear(0);
		InitHighGear(1);
		InitHighGear(2);
		InitHighGear(3);

		g_bRefreshScreenBuffer = true;

		//m_logoSprite = NEW CSprite("textures\\gameloft.bsp");
		setGameState(GAME_STATE_LOGO);
		m_loadingLength = 1;
		
		break;
	case GAME_STATE_LOADING:
		procLoading();
		break;
	case GAME_STATE_LOGO:
	{
		GetLib2D().DrawRect(0, 0, m_dispX, m_dispY, LOGO_BACKGROUND_COLOR);
		int alpha = 255;
		if (GETTIMEMS() - m_logoTimer < LOGO_START_FRAME)
			alpha = 0;
		else if (GETTIMEMS() - m_logoTimer < LOGO_ALPHA_FRAME + LOGO_START_FRAME)
			alpha = (GETTIMEMS() - m_logoTimer - LOGO_START_FRAME) * 255 / LOGO_ALPHA_FRAME;
		m_logoSprite->SetGlobalAlpha(alpha);
		GetLib2D().setColor(LOGO_BACKGROUND_COLOR | (alpha<<24) );
		m_logoSprite->DrawModule(GetLib2D(), m_dispX >> 1, m_dispY >> 1, IS_PORTRAIT ? GAMELOFT_F_PORTRAIT : GAMELOFT_F_LANDSCAPE);
		
		if (GETTIMEMS() - m_logoTimer > LOGO_TOTAL_FRAME)
		{
			delete(m_logoSprite);
			setGameState(GAME_STATE_TITLE);
			m_loadingLength = 1;
		}
		break;	
	}
	case GAME_STATE_MAIN:
		procMaingame();
		break;
	case GAME_STATE_TITLE:
		m_SplashScreen->DrawFrame(GetLib2D(), 0, 0, 0);
		if (Gapi().m_Keypad.Keypad_HasAnyKeyBeenPressed() 
#ifdef USE_TOUCH_SCREEN
				|| touchZones->WasZoneActivated(0)
#endif // USE_TOUCH_SCREEN
				)
		{
			//setGameState(GAME_STATE_MENU);
			setGameState(GAME_STATE_MAIN, true);
		}
		break;
	case GAME_STATE_MENU:
		m_SplashScreen->DrawFrame(GetLib2D(), 0, 0, 0);
		break;
	}

	m_FrameCounter++;

#ifdef USE_TOUCH_SCREEN
//	if (m_gState != gs_play
//		
//		&& m_gState != gs_suspended)
//	{
		touchZones->Update();
	#ifdef DRAW_TOUCH_ZONES
		touchZones->DrawZones(GetLib2D());
	#endif // DRAW_TOUCH_ZONES
//	}
#endif // USE_TOUCH_SCREEN


}




#ifdef USE_TOUCH_SCREEN
void CHighGear::ZonePressed(short zoneId)
{
	return;
}

void CHighGear::ZoneReleased(short zoneId)
{
	return;
}

void CHighGear::ZoneActivated(short zoneId)
{
	return;
}

void CHighGear::ZoneMove(short zoneId, int x, int y)
{
	return;
}
#endif // USE_TOUCH_SCREEN

void CHighGear::setGameState(e_m_gState state, bool bDrawL)
{
	m_gState = GAME_STATE_LOADING;
	m_gStateNext = state;
	m_bDrawLoading = bDrawL;
	m_loadingCounter = 0;
}


void CHighGear::procLoading()
{
	if (m_bDrawLoading)
	{
		//Draw LoadingScreen
		GetLib2D().DrawRect(0, 0, m_dispX, m_dispY, LOADING_BACKGROUND_COLOR);
	}
#ifdef USE_TOUCH_SCREEN
	if (m_loadingCounter == 0)
		touchZones->ClearZones();
#endif
	switch (m_gStateNext)
	{
	case GAME_STATE_LOGO:
		m_logoSprite = NEW CSprite("logo.bsprite");
		m_logoTimer = GETTIMEMS();
		break;
	case GAME_STATE_TITLE:
		m_SplashScreen = NEW CSprite("splash.bsprite");
		touchZones->AddZone(0, 0, 0, m_dispX, m_dispY);
		break;
	case GAME_STATE_MAIN:
		delete(m_SplashScreen);
		m_bg = NEW CSprite("bg_floor.bsprite");

		m_gameSprite = NEW CSprite*[MAX_GAMESPRITE];
		m_gameSprite[0] = NEW CSprite("mc00.bsprite");
		m_gameSprite[1] = NEW CSprite("enemy_mummy.bsprite");
		m_gameSprite[2] = NEW CSprite("enemy_vampire.bsprite");
		m_gameSprite[3] = NEW CSprite("mcbullet.bsprite");
		m_pad = NEW CSprite("vpad.bsprite");
		touchZones->AddZone(ZONEID_PAD_LEFT, VPAD_X - 10, VPAD_Y + 25, VPAD_X + 30, VPAD_Y + 65); 
		touchZones->AddZone(ZONEID_PAD_RIGHT, VPAD_X + 55, VPAD_Y + 25, VPAD_X + 95, VPAD_Y + 65); 
		touchZones->AddZone(ZONEID_PAD_FIRE, VPAD_FIRE_X, VPAD_FIRE_Y, VPAD_FIRE_X + 40, VPAD_FIRE_Y + 40); 
		initActors();
		break;
	}
	if (++ m_loadingCounter <= m_loadingLength)
		m_gState = m_gStateNext;
}


void CHighGear::initActors()
{
	m_actors = NEW CActor*[MAX_ACTOR];
	for (int i = 0; i < MAX_ACTOR; i ++)
		m_actors[i] = NEW CActor(this);
	//m_actors[0] = NEW CActor();
	m_actors[0]->init(CActor::ACTOR_MC, m_gameSprite[0], MC_XCOORD, MC_YCOORD);

	m_actors[1]->init(CActor::ACTOR_MUMMY, m_gameSprite[1], m_Random.GetNumber(0, 4), 0);
	m_actors[2]->init(CActor::ACTOR_VAMPIRE, m_gameSprite[2], m_Random.GetNumber(0, 4), 0);
}

int CHighGear::getEmptyActorIndex()
{
	for (int i = 0 ; i < MAX_ACTOR; i ++)
		if (m_actors[i]->m_type == CActor::ACTOR_NONE) return i;

	return -1;
}

void CHighGear::procMaingame()
{
	//////////////////////////
	//UPDATE
	//////////////////////////

	//Update Touch
	if (m_actors[0])
	{
		if (touchZones->IsZonePressed(ZONEID_PAD_LEFT))
		{
			m_actors[0]->move(-1);
		}
		else if (touchZones->IsZonePressed(ZONEID_PAD_RIGHT))
		{
			m_actors[0]->move(1);
		}
		else if (touchZones->IsZonePressed(ZONEID_PAD_FIRE))
		{
			//FIRE!
			int index = getEmptyActorIndex();

			if (index > -1)
			{
				m_actors[index]->init(CActor::ACTOR_MCBULLET, m_gameSprite[3], m_actors[0]->m_posX, m_actors[0]->m_posY);
			}
		}

	}

	//UPDATE actors
	for (int i = 0; i < MAX_ACTOR; i ++)
	{
		if (m_actors[i])
		{
			m_actors[i]->update();
			if (m_actors[i]->m_state == CActor::ACTOR_STATE_DESTROYED)
				m_actors[i]->m_type = CActor::ACTOR_NONE;
		}
	}

	//UPDATE COLLIDE
	for (int i = 0; i < MAX_ACTOR; i ++)
	{
		if (m_actors[i])
		{
			if (m_actors[i]->m_type == CActor::ACTOR_MCBULLET)
			{
				for (int j = 0; j < MAX_ACTOR; j ++)
				{
					if (m_actors[j]->m_type != CActor::ACTOR_MUMMY 
						&& m_actors[j]->m_type != CActor::ACTOR_VAMPIRE) continue;

					if (m_actors[j]->m_posX == m_actors[i]->m_posX && 
						m_actors[j]->m_posY == m_actors[i]->m_posY)
					{
						m_actors[i]->m_state = CActor::ACTOR_NONE;	//Bullet X
						m_actors[j]->notifyState(CActor::ACTOR_STATE_DAMAGED);

int index = getEmptyActorIndex();

			if (index > -1)






			{
				m_actors[index]->init(CActor::ACTOR_MCBULLET, m_gameSprite[3], m_actors[0]->m_posX, m_actors[0]->m_posY);
			}
					}
				}
			}
		}
	}

	//genEnemy();
	
	/////////////////////////
	//DRAW
	/////////////////////////

	//BG
	GetLib2D().DrawRect(0, 0, m_dispX, m_dispY, 0xF111, 0xFEEE);
	m_bg->DrawFrame(GetLib2D(), m_dispX >> 1, m_dispY >> 1, 1);


	//Sort Actor's draw order
	int* drawActorSortArray;
	int drawActorCnt = 0;

	drawActorSortArray = new int[MAX_ACTOR * 2];

	for (int i = 0; i < MAX_ACTOR; i ++) drawActorSortArray[i * 2] = -1;

	for (int i = 0; i < MAX_ACTOR; i ++)
	{
		if (!m_actors[i]) continue;
		if (m_actors[i]->m_type == CActor::ACTOR_NONE) continue;
		if (m_actors[i]->spr == NULL) continue;


		for (int j = 0; j < MAX_ACTOR; j ++)
		{
			if (drawActorSortArray[j * 2] == -1 
				|| drawActorSortArray[j * 2 + 1] > m_actors[i]->m_posY)
			{
			//ADD this @drawActorSortArray which index is j
				if (j < drawActorCnt)
				{
					for (int k = drawActorCnt; k >= j; k--)
					{
						drawActorSortArray[(k + 1)* 2] = drawActorSortArray[k * 2];
						drawActorSortArray[(k + 1)* 2 + 1] = drawActorSortArray[k * 2 + 1];
					}
				}

				drawActorSortArray[j * 2] = i;
				drawActorSortArray[j * 2 + 1] = m_actors[i]->m_posY;
				drawActorCnt ++;
				break;
			}
		}
		
	}
	

	for (int i = 0; i < drawActorCnt; i ++)
	{
		m_actors[drawActorSortArray[i * 2]]->draw(GetLib2D());
		/*if (m_actors[i])
		{
			m_actors[i]->draw(GetLib2D());
		}*/
	}

	delete(drawActorSortArray);

	//DRAW VIRTUAL PAD
	m_pad->DrawModule(GetLib2D(), VPAD_X, VPAD_Y, 0);
	m_pad->DrawModule(GetLib2D(), VPAD_FIRE_X, VPAD_FIRE_Y, 1);
}
