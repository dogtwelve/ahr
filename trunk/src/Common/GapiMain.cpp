#pragma warning(disable:4786)
// simple demo using GAPI for S60
// load and display a scrooling picture
// note: to run program with windows S60 emulator, you must copy the TGA file using file explorer in
// emulator HighGear folder (/* your PC SDK path */ + Epoc32\Release\wins\UDEB\Z\SYSTEM\apps\GAPIS60)
// (you can use file search function to retrieve it)
#pragma warning(disable:4786)

#ifdef __SYMBIAN32__

	#include "Application.h"
#ifdef NGI
	#include "osextensions\rga\Input.h"	
#endif // NGI
#endif

#ifdef IPHONE
	#include "IphoneUtil.h"
#endif

#include "Config.h"
#include "Functor.h"
#include "Gendef.h"

#include "HG/HighGear.h"

#ifdef CHECK_MEMORY_LEAKS
	#include "MemoryManager.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef IPHONE
extern "C" void setStatusBarOrientation(int orient);
#endif

int g_DeviceType = DEVICE_TYPE_ITOUCH;
unsigned int g_SystemVersion = 0;

CGapi::CGapi()
{
	midi_only = false;

	specialDisplay = false;
	ScreenPtr = NULL;
	m_pHighGear = NULL;
	m_UpdateDisplay = NULL;

	mDrmState = DRM_STATE_VOID;
	m_bQuit = false;
	TickCount256 = 0;
	PathLen = 0;
	DataPath[0] = 0;

	m_bTurnOnBackLight = false;
	m_nSignalStrength = 0;

	m_displayMode = -1;

	m_orientationOptionsInMenus = true;

	mCurrentOrientation		= ORIENTATION_PORTRAIT;
	
	m_isWideMode			= false;

	mLeftSoftKeyPosition	= SOFTKEY_POSITION_BOTTOM | SOFTKEY_POSITION_LEFT;
	mRightSoftKeyPosition	= SOFTKEY_POSITION_BOTTOM | SOFTKEY_POSITION_RIGHT;

	mValidOrientations		= ORIENTATION_PORTRAIT | ORIENTATION_LANDSCAPE_90 | ORIENTATION_LANDSCAPE_270 | ORIENTATION_SYSTEM;
	mAvailableOrientations	= ORIENTATION_PORTRAIT | ORIENTATION_LANDSCAPE_90 | ORIENTATION_LANDSCAPE_270 | ORIENTATION_SYSTEM;

	mDisplayOrientationRequest = -1;
	mDisplayOrientationPending = -1;
	mOrientationChangeIgnoreFrames = -1;
	
	mDisplayOrientationTimer = 0;
	mDisplayOrientationOld = -1;
	
	

	mDrmFileName = 0;
#ifdef NGI
	mSwitchToNgage = false;
#endif // NGI
}

// -----------------------------------------------------
// Main game init, must return 1 if success
// -----------------------------------------------------
int CGapi::InitGame(void* os)
{
	// Initialize the performance monitor code.
    m_bTurnOnBackLight = false;

	m_nSignalStrength = 0;

	CSingletonManager::Create();

	m_pHighGear = NEW CHighGear(*this);
	if (!m_pHighGear )
		return 0;

	return m_pHighGear->GameInit();
}

// -----------------------------------------------------
// Main game loop
// -----------------------------------------------------

extern bool g_bRefreshScreenBuffer;
extern bool g_bSkipRendering;

int CGapi::LoopGame(void)
{
#ifdef IPHONE


	{
		int crtOrientation = GetPhoneOrientation();
		
		if (crtOrientation < 0 || mDisplayOrientationOld != crtOrientation)
		{
			mDisplayOrientationTimer = ORIENTATION_CHANGE_TIME;

			// on menus change
			//if (m_pHighGear->m_gState != CHighGear::gs_play)
				mDisplayOrientationTimer = 1;
		}
		
		if (mDisplayOrientationTimer > 0)
		{
			mDisplayOrientationTimer--;

			if (mDisplayOrientationTimer == 0)
			{
				mDisplayOrientationPending = crtOrientation;
			}			
		}
		
		mDisplayOrientationOld = crtOrientation;
	}
#endif
	
	if (mDisplayOrientationPending != -1) 
	{
#ifdef WIN32
		//LAngelov: Patch for Windows version to actualize variables actually get from the system
		if (mDisplayOrientationPending == ORIENTATION_PORTRAIT)
		{
			mLeftSoftKeyPosition	= SOFTKEY_POSITION_BOTTOM | SOFTKEY_POSITION_LEFT;
			mRightSoftKeyPosition	= SOFTKEY_POSITION_BOTTOM | SOFTKEY_POSITION_RIGHT;
		}
		else
		{
			mLeftSoftKeyPosition	= SOFTKEY_POSITION_TOP | SOFTKEY_POSITION_RIGHT;
			mRightSoftKeyPosition	= SOFTKEY_POSITION_BOTTOM | SOFTKEY_POSITION_RIGHT;
		}
#endif

		if (m_pHighGear->InitDisplayOrientation(mDisplayOrientationPending))
		{
			mCurrentOrientation = mDisplayOrientationPending;
			g_bRefreshScreenBuffer = true;
			g_bSkipRendering = true;
			mDisplayOrientationPending = -1;
			m_bOrientationChangedThisFrame = true;
		}
		m_Keypad.Keypad_Reset();
	}
	m_pHighGear->GameLoop();
	
	m_bOrientationChangedThisFrame = false;
//	m_pHighGear->m_AfterSuspend = false;

#ifdef USE_SHOW_STATUS_BAR
#ifdef IPHONE
//	if (m_pHighGear->bChangeStatusBarOrientation)
//		setStatusBarOrientation(mCurrentOrientation);
#endif // IPHONE

	//m_pHighGear->bChangeStatusBarOrientation = false;
#endif // USE_SHOW_STATUS_BAR

	return 1;
}

// -----------------------------------------------------
// Main game terminate function
// -----------------------------------------------------
void CGapi::EndGame(void)
{
	CSingletonManager::Shutdown();
#ifdef CHECK_MEMORY_LEAKS
	//CMemoryManager::Exit();
#endif

#ifdef BREW_MEMORY_DEBUG
	debug_showallocated();
#endif
}

// -----------------------------------------------------
// Util functions
// -----------------------------------------------------

// concat DataPath + FileName -> DataPath
char *CGapi::MakePath(const char *FileName)
{
  DataPath[PathLen] = 0;

  A_ASSERT( ( PathLen + strlen(FileName) + 1 ) < sizeof(DataPath) );

#ifdef WINDOWS_MOBILE
  DataPath[0] = 0;
  strcat(DataPath, GAME_PATH);
  //PathLen = 14;
  strcat(DataPath, FileName);
#else
  strcat(DataPath, FileName);
#endif
  return DataPath;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

int CGapi::KeyCode2Keypad(int iKeyCode)
{
	int keymap[3][14] = {
//PORTRAIT
		{ kKeypad_DPad_Left, kKeypad_DPad_Up, kKeypad_DPad_Right, kKeypad_DPad_Down, kKeypad_0, kKeypad_1, kKeypad_2, kKeypad_3, kKeypad_4, kKeypad_5, kKeypad_6, kKeypad_7, kKeypad_8, kKeypad_9 },
//LANDSCAPE 90		
		{ kKeypad_DPad_Down, kKeypad_DPad_Left, kKeypad_DPad_Up, kKeypad_DPad_Right, kKeypad_0, kKeypad_1, kKeypad_2, kKeypad_3, kKeypad_4, kKeypad_5, kKeypad_6, kKeypad_7, kKeypad_8, kKeypad_9 },
//LANDSCAPE 270	    
	    { kKeypad_DPad_Up, kKeypad_DPad_Right, kKeypad_DPad_Down, kKeypad_DPad_Left, kKeypad_0, kKeypad_1, kKeypad_2, kKeypad_3, kKeypad_4, kKeypad_5, kKeypad_6, kKeypad_7, kKeypad_8, kKeypad_9 },
	};
	
	int keyset;
	keyset = CHighGear::GetInstance()->GetKeyBoardConfig();

	// rax - on Win32, do not change key mapping
#if defined __WIN32__ && !defined WINDOWS_MOBILE
	keyset = 0;
#endif // __WIN32__


	switch(iKeyCode)
	{
#ifdef NGI
		case ngi::INPUT_KEY_DIGITAL_LEFT :				return kKeypad_DPad_Left;
        case ngi::INPUT_KEY_DIGITAL_UP :				return kKeypad_DPad_Up;
		case ngi::INPUT_KEY_DIGITAL_RIGHT :				return kKeypad_DPad_Right;
		case ngi::INPUT_KEY_DIGITAL_DOWN :				return kKeypad_DPad_Down;
		case ngi::INPUT_KEY_OK :           		 		return kKeypad_DPad_Click;
        case ngi::INPUT_KEY_0:							return kKeypad_0;
        case ngi::INPUT_KEY_1:							return kKeypad_1;
        case ngi::INPUT_KEY_2:							return kKeypad_2;
        case ngi::INPUT_KEY_3:							return kKeypad_3;
        case ngi::INPUT_KEY_4:							return kKeypad_4;
        case ngi::INPUT_KEY_5:							return kKeypad_5;
        case ngi::INPUT_KEY_6:							return kKeypad_6;
        case ngi::INPUT_KEY_7:							return kKeypad_7;
        case ngi::INPUT_KEY_8:							return kKeypad_8;
        case ngi::INPUT_KEY_9:							return kKeypad_9;

		case ngi::INPUT_KEY_HASH:						return kKeypad_Pound;
		case ngi::INPUT_KEY_STAR:						return kKeypad_Star;

		case ngi::INPUT_KEY_GAMING_A:					return kKeypad_Gaming_Key_A;
		case ngi::INPUT_KEY_GAMING_B:					return kKeypad_Gaming_Key_B;

//      case ngi::INPUT_KEY_CLEAR:				    	return kKeypad_C;
	    case ngi::INPUT_KEY_CLEAR:				    	return kKeypad_Pen;
		case ngi::INPUT_KEY_EDIT:						return kKeypad_Pen;

        case ngi::INPUT_KEY_LSK :         				return kKeypad_SoftKey_Left;
        case ngi::INPUT_KEY_RSK :        				return kKeypad_SoftKey_Right;

		case ngi::INPUT_KEY_VOLUME_UP:					return kKeypad_Volume_Up;
		case ngi::INPUT_KEY_VOLUME_DOWN:				return kKeypad_Volume_Down;

		default :										return -1;

#else // NGI

		case KEY_LEFT :									return keymap[keyset][0];
        case KEY_UP :									return keymap[keyset][1];
		case KEY_RIGHT :								return keymap[keyset][2];
		case KEY_DOWN :									return keymap[keyset][3];
			
#if defined (__SYMBIAN32__) || defined (IPHONE) 
		case KEY_CLICK :            					return kKeypad_DPad_Click;
#endif

        case KEY_0:										return keymap[keyset][4];//kKeypad_0;
        case KEY_1:										return keymap[keyset][5];//kKeypad_1;
        case KEY_2:										return keymap[keyset][6];//kKeypad_2;
        case KEY_3:										return keymap[keyset][7];//kKeypad_3;
        case KEY_4:										return keymap[keyset][8];//kKeypad_4;
        case KEY_5:										return keymap[keyset][9];//kKeypad_5;
#ifndef __SYMBIAN32__
		case KEY_ENTER:									return keymap[keyset][9];//kKeypad_5;
#endif
        case KEY_6:										return keymap[keyset][10];//kKeypad_6;
        case KEY_7:										return keymap[keyset][11];//kKeypad_7;
        case KEY_8:										return keymap[keyset][12];//kKeypad_8;
        case KEY_9:										return keymap[keyset][13];//kKeypad_9;

		case KEY_DIEZ:									return kKeypad_Pound;
		case KEY_STAR:									return kKeypad_Star;

//		case KEY_A:										return kKeypad_Gaming_Key_A;
//		case KEY_B:										return kKeypad_Gaming_Key_B;

//      case KEY_C:									    return kKeypad_C;
//		case KEY_PEN:									return kKeypad_Pen;

//#ifdef INVERT_SOFTKEYS
//        case KEY_LEFTSOFT :         					return kKeypad_SoftKey_Right;
//        case KEY_RIGHTSOFT :        					return kKeypad_SoftKey_Left;
//#else
        case KEY_LEFTSOFT :         					return kKeypad_SoftKey_Left;
        case KEY_RIGHTSOFT :        					return kKeypad_SoftKey_Right;
//#endif

//		case KEY_VOLUME_UP:								return kKeypad_Volume_Up;
//		case KEY_VOLUME_DOWN:							return kKeypad_Volume_Down;

		default :					return -1;

#endif // NGI
	}
}

//------------------------------------------------------
//------------------------------------------------------
void CGapi::KeyEvent(bool bPressed, int iKey)
{
	if(bPressed)
	{
#if DRAW_KEY_CODE
		m_iP_Key_Code = iKey;
#endif
		m_Keypad.Keypad_AddEvent(KeyCode2Keypad(iKey), kKeypad_EventType_Pressed);
	}
	else
	{
#if DRAW_KEY_CODE
		m_iR_Key_Code = iKey;
#endif
		m_Keypad.Keypad_AddEvent(KeyCode2Keypad(iKey), kKeypad_EventType_Released);
	}
}

void CGapi::UpdateDisplay() const
{
    A_ASSERT(m_UpdateDisplay);
    (*m_UpdateDisplay)();
}

void CGapi::TurnOnBackLight()
{
    m_bTurnOnBackLight = true;
}


int	CGapi::GetDisplayOrientation() 
{
	return mCurrentOrientation;
}

int	CGapi::GetAvailableDirections() 
{
	int available = 0;

	switch(mCurrentOrientation) 
	{
		case ORIENTATION_PORTRAIT:
			if(mValidOrientations & ORIENTATION_LANDSCAPE_90)
				available |= ORIENTATION_DIRECTION_LEFT;
			if(mValidOrientations & ORIENTATION_LANDSCAPE_270)
				available |= ORIENTATION_DIRECTION_RIGHT;
		break;

		case ORIENTATION_LANDSCAPE_90:
			if(mValidOrientations & ORIENTATION_PORTRAIT)
				available |= ORIENTATION_DIRECTION_RIGHT;
			if(mValidOrientations & ORIENTATION_LANDSCAPE_270)
				available |= ORIENTATION_DIRECTION_FLIP;
		break;

		case ORIENTATION_LANDSCAPE_270:
			if(mValidOrientations & ORIENTATION_PORTRAIT)
				available |= ORIENTATION_DIRECTION_LEFT;
			if(mValidOrientations & ORIENTATION_LANDSCAPE_90)
				available |= ORIENTATION_DIRECTION_FLIP;
		break;
	}

	if(mValidOrientations & ORIENTATION_SYSTEM)
		available |= ORIENTATION_DIRECTION_SYSTEM;

	return available;
}

int	CGapi::GetAvailableOrientations()
{
	return mAvailableOrientations;
}

void CGapi::ApplyDirection(int orientationDirection)
{
	int newOrientation = mCurrentOrientation;

	switch(mCurrentOrientation)
	{
		case ORIENTATION_PORTRAIT:
			if(orientationDirection == ORIENTATION_DIRECTION_LEFT)
				newOrientation = ORIENTATION_LANDSCAPE_90;
			if(orientationDirection == ORIENTATION_DIRECTION_RIGHT)
				newOrientation = ORIENTATION_LANDSCAPE_270;
		break;

		case ORIENTATION_LANDSCAPE_90:
			if(orientationDirection == ORIENTATION_DIRECTION_RIGHT)
				newOrientation = ORIENTATION_PORTRAIT;
			if(orientationDirection == ORIENTATION_DIRECTION_FLIP)
				newOrientation = ORIENTATION_LANDSCAPE_270;
		break;

		case ORIENTATION_LANDSCAPE_270:
			if(orientationDirection == ORIENTATION_DIRECTION_LEFT)
				newOrientation = ORIENTATION_PORTRAIT;
			if(orientationDirection == ORIENTATION_DIRECTION_FLIP)
				newOrientation = ORIENTATION_LANDSCAPE_90;
		break;
	}

	if (orientationDirection == ORIENTATION_DIRECTION_SYSTEM)
		newOrientation = ORIENTATION_SYSTEM;

#ifdef WIN32
	mDisplayOrientationPending = newOrientation;
	mOrientationChangeIgnoreFrames = 2;
#else
	mDisplayOrientationPending = newOrientation;
	mDisplayOrientationRequest = newOrientation;
#endif
}

void CGapi::ApplyOrientation(int orientation)
{
#ifdef WIN32
	mDisplayOrientationPending = orientation;
	mOrientationChangeIgnoreFrames = 2;
#else
	mDisplayOrientationPending = orientation;
	mDisplayOrientationRequest = orientation;
#endif
}
