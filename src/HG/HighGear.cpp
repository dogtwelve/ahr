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
#define LOGO_FADEIN_FRAME	100
#define LOGO_TOTAL_FRAME	2400
#define LOGO_FADEOUT_FRAME	(LOGO_TOTAL_FRAME - LOGO_ALPHA_FRAME - LOGO_FADEIN_FRAME)

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
	
	m_state = GAME_STATE_INIT;
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
		m_AudioManager.SetSoundStatus(true);

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
//		if (m_state == gs_play && !Gapi().m_orientationOptionsInMenus)
//		{
//			m_PlayingGame->RecreateInGameMenu();
//		}
//		if (m_state == gs_ingame_menu)
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
	switch( m_state )
	{
	case GAME_STATE_INIT:
		InitHighGear(0);
		InitHighGear(1);
		InitHighGear(2);
		InitHighGear(3);
		m_Random.SetSeed(GETTIMEMS());

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
		int cFrame = GETTIMEMS() - m_logoTimer;
		if (cFrame < LOGO_FADEIN_FRAME || cFrame > LOGO_TOTAL_FRAME - LOGO_FADEIN_FRAME)
			alpha = 0;
		else if (cFrame < LOGO_ALPHA_FRAME + LOGO_FADEIN_FRAME)
			alpha = (cFrame - LOGO_FADEIN_FRAME) * 255 / LOGO_ALPHA_FRAME;
		else if (cFrame > LOGO_FADEOUT_FRAME)
			alpha = 255 - ((cFrame - LOGO_FADEOUT_FRAME) * 255 / LOGO_ALPHA_FRAME);
		m_logoSprite->SetGlobalAlpha(alpha);
		GetLib2D().setColor(LOGO_BACKGROUND_COLOR | (alpha<<24) );
		m_logoSprite->DrawModule(GetLib2D(), (m_dispX - m_logoSprite->GetModuleWidth(0)) >> 1 ,
			(m_dispY - m_logoSprite->GetModuleHeight(0)) >> 1, IS_PORTRAIT ? GAMELOFT_F_PORTRAIT : GAMELOFT_F_LANDSCAPE);
		
		if (GETTIMEMS() - m_logoTimer > LOGO_TOTAL_FRAME)
		{
			GetLib2D().setColor(LOGO_BACKGROUND_COLOR | (255<<24) );
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
		GetLib2D().DrawRect(0, 0, m_dispX, m_dispY, 0xF000, 0xF000);
		GetLib2D().setColor(0x33FFFFFF);
		m_bg[0]->DrawModule(GetLib2D(), 0, 0, 0);
		GetLib2D().setColor(0xFFFFFFFF);
		m_SplashScreen->DrawFrame(GetLib2D(), m_dispX >> 1, m_dispY >> 1, 0, CSprite::FLAGS_ADDITIVE_BLENDING);
		m_SplashScreen->DrawFrame(GetLib2D(), m_dispX >> 1, (m_dispY >> 1) + 60, 1, CSprite::FLAGS_ADDITIVE_BLENDING);
		if (Gapi().m_Keypad.Keypad_HasAnyKeyBeenPressed() 
#ifdef USE_TOUCH_SCREEN
				|| touchZones->WasZoneActivated(0)
#endif // USE_TOUCH_SCREEN
				)
		{
			//setGameState(GAME_STATE_MENU);
//JK_SOUND_IMPLEMENT
			m_AudioManager.stopAllSounds();
			setGameState(GAME_STATE_MAIN, true);
		}
		break;
	case GAME_STATE_MENU:
		m_SplashScreen->DrawFrame(GetLib2D(), 0, 0, 0);
		break;
	}

	m_FrameCounter++;

#ifdef USE_TOUCH_SCREEN
//	if (m_state != gs_play
//		
//		&& m_state != gs_suspended)
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
	switch (zoneId)
	{
//		case ZONEID_PLAY_AGAIN_YES:
//			touchZones->ClearZones();
//			initActors();
//			m_gameTime = GETTIMEMS();
//			m_gameState = GAME_READY;
//			resetGame();
//			break;
//		case ZONEID_PLAY_AGAIN_NO:
//			touchZones->ClearZones();
//			setGameState(GAME_STATE_TITLE);
//			break;
		case ZONEID_PAD_LEFT:
			MAINCHAR->move(-1);
			break;
		case ZONEID_PAD_RIGHT:
			MAINCHAR->move(1);
			break;
		case ZONEID_PAD_FIRE:
			
			//if (MAINCHAR->canFire())
			//{
			//	//FIRE!
			//	int index = getEmptyActorIndex();
			//	
			//	if (index > -1)
			//	{
			//		m_actors[index]->init(CActor::ACTOR_MCBULLET, GAMESPRITE_MCBULLET, MAINCHAR->m_posX, MAINCHAR->m_posY);
			//	}
			//	MAINCHAR->notifyState(CActor::ACTOR_STATE_ATTACK, 15 - m_btDelay * 2);
			//}
			break;
	}
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

void CHighGear::setGameState(e_m_state state, bool bDrawL)
{
	m_state = GAME_STATE_LOADING;
	m_stateNext = state;
	m_bDrawLoading = bDrawL;
	m_loadingCounter = 0;
}

void CHighGear::resetGame ()
{
	m_kill = 0;
	m_level = 0;
	m_village = MAX_VILLAGE;
	m_hp = MAX_HP;
	
	//####	bullet levels : 0~4
	m_enemyGenIndex = 1;
	m_btPow = 0;
	m_btDelay = 0;
	
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
	switch (m_stateNext)
	{
	case GAME_STATE_LOGO:
		m_logoSprite = NEW CSprite("sprite\\logo.bsprite");
		m_logoTimer = GETTIMEMS();
		
		break;
	case GAME_STATE_TITLE:
#ifndef WIN32
#define JK_SOUND_IMPLEMENT
#endif
#ifdef JK_SOUND_IMPLEMENT
		m_AudioManager.m_soundWrap->MusicLoad("bg_title.mp3");//("bg_title.mp3");//"sample1.mp3");
		m_AudioManager.m_soundWrap->MusicStart(true);
#endif
		m_SplashScreen = NEW CSprite("sprite\\title.bsprite");
		m_bg = NEW CSprite*[2];

		m_bg[0] = NEW CSprite("sprite\\bg.bsprite");
		m_gameSprite = NEW CSprite*[MAX_GAMESPRITE];
		GAMESPRITE_MC = NEW CSprite("sprite\\mc00.bsprite");

		touchZones->AddZone(0, 0, 0, m_dispX, m_dispY);
		break;
	case GAME_STATE_MAIN:
		//######################
		//####	load resources
		//######################
#ifdef JK_SOUND_IMPLEMENT
		m_AudioManager.m_soundWrap->MusicFree();	
		m_AudioManager.m_soundWrap->MusicLoad("bg_game.mp3");//"sample1.mp3");
#endif
		delete(m_SplashScreen);
		m_ui = NEW CSprite("sprite\\interface0.bsprite");


		GAMESPRITE_ITEM = NEW CSprite("sprite\\items.bsprite");
		GAMESPRITE_MUMMY = NEW CSprite("sprite\\enemy_mummy.bsprite");
//		GAMESPRITE_VAMPIRE = NEW CSprite("sprite\\enemy_vampire.bsprite");
		GAMESPRITE_WOLF = NEW CSprite("sprite\\enemy_wolf.bsprite");
		GAMESPRITE_MCBULLET = NEW CSprite("sprite\\mcbullet.bsprite");

		m_pad = NEW CSprite("sprite\\vpad.bsprite");
		//#####################
		//####	reset game
		//####		::reset Actor
		//####		::reset game variables
		initActors();
		m_gameTime = GETTIMEMS();
		m_gameState = GAME_READY;
		resetGame();
		break;
	}
	if (++ m_loadingCounter <= m_loadingLength)
		m_state = m_stateNext;
}


void CHighGear::initActors()
{
	m_actors = NEW CActor*[MAX_ACTOR];
	for (int i = 0; i < MAX_ACTOR; i ++)
		m_actors[i] = NEW CActor(this);
	
	//Just set MC's actor
	MAINCHAR->init(CActor::ACTOR_MC, GAMESPRITE_MC, MC_XCOORD, MC_YCOORD);

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

	switch (m_gameState)
	{
	case GAME_READY:
	{
		if (GETTIMEMS() - m_gameTime > 3000)
		{
			m_AudioManager.m_soundWrap->MusicStart(true);

			//Set the game
			m_gameState = GAME_START;
			m_gameTime = GETTIMEMS();
			
			touchZones->AddZone(ZONEID_PAD_LEFT, 
								TOUCH_AREA_LBUTTON_X,
								TOUCH_AREA_LBUTTON_Y,
								TOUCH_AREA_LBUTTON_X + TOUCH_AREA_BUTTON_W,
								TOUCH_AREA_LBUTTON_Y + TOUCH_AREA_BUTTON_H
								);
			touchZones->AddZone(ZONEID_PAD_RIGHT, 
								TOUCH_AREA_RBUTTON_X,
								TOUCH_AREA_RBUTTON_Y,
								TOUCH_AREA_RBUTTON_X + TOUCH_AREA_BUTTON_W,
								TOUCH_AREA_RBUTTON_Y + TOUCH_AREA_BUTTON_H
								);
			touchZones->AddZone(ZONEID_PAD_FIRE, 
								TOUCH_AREA_FBUTTON_X,
								TOUCH_AREA_FBUTTON_Y,
								TOUCH_AREA_FBUTTON_X + TOUCH_AREA_BUTTON_W,
								TOUCH_AREA_FBUTTON_Y + TOUCH_AREA_BUTTON_H
								);
//			touchZones->SetListener(this);
		}
		break;
	}
	case GAME_START:
		if (m_hp <= 0) 
		{
			m_gameState = GAME_OVER;
			m_gameTime = GETTIMEMS() - m_gameTime;
			MAINCHAR->notifyState(CActor::ACTOR_STATE_MCDIE, NULL);
			touchZones->AddZone(ZONEID_PLAY_AGAIN_YES,
								(m_dispX >> 1) - 60 - (m_ui->GetModuleWidth(16) >> 1),
								(m_dispY >> 1) + 55,
								(m_dispX >> 1) - 60 + (m_ui->GetModuleWidth(16) >> 1),
								(m_dispY >> 1) + 55 + (m_ui->GetModuleHeight(16))
								);
			touchZones->AddZone(ZONEID_PLAY_AGAIN_NO, 
								(m_dispX >> 1) + 60 - (m_ui->GetModuleWidth(16) >> 1),
								(m_dispY >> 1) + 55,
								(m_dispX >> 1) + 60 + (m_ui->GetModuleWidth(16) >> 1),
								(m_dispY >> 1) + 55 + (m_ui->GetModuleHeight(16))
								);
			
			break;
		}
		//updateLevel();
		//updateTimer();
			
		if ((GETTIMEMS() - m_gameTime) % 5000 == 0)
			m_level ++;
		genEnemy();

		//Update Touch
//		if (MAINCHAR)
		{
//			if (touchZones->IsZoneReleased(ZONEID_PAD_LEFT))
//			{
//				if (m_keyRepeatValue & (1 << ZONEID_PAD_LEFT))
//				{
//					m_keyRepeatCounter = 0;
//				}
//			}
//			if (touchZones->IsZoneReleased(ZONEID_PAD_RIGHT))
//			{
//				if (m_keyRepeatValue & (1 << ZONEID_PAD_RIGHT))
//					m_keyRepeatCounter = 0;
//			}
//
//			m_keyRepeatValue = 0;
			if (touchZones->IsZonePressed(ZONEID_PAD_LEFT))
			{
//				m_keyRepeatValue |= (1 << ZONEID_PAD_LEFT);
				MAINCHAR->move(-1);
			}
			if (touchZones->IsZonePressed(ZONEID_PAD_RIGHT))
			{
//				m_keyRepeatValue |= (1 << ZONEID_PAD_RIGHT);
				MAINCHAR->move(1);
			}
			
			

//			if (m_keyRepeatValue != ((1 << ZONEID_PAD_LEFT) | (1 << ZONEID_PAD_RIGHT))
//				&& m_keyRepeatValue)
//			{
//				if (m_keyRepeatCounter != 1)
//					MAINCHAR->move(m_keyRepeatValue == (1 << ZONEID_PAD_LEFT) ? -1 : 1);
//				m_keyRepeatCounter++;
//			}

			if (touchZones->IsZonePressed(ZONEID_PAD_FIRE))
			{
				if (MAINCHAR->canFire())
				{
					//FIRE!
					int index = getEmptyActorIndex();

					if (index > -1)
					{
						m_actors[index]->init(CActor::ACTOR_MCBULLET, GAMESPRITE_MCBULLET, MAINCHAR->m_posX, MAINCHAR->m_posY);
					}
					MAINCHAR->notifyState(CActor::ACTOR_STATE_ATTACK, 15 - m_btDelay * 2);
#ifdef JK_SOUND_IMPLEMENT
					m_AudioManager.SampleSetVolume(0, 50);
					m_AudioManager.SampleStart(0, false);
#endif

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
		
		break;
	}
	
	/////////////////////////
	//DRAW
	/////////////////////////

	//BG
	GetLib2D().DrawRect(0, 0, m_dispX, m_dispY, 0xF000, 0xF000);
	m_bg[0]->DrawModule(GetLib2D(), 0, 0, 0);
//	m_bg[1]->DrawFrame(GetLib2D(), m_dispX >> 1, LEVEL_Y_START + 125, 0);


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

#define UI_KILL_X	240
#define UI_KILL_Y	30
#define UI_KILL_CAPTION_WIDTH	20
#define UI_VILLAGE_X	10
#define UI_VILLAGE_Y	60
#define UI_VILLAGE_CAPTION_X	180
#define UI_VILLAGE_CAPTION_WIDTH	20

	//####	Draw UIs
	if (m_gameState == GAME_READY)
	{	//'start' catpion
		if ((GETTIMEMS() / 500) % 2)
			m_ui->DrawFrame(GetLib2D(), m_dispX >> 1, m_dispY >> 1, 0);
	}

	else if (m_gameState == GAME_START)
	{
		GetLib2D().setColor(0x44FFFFFF);
		//DRAW VIRTUAL PAD
		m_pad->DrawModule(GetLib2D(), 
						TOUCH_AREA_LBUTTON_X + (TOUCH_AREA_BUTTON_W >> 1) - (m_pad->GetModuleWidth(0) >> 1),
						TOUCH_AREA_LBUTTON_Y + (TOUCH_AREA_BUTTON_H >> 1) - (m_pad->GetModuleHeight(0) >> 1), 
						0, 0);
		m_pad->DrawModule(GetLib2D(), 
						TOUCH_AREA_RBUTTON_X + (TOUCH_AREA_BUTTON_W >> 1) - (m_pad->GetModuleWidth(0) >> 1),
						TOUCH_AREA_RBUTTON_Y + (TOUCH_AREA_BUTTON_H >> 1) - (m_pad->GetModuleHeight(0) >> 1), 
						0, 1);

		m_pad->DrawModule(GetLib2D(), 
						TOUCH_AREA_FBUTTON_X + (TOUCH_AREA_BUTTON_W >> 1) - (m_pad->GetModuleWidth(1) >> 1),
						TOUCH_AREA_FBUTTON_Y + (TOUCH_AREA_BUTTON_H >> 1) - (m_pad->GetModuleHeight(1) >> 1), 
						1);

		GetLib2D().setColor(0xFFFFFFFF);
		//Kills
		m_ui->DrawFrame(GetLib2D(), UI_KILL_X, UI_KILL_Y, 11);
		drawNum(UI_KILL_X - UI_KILL_CAPTION_WIDTH, UI_KILL_Y, m_kill, UI_KILL_CAPTION_WIDTH, true);

		//TIME
		m_ui->DrawFrame(GetLib2D(), UI_VILLAGE_X, UI_VILLAGE_Y, 12);
		
		drawNum(UI_VILLAGE_CAPTION_X, UI_VILLAGE_Y, ((GETTIMEMS() - m_gameTime) / 10) % 100, UI_VILLAGE_CAPTION_WIDTH, true);
		drawNum(UI_VILLAGE_CAPTION_X - 40, UI_VILLAGE_Y, ((GETTIMEMS() - m_gameTime) / 1000), UI_VILLAGE_CAPTION_WIDTH, true);
//		drawNum(UI_VILLAGE_CAPTION_X - 60, UI_VILLAGE_Y, ((GETTIMEMS() - m_gameTime) / 60000) % 60, UI_VILLAGE_CAPTION_WIDTH, true);
	}
	else if (m_gameState == GAME_OVER)
	{
//		GetLib2D().setColor(0x44000000);
//		GetLib2D().DrawRect(0, 0, m_dispX, m_dispY, 0xFFFFFF);
		//Kills
		m_ui->DrawFrame(GetLib2D(), UI_KILL_X, UI_KILL_Y, 11);
		drawNum(UI_KILL_X - UI_KILL_CAPTION_WIDTH, UI_KILL_Y, m_kill, UI_KILL_CAPTION_WIDTH, true);
		
		//TIME
		m_ui->DrawFrame(GetLib2D(), UI_VILLAGE_X, UI_VILLAGE_Y, 12);
		
		drawNum(UI_VILLAGE_CAPTION_X, UI_VILLAGE_Y, (m_gameTime / 10) % 100, UI_VILLAGE_CAPTION_WIDTH, true);
		drawNum(UI_VILLAGE_CAPTION_X - 40, UI_VILLAGE_Y, (m_gameTime / 1000), UI_VILLAGE_CAPTION_WIDTH, true);
		
		GetLib2D().setColor(0xAAFFFFFF);
		m_ui->DrawFrame(GetLib2D(), m_dispX >> 1, (m_dispY >> 1) - 30, 13);
		
		m_ui->DrawModule(GetLib2D(), (m_dispX >> 1) - (m_ui->GetModuleWidth(14) >> 1), (m_dispY >> 1) + 20, 14, 0);
		m_ui->DrawModule(GetLib2D(), (m_dispX >> 1) - 60 - (m_ui->GetModuleWidth(16) >> 1), (m_dispY >> 1) + 55, 16, 0);
		m_ui->DrawModule(GetLib2D(), (m_dispX >> 1) + 60 - (m_ui->GetModuleWidth(17) >> 1), (m_dispY >> 1) + 55, 17, 0);
		
		if (touchZones->WasZoneActivated(ZONEID_PLAY_AGAIN_YES))
		{ 
			touchZones->ClearZones();
			initActors();
			m_gameTime = GETTIMEMS();
			m_gameState = GAME_READY;
			resetGame();
		}
		else if (touchZones->WasZoneActivated(ZONEID_PLAY_AGAIN_NO))
		{
			touchZones->ClearZones();
			setGameState(GAME_STATE_TITLE);
		}
	}
}

int CHighGear::countEnemy()
{
	int cnt = 0;

	for (int i = 0 ;i < MAX_ACTOR; i ++)
	{
		if (!m_actors[i]) continue;
		if (!m_actors[i]->isEnemy()) continue;
		if (m_actors[i]->m_state == CActor::ACTOR_STATE_DESTROYED) continue;

		cnt ++;
	}

	return cnt;
}

void CHighGear::genEnemy()
{
	m_currentEnemyCnt = countEnemy();

	if (m_currentEnemyCnt > (m_level + 1) * 3) return;

	if (m_Random.GetNumber(0, 100) < 5 + m_level / 4)
	{
		int index = getEmptyActorIndex();

		if (index > -1)
		{
			
			int eType = m_Random.GetNumber(0, MAX_ENEMY);

			m_actors[index]->init(CActor::ACTOR_MUMMY + eType, m_gameSprite[GAMESPRITE_ENEMY_START_INDEX + eType],
									m_Random.GetNumber(0, LEVEL_UNIT_WIDTH), 0, m_level / 20);
			if (m_enemyGenIndex % Max(7 - m_level / 10, 3) == 0) 
				m_actors[index]->bHasItem = true;
			if (++ m_enemyGenIndex > 10000) m_enemyGenIndex = 1;
		}
		
	}
}

void CHighGear::drawNum(int x, int y, int value, int fontWidth, bool bRightAlign)
{
	int digit = getNumDigit(value);
	
	if (!bRightAlign) x += fontWidth * digit;
	for (int i = 0 ; i < digit; i ++)
	{
		m_ui->DrawFrame(GetLib2D(), x - fontWidth * i, y , 1 + value % 10);
		value /= 10;
	}
}

int CHighGear::getNumDigit(int v)
{
	if (v < 0) return 0;
	if (v == 0) return 1;

	int cnt = 0;
	while (v > 0)
	{
		v /= 10;
		cnt ++;
	}
	return cnt;
}