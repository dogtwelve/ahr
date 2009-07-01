
#include "keypad.h"
#include "HG/HighGear.h"

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

CKeypad::CKeypad()
{
	Keypad_State_Old		= 0;
	Keypad_State_Current	= 0;
	Keypad_EventQueue_Start	= 0;
	Keypad_EventQueue_End	= 0;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_IsKeyPressed(unsigned int Param_Key)
// Returns TRUE  : Key Is Currently Held Down
// Returns FALSE : Key Is Unpressed
{
	return((Keypad_State_Current & (1 << Param_Key)) != 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_HasKeyChanged(unsigned int Param_Key)
// Returns TRUE  : Key Changed From Down <-> Up During Last Update
// Returns FALSE : Key State Remained Identical During Last Update
{
	return(((Keypad_State_Current ^ Keypad_State_Old) & (1 << Param_Key)) != 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_HasKeyBeenPressed(unsigned int Param_Key)
// Returns TRUE  : Key Changed From Up -> Dowb During Last Update
// Returns FALSE : Key State Either Remained Identical Or Went Down -> Up During Last Update
{
	return(((Keypad_State_Current & ~Keypad_State_Old) & (1 << Param_Key)) != 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_IsAnyKeyPressed()
// Returns TRUE  : There Is A Key Currently Held Down
// Returns FALSE : All Keys Are Unpressed
{
	return(Keypad_State_Current != 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_HasAnyKeyChanged()
// Returns TRUE  : One Or More Keys Changed From Down <-> Up During Last Update
// Returns FALSE : All Keys Remained Identical During Last Update
{
	return(Keypad_State_Current != Keypad_State_Old);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_HasAnyKeyBeenPressed()
// Returns TRUE  : One Or More Keys Changed From Up -> Dowb During Last Update
// Returns FALSE : All Keys Either Remained Identical Or Went Down -> Up During Last Update
{
//#ifdef IPHONE
//	LOGDEBUG("CKeypad::Keypad_HasAnyKeyBeenPressed - Keypad_State_Current[%d], Keypad_State_Old[%d]\n", 
//			 Keypad_State_Current, Keypad_State_Old);
//#endif
	
	return((Keypad_State_Current & ~Keypad_State_Old) != 0);
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

int CKeypad::Keypad_GetLastPressedKeyCode()
{
	int change_key_state = (Keypad_State_Current & ~Keypad_State_Old) >> 1;

	int key_code = 0;
	while( change_key_state != 0 )
	{
		key_code++;
		change_key_state >>= 1;
	}

	return key_code;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_IsActionKeyBeenPressed( unsigned int action_key, bool permanently )
// Returns TRUE  : Action key was pressed
// Returns FALSE : Action key wasn't pressed
{
	int mask;
	CHighGear* hg = CHighGear::GetInstance();

//	mask = *(hg->m_Profile.GetKeymap() + action_key);
//
//	/// Always check for DPad keys
//	if( action_key == CONTROLS_ENTRY_NITRO )
//		mask |= (1<<kKeypad_DPad_Click);
//	if( action_key == CONTROLS_ENTRY_BRAKES )
//		mask |= (1<<kKeypad_DPad_Down);
//	if( action_key == CONTROLS_ENTRY_TURN_LEFT )
//		mask |= (1<<kKeypad_DPad_Left);
//	if( action_key == CONTROLS_ENTRY_TURN_RIGHT )
//		mask |= (1<<kKeypad_DPad_Right);
	
	if( permanently )
		return( (Keypad_State_Current & mask) != 0 );
	
	return( ((Keypad_State_Current & ~Keypad_State_Old) & mask) != 0 );	
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_HasActionKeyBeenDoublePressed(unsigned int action_key)
{
	int mask;
//	int mask = *(CHighGear::GetInstance()->m_Profile.GetKeymap() + action_key);	
//	/// Always check for DPad keys
//	if( action_key == CONTROLS_ENTRY_NITRO )
//		mask |= (1<<kKeypad_DPad_Click);
//	if( action_key == CONTROLS_ENTRY_BRAKES )
//		mask |= (1<<kKeypad_DPad_Down);
//	if( action_key == CONTROLS_ENTRY_TURN_LEFT )
//		mask |= (1<<kKeypad_DPad_Left);
//	if( action_key == CONTROLS_ENTRY_TURN_RIGHT )
//		mask |= (1<<kKeypad_DPad_Right);

	return (mask & (1<<Keypad_Repeated_Key_ID)) && Keypad_Repeated_Key_Count >= 2 && Keypad_Repeated_Key_Time == 0;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void	CKeypad::Keypad_RemoveActionKey( unsigned int action_key )
{
	int key = 0;
	int mask;
//	CHighGear* hg = CHighGear::GetInstance();
//	A_ASSERT( hg );	
//
//	mask = *(hg->m_Profile.GetKeymap() + action_key);
	
	while( key < kKeypad_OUTOFBOUNDS )
	{
		if( (mask & 1) != 0 )
			Keypad_AddEvent( key, kKeypad_EventType_Released );
		
		key++;
		mask >>= 1;
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_AddEvent(unsigned int Param_Key, unsigned int Param_EventType)
// Returns TRUE  : Key Event Added Successfully To Queue
// Returns FALSE : Key Event Dropped (Key Invalid / Queue Full)
{
	// Ensure Queue Not Full
	if ( ((Keypad_EventQueue_End + 1) & (kKeypad_EventQueue_MaxSize - 1)) == Keypad_EventQueue_Start )
	{
		Keypad_ResetQueue();
		//return(false); // rax
	}

	// Ensure Valid Key

	if (Param_Key >= kKeypad_OUTOFBOUNDS)
	{
		return(false);
	}

	// Add Event To Queue

	Keypad_EventQueue[Keypad_EventQueue_End] = (unsigned char)((Param_EventType << kKeypad_Bits_KeyID) | Param_Key);

	// Increase Queue End Counter

	Keypad_EventQueue_End = (unsigned char)((Keypad_EventQueue_End + 1) & (kKeypad_EventQueue_MaxSize - 1));

	return(true);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

#define KEY_MULTI_CLICK_THRESSHOLD	4

void CKeypad::Keypad_UpdateState(bool Param_EntireQueue)
{
	unsigned int     KeyID;
	unsigned int     EventType;
	unsigned int     StateChange		= 0;
	bool	DoubleKeyEvent	= false;

	if (Keypad_Repeated_Key_Time >= KEY_MULTI_CLICK_THRESSHOLD)
	{
		Keypad_Repeated_Key_ID = kKeypad_NULL;
		Keypad_Repeated_Key_Count = 0;
	}
	else
	{
		Keypad_Repeated_Key_Time++;
	}

	// Loop : Until Queue Empty or Simultaneous Events On Same Key
	while ((Keypad_EventQueue_Start != Keypad_EventQueue_End) && (!DoubleKeyEvent || Param_EntireQueue))
	{
		// Get Event Type & Key ID

		EventType = Keypad_EventQueue[Keypad_EventQueue_Start] >> kKeypad_Bits_KeyID;
		KeyID     = Keypad_EventQueue[Keypad_EventQueue_Start] &  kKeypad_Mask_KeyID;

		if (EventType == kKeypad_EventType_Pressed)
		{
			if (KeyID != Keypad_Repeated_Key_ID)
			{
				Keypad_Repeated_Key_ID = KeyID;
				Keypad_Repeated_Key_Count = 1;
				Keypad_Repeated_Key_Time = 0;
			}
			else
			{
				Keypad_Repeated_Key_Count++;
				Keypad_Repeated_Key_Time = 0;
			}
		}

		// Ensure No Simultaneous Events On Same Key

		if ((StateChange & (1 << KeyID)) != 0)
		{
			DoubleKeyEvent = true;
		}
		else
		{
			// Ensure Event Should Change Key State

			if (((Keypad_State_Current >> KeyID) & kKeypad_Mask_EventType) != EventType)
			{
				StateChange = StateChange | (1 << KeyID);
			}

			// Increase Queue Start Counter

			Keypad_EventQueue_Start = (unsigned char)((Keypad_EventQueue_Start + 1) & (kKeypad_EventQueue_MaxSize - 1));
		}
	}

	// Flip Key States

	Keypad_State_Old      = Keypad_State_Current;
	Keypad_State_Current ^= StateChange;

}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void CKeypad::Keypad_ResetQueue()
{
    Keypad_EventQueue_Start = Keypad_EventQueue_End;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void CKeypad::Keypad_ResetStates()
{
    Keypad_State_Old = 0;
    Keypad_State_Current = 0;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void CKeypad::Keypad_Reset()
{
    Keypad_ResetQueue();
	Keypad_ResetStates();
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_IsKeyPressed_MenuUp()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_IsKeyPressed( kKeypad_DPad_Up ) || Keypad_IsKeyPressed( kKeypad_2 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_IsKeyPressed( kKeypad_DPad_Up ) || Keypad_IsKeyPressed( kKeypad_6 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_IsKeyPressed( kKeypad_DPad_Up ) || Keypad_IsKeyPressed( kKeypad_4 );
	}
	return false;
}

bool CKeypad::Keypad_IsKeyPressed_MenuDown()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_IsKeyPressed( kKeypad_DPad_Down ) || Keypad_IsKeyPressed( kKeypad_8 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_IsKeyPressed( kKeypad_DPad_Down ) || Keypad_IsKeyPressed( kKeypad_4 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_IsKeyPressed( kKeypad_DPad_Down ) || Keypad_IsKeyPressed( kKeypad_6 );
	}
	return false;
}

bool CKeypad::Keypad_IsKeyPressed_MenuLeft()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_IsKeyPressed( kKeypad_DPad_Left ) || Keypad_IsKeyPressed( kKeypad_4 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_IsKeyPressed( kKeypad_DPad_Left ) || Keypad_IsKeyPressed( kKeypad_2 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_IsKeyPressed( kKeypad_DPad_Left ) || Keypad_IsKeyPressed( kKeypad_8 );
	}
	return false;
}

bool CKeypad::Keypad_IsKeyPressed_MenuRight()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_IsKeyPressed( kKeypad_DPad_Right ) || Keypad_IsKeyPressed( kKeypad_6 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_IsKeyPressed( kKeypad_DPad_Right ) || Keypad_IsKeyPressed( kKeypad_8 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_IsKeyPressed( kKeypad_DPad_Right ) || Keypad_IsKeyPressed( kKeypad_2 );
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CKeypad::Keypad_HasKeyBeenPressed_MenuUp()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Up ) || Keypad_HasKeyBeenPressed( kKeypad_2 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Up ) || Keypad_HasKeyBeenPressed( kKeypad_6 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Up ) || Keypad_HasKeyBeenPressed( kKeypad_4 );
	}
	return false;
}

bool CKeypad::Keypad_HasKeyBeenPressed_MenuDown()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Down ) || Keypad_HasKeyBeenPressed( kKeypad_8 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Down ) || Keypad_HasKeyBeenPressed( kKeypad_4 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Down ) || Keypad_HasKeyBeenPressed( kKeypad_6 );
	}
	return false;
}

bool CKeypad::Keypad_HasKeyBeenPressed_MenuLeft()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Left ) || Keypad_HasKeyBeenPressed( kKeypad_4 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Left ) || Keypad_HasKeyBeenPressed( kKeypad_2 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Left ) || Keypad_HasKeyBeenPressed( kKeypad_8 );
	}
	return false;
}

bool CKeypad::Keypad_HasKeyBeenPressed_MenuRight()
{
	switch (CHighGear::GetInstance()->Gapi().GetDisplayOrientation())
	{
	case ORIENTATION_PORTRAIT:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Right ) || Keypad_HasKeyBeenPressed( kKeypad_6 );
	case ORIENTATION_LANDSCAPE_90:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Right ) || Keypad_HasKeyBeenPressed( kKeypad_8 );
	case ORIENTATION_LANDSCAPE_270:
		return Keypad_HasKeyBeenPressed( kKeypad_DPad_Right ) || Keypad_HasKeyBeenPressed( kKeypad_2 );
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------