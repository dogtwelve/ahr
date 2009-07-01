#ifndef _Gapi_h_
#define _Gapi_h_
// Common Gapi interface with Symbian
// To do left:
//   - Support for a sound server.
//   - Check why icon defined in aif folder do not appear.

#include "Keypad.h"
#include "GenDef.h"

// forward declarations
class CFunctor;

#ifdef NGI

#include <runtime.h>
using namespace ngi;

#endif // NGI

#ifndef __SYMBIAN32__

#ifdef IPHONE
	// defines for iphone

	#define KEY_LEFT  0x25                 
	#define KEY_RIGHT 0x27                 
	#define KEY_UP    0x26                 
	#define KEY_DOWN  0x28                 
	#define KEY_CLICK 0x20                 // Space

	#define KEY_0     0x60                 // 0 (on numpad)
	#define KEY_1     0x67 //0x61 // INVERT 1,2,3 and 7,8,9
	#define KEY_2     0x68 //0x62
	#define KEY_3     0x69 //0x63
	#define KEY_4     0x64
	#define KEY_5     0x65
	#define KEY_ENTER	13
	#define KEY_6     0x66
	#define KEY_7     0x61 //0x67
	#define KEY_8     0x62 //0x68
	#define KEY_9     0x63 //0x69
	#define KEY_STAR  0x6a // *

	#define KEY_DIEZ  0x6f // '/' 

	#define KEY_A	  0x41 // A	
	#define KEY_B	  0x42 // B
	#define KEY_C     0x43 // C
	#define KEY_PEN   0x50 // P
	#define KEY_VOLUME_UP   0x55 // U
	#define KEY_VOLUME_DOWN 0x44 // D

	#define KEY_LEFTSOFT  0x2e // Del
	#define KEY_RIGHTSOFT 0x22 // Page down
#else

	// defines for windows

	#define KEY_LEFT  0x25                 
	#define KEY_RIGHT 0x27                 
	#define KEY_UP    0x26                 
	#define KEY_DOWN  0x28                 

	#define KEY_CLICK	13	//enter again
	#define KEY_ENTER	13

#ifdef WINDOWS_MOBILE
	#define KEY_0     48
	#define KEY_1     69
	#define KEY_2     82
	#define KEY_3     84
	#define KEY_4     68
	#define KEY_5     70
	#define KEY_6     71
	#define KEY_7     88
	#define KEY_8     67
	#define KEY_9     86
	#define KEY_STAR  83
	#define KEY_DIEZ  90

	//@rax - please don't change settings for the WM platform
	//#define KEY_0     48
	//#define KEY_1     49
	//#define KEY_2     50
	//#define KEY_3     51
	//#define KEY_4     52
	//#define KEY_5     53
	//#define KEY_6     54
	//#define KEY_7     55
	//#define KEY_8     56
	//#define KEY_9     57

	//#define KEY_STAR  83
	//#define KEY_DIEZ  90

	#define KEY_LEFTSOFT  112 // Del
	#define KEY_RIGHTSOFT 113 // Page down

#else

	#define KEY_0     VK_NUMPAD0
	#define KEY_1     VK_NUMPAD1
	#define KEY_2     VK_NUMPAD2
	#define KEY_3     VK_NUMPAD3
	#define KEY_4     VK_NUMPAD4
	#define KEY_5     VK_NUMPAD5
	#define KEY_6     VK_NUMPAD6
	#define KEY_7     VK_NUMPAD7
	#define KEY_8     VK_NUMPAD8
	#define KEY_9     VK_NUMPAD9

	#define KEY_STAR  VK_MULTIPLY
	#define KEY_DIEZ  VK_DIVIDE

	#define KEY_LEFTSOFT  0x2e // Del
	#define KEY_RIGHTSOFT 0x22 // Page down

#endif

	#define KEY_A	  0x41 // A	
	#define KEY_B	  0x42 // B

	#define KEY_PEN   0x50 // P
	#define KEY_VOLUME_UP   0x55 // U
	#define KEY_VOLUME_DOWN 0x44 // D	

	#define KEY_ACTION_LEFT  0x2d // Insert
	#define KEY_ACTION_RIGHT 0x21 // Page up
#endif


#else // __SYMBIAN32__
// ----------------------------------------------
// defines for Epoc version

// Some key codes. 
// Note: More Symbian key codes can be found in Epoc32/Include/E32KEYS.H in symbian SDK
#ifdef MOTOROLA_Z8
	#define KEY_LEFT  0xAA                 // EStdKeyLeftArrow
	#define KEY_RIGHT 0xAB                 // EStdKeyRightArrow
	#define KEY_UP    0xA8                 // EStdKeyUpArrow
	#define KEY_DOWN  0xA9                 // EStdKeyDownArrow
	#define KEY_CLICK 0xAC
#else
	#define KEY_LEFT  0x0e                 // EStdKeyLeftArrow
	#define KEY_RIGHT 0x0f                 // EStdKeyRightArrow
	#define KEY_UP    0x10                 // EStdKeyUpArrow
	#define KEY_DOWN  0x11                 // EStdKeyDownArrow
	#define KEY_CLICK 0xa7
#endif

#define KEY_0     0x30                 // 0
#define KEY_1     0x31
#define KEY_2     0x32
#define KEY_3     0x33
#define KEY_4     0x34
#define KEY_5     0x35
#define KEY_6     0x36
#define KEY_7     0x37
#define KEY_8     0x38
#define KEY_9     0x39
#define KEY_DIEZ  0x7f                 // '#'

#define KEY_C     0x01
#define KEY_PEN   0x12

#ifdef MOTOROLA_Z8
	#define KEY_STAR  0x85                 // '*'
	#define KEY_LEFTSOFT  0x6C
	#define KEY_RIGHTSOFT 0x6D	//and moto
#else
	#define KEY_STAR  0x2A                 // '*'
	#define KEY_LEFTSOFT  0xa4
	#define KEY_RIGHTSOFT 0xa5
#endif
//#define sqrtf sqrt                     // symbian do not know sqrtf function
#endif

#define	SOFTKEY_POSITION_LEFT			(1 << 0)
#define	SOFTKEY_POSITION_RIGHT			(1 << 1)
#define	SOFTKEY_POSITION_TOP			(1 << 2)
#define	SOFTKEY_POSITION_BOTTOM			(1 << 3)

#define ORIENTATION_PORTRAIT			(1 << 0)
#define ORIENTATION_LANDSCAPE_90		(1 << 1)
#define ORIENTATION_LANDSCAPE_270		(1 << 2)

#ifdef NGI
	#define ORIENTATION_SYSTEM				ORIENTATION_LANDSCAPE_270
#else
	#define ORIENTATION_SYSTEM				ORIENTATION_PORTRAIT
#endif

#define	ORIENTATION_DIRECTION_LEFT		(1 << 0)
#define	ORIENTATION_DIRECTION_RIGHT		(1 << 1)
#define	ORIENTATION_DIRECTION_FLIP		(1 << 2)
#define	ORIENTATION_DIRECTION_SYSTEM	(1 << 3)

#define DEVICE_TYPE_IPHONE				0
#define DEVICE_TYPE_ITOUCH				1

extern int g_DeviceType;
extern unsigned int g_SystemVersion;

class CHighGear;                         // class containing pocket gl application

// GAPI interface
class CGapi
{
public:

	CGapi();

	int InitGame(void* pos);                  // main game init, must return 1 if success
	int LoopGame(void);                  // main game loop, must return 0 to quit
	void EndGame(void);                  // called before to quit (Ui quit key pressed), allow to free some memory. if *Cancel set to 1, then quit is canceled
	
	void DrawFPS(int);

	inline void Quit() { m_bQuit = true; }

    void TurnOnBackLight();

  // keyboard
//  int KeyPressedId;                    // Id of last keypressed, can be used as key pressed event. (must be reseted by client)
//  int KeyExitCode;                     // define extra key code that will make application exit (KEY_ESC always valid)
//  unsigned char KeyPressed[256];       // 1 if key[n] pressed, else 0
#if DRAW_KEY_CODE
	int m_iP_Key_Code;
	int m_iR_Key_Code;
	char m_sKey_Code[64];
#endif
  CKeypad m_Keypad;

	int		m_displayMode;			// 0 = 4k, 1 = 64k, 2 = 262k, 3 = 16M
	bool	limitedPhone;

	//SEFU
	bool				midi_only;

	int KeyCode2Keypad(int iKeyCode);
	void KeyEvent(bool bPressed, int iKey);


	// screen
	unsigned short	*ScreenPtr;           // ptr to screen

	// time
	unsigned int TickCount256;           // system tick counter in ms*256, about 4h40 min loop

	// path to access datas
	int PathLen;                         // size of path in DataPath

	char DataPath[MAX_PATH_LENGTH];                  // path on datas, allow use of fopen, fread, ...

	char *MakePath(const char *FileName);      // concat DataPath + FileName -> DataPath

	void UpdateDisplay() const;

	// pocket GL application
	CHighGear *m_pHighGear;

#define DRM_STATE_STARTUP_FLAG				(0x0001)
#define DRM_STATE_ACTIVATE_LICENSE_FLAG		(0x0002)

#define DRM_STATE_REQUEST_FLAG				(0x0010)
#define DRM_STATE_WAIT_FLAG					(0x0020)
#define DRM_STATE_COMPLETE_FLAG				(0x0040)

#define DRM_STATE_LICENSE_VALID_FLAG		(0x0100)
#define DRM_STATE_LICENSE_INVALID_FLAG		(0x0200)
#define DRM_STATE_LICENSE_EXPIRED_FLAG		(0x0400)

#define DRM_STATE_CONSUMPTION_START			(0x1000)
#define DRM_STATE_CONSUMPTION_STOP			(0x2000)
#define DRM_STATE_CONSUMING_LICENSE			(0x4000)

#define	DRM_STATE_VOID						(0x0000)

#define	DRM_STATE_STARTUP					(DRM_STATE_STARTUP_FLAG | DRM_STATE_REQUEST_FLAG)
#define	DRM_STATE_STARTUP_WAIT				(DRM_STATE_STARTUP_FLAG | DRM_STATE_WAIT_FLAG)
#define	DRM_STATE_STARTUP_COMPLETE			(DRM_STATE_STARTUP_FLAG | DRM_STATE_COMPLETE_FLAG)

#define	DRM_STATE_ACTIVATE_LICENSE			(DRM_STATE_ACTIVATE_LICENSE_FLAG | DRM_STATE_REQUEST_FLAG)
#define	DRM_STATE_ACTIVATE_LICENSE_WAIT		(DRM_STATE_ACTIVATE_LICENSE_FLAG | DRM_STATE_WAIT_FLAG)
#define	DRM_STATE_ACTIVATE_LICENSE_COMPLETE	(DRM_STATE_ACTIVATE_LICENSE_FLAG | DRM_STATE_COMPLETE_FLAG)

	CFunctor *m_UpdateDisplay;
	int		mDrmState;
	char*	mDrmFileName;

	void StartConsumeLicense() { mDrmState |= DRM_STATE_CONSUMPTION_START; }
	void StopConsumeLicense()  { mDrmState |= DRM_STATE_CONSUMPTION_STOP; } 

	bool m_bQuit;
	bool specialDisplay;

    // Back light
    bool m_bTurnOnBackLight;

    int	m_nSignalStrength;

	bool m_orientationOptionsInMenus;

	int		mLeftSoftKeyPosition;
	int		mRightSoftKeyPosition;

	int		mValidOrientations;
	int		mAvailableOrientations;
	int		mCurrentOrientation;
	int		mStartUpOrientation;

	bool	m_isWideMode;

	int		GetDisplayOrientation();
	int		GetAvailableDirections();
	int		GetAvailableOrientations();

	void	ApplyDirection(int orientationDirection);
	void	ApplyOrientation(int orientation);

	int		mDisplayOrientationRequest;	// Set by game menus to request orientation change
	int		mDisplayOrientationPending;	// Set by system callback and have to be checked by game modules to reinitialize themselves
	
	bool	m_bOrientationChangedThisFrame; //this is made true after orientation was changed, stays true for exactly one frame

	const static int ORIENTATION_CHANGE_TIME = 10; // 16;
	int		mDisplayOrientationTimer;
	int		mDisplayOrientationOld;

	int		mOrientationChangeIgnoreFrames;	//salcideon - a little bit complicated mechanism that allows me to detect
											//if user changed orientation from game menu.

//	int		mOrientationWaitAFrame;			//used this to skip a frame after changing orientation before drawing in the backbuffer (so that display can be reinitialized)

#ifdef NGI
	bool	mSwitchToNgage;	// Set by system callback and have to be checked by game modules to reinitialize themselves
#endif // NGI

//#ifdef WIN32
public:
	static enum {
		RUN_NO_INTERFACE = (1 << 0),
		RUN_NO_BG_OBJECTS = (1 << 1),
		RUN_NO_LENS_FLARE = (1 << 2),
		RUN_NO_DRAW = (1 << 3),
	} eRunFlags;

	int mRunFlags;
//#endif

};

#endif
