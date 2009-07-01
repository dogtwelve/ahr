#ifndef __KEYPAD__
#define __KEYPAD__

// Generic Keypad Engine For Cellphone Games
// Highly Responsive, Multiple-Input Compatible, Mostly Foolproof

// By Alexandre David
// (C)2003 Gameloft Canada

// ............................................................................
// ............................................................................
// Constants
// ............................................................................
// ............................................................................
#define kKeypad_EventType_Released 0
#define kKeypad_EventType_Pressed  1

#define kKeypad_Update_Smart       false
#define kKeypad_Update_Flush       true

#define kKeypad_NULL               0
#define kKeypad_0                  1
#define kKeypad_1                  2
#define kKeypad_2                  3
#define kKeypad_3                  4
#define kKeypad_4                  5
#define kKeypad_5                  6
#define kKeypad_6                  7
#define kKeypad_7                  8
#define kKeypad_8                  9
#define kKeypad_9                  10
#define kKeypad_Pound              11
#define kKeypad_Star               12
#define kKeypad_Gaming_Key_A       13
#define kKeypad_Gaming_Key_B       14
#define kKeypad_DPad_Up            15
#define kKeypad_DPad_Left          16
#define kKeypad_DPad_Right         17
#define kKeypad_DPad_Down          18
#define kKeypad_DPad_Click         19
#define kKeypad_C                  20
#define kKeypad_Pen                21
#define kKeypad_SoftKey_Left       22
#define kKeypad_SoftKey_Right      23
#define kKeypad_Phone_Green        24
#define kKeypad_Phone_Red          25
#define kKeypad_Menu               26
#define kKeypad_Volume_Up		   28
#define kKeypad_Volume_Down	       29
#define kKeypad_OUTOFBOUNDS        30

#define kKeypad_EventQueue_MaxSize 16

#define kKeypad_Bits_KeyID         6
#define kKeypad_Bits_EventType     1

#define kKeypad_Mask_KeyID         ((1 << kKeypad_Bits_KeyID    ) - 1)
#define kKeypad_Mask_EventType     ((1 << kKeypad_Bits_EventType) - 1)

#define kKeypad_Get_Key_Mask( key )		(1<<key)

#define kKeypad_Last_Key_Code		kKeypad_Gaming_Key_B
class CKeypad
{
public:
			CKeypad						();

	bool	Keypad_IsKeyPressed			(unsigned int Param_Key);
	bool	Keypad_HasKeyChanged		(unsigned int Param_Key);
	bool	Keypad_HasKeyBeenPressed	(unsigned int Param_Key);
	bool	Keypad_IsAnyKeyPressed		();
	bool	Keypad_HasAnyKeyChanged		();
	bool	Keypad_HasAnyKeyBeenPressed	();
	int		Keypad_GetLastPressedKeyCode();
	bool	Keypad_IsActionKeyBeenPressed( unsigned int action_key, bool permanently = false );
	bool	Keypad_HasActionKeyBeenDoublePressed	(unsigned int action_key);

	bool	Keypad_IsKeyPressed_MenuUp();
	bool	Keypad_IsKeyPressed_MenuDown();
	bool	Keypad_IsKeyPressed_MenuLeft();
	bool	Keypad_IsKeyPressed_MenuRight();

	bool	Keypad_HasKeyBeenPressed_MenuUp();
	bool	Keypad_HasKeyBeenPressed_MenuDown();
	bool	Keypad_HasKeyBeenPressed_MenuLeft();
	bool	Keypad_HasKeyBeenPressed_MenuRight();

	void    Keypad_RemoveActionKey		( unsigned int action_key );
	bool	Keypad_AddEvent				(unsigned int Param_Key, unsigned int Param_EventType);
	void	Keypad_UpdateState			(bool Param_EntireQueue);
	void	Keypad_ResetQueue			();
    void    Keypad_ResetStates          ();
	void	Keypad_Reset				();

private:
	unsigned int	Keypad_State_Old;
	unsigned int	Keypad_State_Current;
	unsigned char	Keypad_EventQueue_Start;
	unsigned char	Keypad_EventQueue_End;

	unsigned char	Keypad_Repeated_Key_ID;
	unsigned char	Keypad_Repeated_Key_Time;
	unsigned char	Keypad_Repeated_Key_Count;

	unsigned char	Keypad_EventQueue[kKeypad_EventQueue_MaxSize];
};

#endif
