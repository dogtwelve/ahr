
// define custom pocket gl application for demo 1

#ifndef _HIGHGEAR_H_INCLUDED_
#define _HIGHGEAR_H_INCLUDED_
#include "GenDef.h"

#include "bluetooth/smiledef.h"

#include "config.h"

#include "ArchivedFileManager.h"

extern const char* ArchiveFileNames[ARCHIVE_NB];

#include "Sound/SoundManager.h"

#include "Singleton.h"
#include "PlayingGame_defines.h"
#include "Lib2D/lib2d.h"
#include "Lib2D/Image.h"

#include "AuroraSprite.h"

#ifdef USE_TOUCH_SCREEN
#include "TouchZones.h"
#endif

#define RELEASE_MODE	// define to remove debug menus etc...

#ifdef SHOW_FPS
	#include "FPSCounter.h"
#endif

#ifdef HAS_ACHIEVEMENTS
	#include "Arena/Achievements.h" 
#endif // HAS_ACHIEVEMENTS

#ifdef USE_BACK_LIGHT
	#include "light.h" 
#endif // USE_BACK_LIGHT

#define kArcadeCheatSum 5000000

class CHighGear;

#include "file.h"

class CControl;

#ifdef HAS_MULTIPLAYER
	#define DEVICES_MAX		4
	#define MAX_ACCEPTED_CLIENTS 1

	#include "MultiPlayer/LocalDeviceDetails.h"
	#include "MultiPlayer/MultiplayerConstants.h"

	class Comms;
#endif // HAS_MULTIPLAYER


#include "Actor.h"

/////////////////////////////////////

//class CMainMenuStack;

//////////////////////////////////


class CGapi;

namespace Lib3D
{
	class CLib3D;
}

//////////////////////////////////////////////////


class CHighGear :   public CSingletonNoAutoConstruct<k_nSingletonHighGear, CHighGear>
#ifdef USE_TOUCH_SCREEN
							, ITouchZonesListener
#endif
{
public:

	
#ifdef USE_TOUCH_SCREEN
	CTouchZones* touchZones;
	virtual void ZonePressed(short zoneId);
	virtual void ZoneReleased(short zoneId);
	virtual void ZoneActivated(short zoneId);
	virtual void ZoneMove(short zoneId, int x, int y);
#endif // USE_TOUCH_SCREEN


public:
    CHighGear(CGapi&);
    ~CHighGear();
	
	bool InitHighGear(int step);

	/// Keyboard
			int 	mKeyboardConfiguration;
	inline  void    SetKeyBoardConfig( int keyConfig )	{ mKeyboardConfiguration = keyConfig; }
	inline  int 	GetKeyBoardConfig() const { return mKeyboardConfiguration; }

	/// Display orientaion
	bool	InitDisplayOrientation(int orientation);
	bool	m_bStartUpOrientationOverride;

    int		GameInit(void);
    void	ManageKeys();
	
    void	GameLoop(void);

	CSprite*	m_SplashScreen;
	CSprite*	m_SplashLogo;

inline    CGapi&				Gapi()				{return m_gapi;}

inline    Lib3D::CLib3D&		GetLib3D()			{return *m_lib3D;}
inline    CLib2D&		GetLib2D()					{A_ASSERT(m_lib2D); return *m_lib2D;}


////////////////////////////// Achievments //////////////////////////////
#ifdef HAS_ACHIEVEMENTS
	CAchievement m_Achievement;
	ACHIEVEMENT_HISTORY_CHUNCK	m_AchievementChunk;
#endif // HAS_ACHIEVEMENTS
///////////////////////////// ~Achievments //////////////////////////////

#ifdef USE_BACK_LIGHT
	CLight* m_pLight;
	bool m_bChargerConected;
#endif
////////////////////////////// Sounds //////////////////////////////////

	SoundManager m_AudioManager;
	
	void SetSoundStatus( bool status );

	//void LoadMenuSound();
///////////////////////////// ~Sounds //////////////////////////////////	

public:
	int m_dispX;
	int m_dispY;

    Lib3D::CLib3D*		m_lib3D;             // 3D library objects
    CLib2D*				m_lib2D;
    CGapi&				m_gapi;
	bool				m_isInPhoneCall;
	
	int m_FrameCounter;
	
	CRandom m_Random;

	typedef enum
	{
		GAME_STATE_INIT,
		GAME_STATE_LOGO,
		GAME_STATE_TITLE,
		GAME_STATE_LOADING,
		GAME_STATE_MENU,
		GAME_STATE_MAIN
		/*gs_test_loading,
		gs_test_view,
		gs_test_view2*/
	} e_m_state;
	
	e_m_state m_state, m_stateNext;
	void setGameState(e_m_state, bool bDrawL = false);

	int m_loadingCounter;
	int m_loadingLength;
	int m_keyRepeatCounter;
	int m_keyRepeatValue;
	bool m_bDrawLoading;
	void procLoading();
	void procMaingame();

	

	void drawNum(int x, int y, int value, int fontWidth, bool bRightAlign);
	int getNumDigit(int v);
	Image *m_screenImage2D;
	Image *m_screenImage3D;
	
	CSprite *m_logoSprite;
	
	void resetGame ();
	int m_logoTimer;

	int m_hp;
	int m_kill;
	int m_village;
	int m_level;

	int m_currentEnemyCnt;
	int m_enemyGenIndex;
	int m_btPow;
	int m_btDelay;

#define MAX_ACTOR	500
#define	MAX_GAMESPRITE	20
	void genEnemy();
	int countEnemy();
	void initActors();
	int getEmptyActorIndex();
	//for GAME_STATE_MAIN
	CSprite **m_gameSprite;
	CSprite **m_bg;
	CSprite *m_ui;
	CSprite *m_pad;

	CActor **m_actors;

	int m_gameTime;
	int m_gameState;
};

#endif
