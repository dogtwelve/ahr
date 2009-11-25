#ifndef DEMO_MENU_DEFINES__INCLUDED__
#define DEMO_MENU_DEFINES__INCLUDED__

// --- for screen coordinates and other internal stuff

#include "config.h"

#define MENU_FRAME_SPRITE_FRAME_FRAME		0
#define MENU_FRAME_SPRITE_FRAME_GAMELOFT	1

#define ANIM_FRAME_DELAY					5
//#define SOFTKEY_MARGIN						(2)

#if defined (R240x320)
	
	#define SOFTKEY_MARGIN					(2)
	
	#define MENU_FRAME_PORTRAIT_SPLASH_Y	 11
	#define MENU_FRAME_LANDSCAPE_SPLASH_Y	 11

	#define MENU_FRAME_PORTRAIT_FRAME_Y		 11
	#define MENU_FRAME_PORTRAIT_GAMELOFT_Y	(-3)	// --- BOTTOM

	#define MENU_FRAME_LANDSCAPE_FRAME_Y	(-4)
	#define MENU_FRAME_LANDSCAPE_GAMELOFT_Y	(-3)

	#define DEMOMENUMAIN_PORTRAIT_SPACING	20 // 26 // 24
//	#define DEMOMENUMAIN_PORTRAIT_Y			120 //rax: was 90

	#define DEMOMENUMAIN_LANDSCAPE_SPACING	 18 // 22
	#define DEMOMENUMAIN_NGI_LANDSCAPE_SPACING	 17 // 22
//	#define DEMOMENUMAIN_LANDSCAPE_Y		59


	#define DARKENAREA_TOP_MARGIN_MAIN_MENU_OFFSET	20
	#define DARKENAREA_TOP_MARGIN_PORTRAIT			60// 80
	#define DARKENAREA_TOP_MARGIN_LANDSCAPE			62

	#define DEMOMENUOPTIONS_PORTRAIT_TITLE_OFFSET (-20)

	#define DEMOMENUOPTIONS_PORTRAIT_Y_OFFSET 0
	#define DEMOMENUOPTIONS_PORTRAIT_SPACING 34

	#define DEMOMENUOPTIONS_LANDSCAPE_TITLE_OFFSET (-20)
	#define DEMOMENUOPTIONS_LANDSCAPE_Y_OFFSET (-22)
	#define DEMOMENUOPTIONS_LANDSCAPE_SPACING 30

//---------------------------------------------------------------------------
	#define DEMOMENU_MP_PORTRAIT_TITLE_OFFSET (-21)

	#define DEMOMENU_MP_PORTRAIT_Y_OFFSET (20)
	#define DEMOMENU_MP_PORTRAIT_SPACING 22

	#define DEMOMENU_MP_LANDSCAPE_TITLE_OFFSET (-25)

	#define DEMOMENU_MP_LANDSCAPE_Y_OFFSET (-22 - 24)
	#define DEMOMENU_MP_LANDSCAPE_SPACING 22
//---------------------------------------------------------------------------


	#define DEMOMENUGAMEPLAY_PORTRAIT_Y_OFFSET 50
	#define DEMOMENUGAMEPLAY_PORTRAIT_SPACING 33

	#define DEMOMENUGAMEPLAY_LANDSCAPE_Y_OFFSET 13
	#define DEMOMENUGAMEPLAY_LANDSCAPE_SPACING 30

	#define DEMOMENU_SELECTION_BAR_Y_OFFSET 12

	#define DEMOMENULANGUAGE_PORTRAIT_TITLE_OFFSET 0
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y 8
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y_CHINESE 5

	#define DEMOMENULANGUAGE_PORTRAIT_Y 90
	#define DEMOMENULANGUAGE_PORTRAIT_SPACING 27

	#define DEMOMENULANGUAGE_LANDSCAPE_Y 45
	#define DEMOMENULANGUAGE_LANDSCAPE_SPACING 25

	#define DEMOMENUDISPLAYMODE_PORTRAIT_SPACING 33

	#define DEMOMENUDISPLAYMODE_LANDSCAPE_SPACING 30

	#define DEMOMENUCONTROLS_MODE_PORTRAIT_OFFSET_Y 55
	#define DEMOMENUCONTROLS_MODE_PORTRAIT_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_OFFSET_Y 35
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING 35
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING_CHINESE 44

	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_OFFSET_Y 18
	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_OFFSET_Y (28)
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING 35
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING_CHINESE 39

	#define CONTROLS_SHOW_ITMES_PORTRAIT_NB	5
	#define CONTROLS_SHOW_ITMES_LANDSCAPE_NB 3

	#define HELP_Y_TITLE (IS_PORTRAIT ? 30 : 15)
	#define HELP_Y_TEXT_OFFSET (IS_PORTRAIT ? (-40) : (-30))

	#define HELP_LINE_HEIGHT		12 // (IS_PORTRAIT ? 12 : 10)
	#define HELP_BLANK_LINE_HEIGHT	10 // (IS_PORTRAIT ? 10 : 5)
#ifdef USE_CHINESE_UNICODE_FONT
	#define HELP_LINE_HEIGHT_CHINESE (IS_PORTRAIT ? 13 : 13)
	#define HELP_BLANK_LINE_HEIGHT_CHINESE (IS_PORTRAIT ? 7 : 3)
	#define HELP_BLANK_LINE_CAPTION 5;
#endif
	#define MENU_INGAME_TITLE_HEIGHT	30
	#define MENU_INGAME_BOTTOM_HEIGHT	10

	#define MAIN_MENU_X					80 //SEFU ((m_dispX / 2) + (IS_PORTRAIT ? 10 : 20))
#if defined NGI || defined NGI_WIN32
	#define MAIN_MENU_PORTRAIT_Y		100
#else
	#define MAIN_MENU_PORTRAIT_Y		120
#endif
	#define MAIN_MENU_LANDSCAPE_Y		96
	#define MAIN_MENU_NGI_LANDSCAPE_Y	83
	#define MAIN_MENU_SELECTION_BAR_DX	(- 14)
	#define MAIN_MENU_ARROW_DX			(- 38)
	#define MAIN_MENU_ARROW_DY			(-2)

	#define INGAME_MENU_BOX_W			160
	#define INGAME_MENU_SEL_W			120
	#define INGAME_MENU_SEL_H			16

	#define PRESS_ANY_KEY_Y				(IS_PORTRAIT ? m_dispY - 40 : m_dispY - 40)
	
	#define MENU_TITLE_MAX_TEXT_WIDTH	(m_dispX - 30)

#ifdef HAS_MULTIPLAYER

	#define MP_JOIN_MENU_X								(IS_PORTRAIT ? 70 : 70)
	#define MP_MENU_CONFIRM_CONNECTION_TITLE_Y			(IS_PORTRAIT ? 70 : 70) // TODO 
	#define MP_MENU_CONFIRM_CONNECTION_SPACING_Y		(14)
	#define MP_MENU_CONFIRM_CONNECTION_CLIENT_NAME_Y	(MP_MENU_CONFIRM_CONNECTION_TITLE_Y + MP_MENU_CONFIRM_CONNECTION_SPACING_Y)	 // TODO 
	#define MP_MENU_CONFIRM_CONNECTION_Y				(MP_MENU_CONFIRM_CONNECTION_CLIENT_NAME_Y + 2*MP_MENU_CONFIRM_CONNECTION_SPACING_Y)

#endif // HAS_MULTIPLAYER

#elif defined R352x416
	
	#define SOFTKEY_MARGIN					(2)

	#define MENU_FRAME_PORTRAIT_SPLASH_Y	 11
	#define MENU_FRAME_LANDSCAPE_SPLASH_Y	 11

	#define MENU_FRAME_PORTRAIT_FRAME_Y		 11
	#define MENU_FRAME_PORTRAIT_GAMELOFT_Y	(-3)	// --- BOTTOM

	#define MENU_FRAME_LANDSCAPE_FRAME_Y	(-4)
	#define MENU_FRAME_LANDSCAPE_GAMELOFT_Y	(-3)

	#define DEMOMENUMAIN_PORTRAIT_SPACING 24
	#define DEMOMENUMAIN_PORTRAIT_Y (90 + 48)

	#define DEMOMENUMAIN_LANDSCAPE_SPACING 22
	#define DEMOMENUMAIN_LANDSCAPE_Y (63 + 48)

	// --- set according to sprite, not screen
	#define DARKENAREA_TOP_MARGIN_PORTRAIT 80
	#define DARKENAREA_TOP_MARGIN_LANDSCAPE 62

	#define DEMOMENUOPTIONS_PORTRAIT_TITLE_OFFSET (-20)

	#define DEMOMENUOPTIONS_PORTRAIT_Y_OFFSET (0 + 48)
	#define DEMOMENUOPTIONS_PORTRAIT_SPACING 34

	#define DEMOMENUOPTIONS_LANDSCAPE_TITLE_OFFSET (-20)
	
	#define DEMOMENUOPTIONS_LANDSCAPE_Y_OFFSET (-22 + 48)
	#define DEMOMENUOPTIONS_LANDSCAPE_SPACING 30

//---------------------------------------------------------------------------
	#define DEMOMENU_MP_PORTRAIT_TITLE_OFFSET (-21)

	#define DEMOMENU_MP_PORTRAIT_Y_OFFSET (0 - 6)
	#define DEMOMENU_MP_PORTRAIT_SPACING 22

	#define DEMOMENU_MP_LANDSCAPE_TITLE_OFFSET (-25)

	#define DEMOMENU_MP_LANDSCAPE_Y_OFFSET (-22 - 24)
	#define DEMOMENU_MP_LANDSCAPE_SPACING 22
//---------------------------------------------------------------------------

	#define DEMOMENUGAMEPLAY_PORTRAIT_Y_OFFSET 50
	#define DEMOMENUGAMEPLAY_PORTRAIT_SPACING 33

	#define DEMOMENUGAMEPLAY_LANDSCAPE_Y_OFFSET 13
	#define DEMOMENUGAMEPLAY_LANDSCAPE_SPACING 30

	#define DEMOMENU_SELECTION_BAR_Y_OFFSET 12

	#define DEMOMENULANGUAGE_PORTRAIT_TITLE_OFFSET 0
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y 8
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y_CHINESE 5

	#define DEMOMENULANGUAGE_PORTRAIT_Y 90
	#define DEMOMENULANGUAGE_PORTRAIT_SPACING 27

	#define DEMOMENULANGUAGE_LANDSCAPE_Y 45
	#define DEMOMENULANGUAGE_LANDSCAPE_SPACING 25

	#define DEMOMENUDISPLAYMODE_PORTRAIT_SPACING 33

	#define DEMOMENUDISPLAYMODE_LANDSCAPE_SPACING 30

	#define DEMOMENUCONTROLS_MODE_PORTRAIT_OFFSET_Y 55
	#define DEMOMENUCONTROLS_MODE_PORTRAIT_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_OFFSET_Y 12
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING 35
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING_CHINESE 44

	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_OFFSET_Y 18
	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_OFFSET_Y (-17)
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING 32
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING_CHINESE 39

	#define CONTROLS_SHOW_ITMES_NB	5

	#define HELP_Y_TITLE (IS_PORTRAIT ? 20 : 15)
	#define HELP_Y_TEXT_OFFSET (IS_PORTRAIT ? (-40) : (-30))

	#define HELP_LINE_HEIGHT (IS_PORTRAIT ? 12 : 10)
	#define HELP_BLANK_LINE_HEIGHT (IS_PORTRAIT ? 10 : 5)

#elif defined R176x208
	
	#define SOFTKEY_MARGIN					(2)

	#define MENU_FRAME_PORTRAIT_SPLASH_Y	  6
	#define MENU_FRAME_LANDSCAPE_SPLASH_Y	(-4)

	#define MENU_FRAME_PORTRAIT_FRAME_Y		  6
	#define MENU_FRAME_PORTRAIT_GAMELOFT_Y	(-3)	// --- BOTTOM

	#define MENU_FRAME_LANDSCAPE_FRAME_Y	(-4)
	#define MENU_FRAME_LANDSCAPE_GAMELOFT_Y	(-3)

	#define DEMOMENUMAIN_PORTRAIT_SPACING 17
	#define DEMOMENUMAIN_PORTRAIT_Y (90 - 32)

	#define DEMOMENUMAIN_LANDSCAPE_SPACING 20
	#define DEMOMENUMAIN_LANDSCAPE_Y (63 - 36)

	// --- set according to sprite, not screen
	#define DARKENAREA_TOP_MARGIN_MAIN_MENU_OFFSET	16
	#define DARKENAREA_TOP_MARGIN_PORTRAIT (80 - 24)
	#define DARKENAREA_TOP_MARGIN_LANDSCAPE 62

	#define DEMOMENUOPTIONS_PORTRAIT_TITLE_OFFSET (-21)

	#define DEMOMENUOPTIONS_PORTRAIT_Y_OFFSET (0 - 6)
	#define DEMOMENUOPTIONS_PORTRAIT_SPACING 22

	#define DEMOMENUOPTIONS_LANDSCAPE_TITLE_OFFSET (-25)

	#define DEMOMENUOPTIONS_LANDSCAPE_Y_OFFSET (-22 - 24)
	#define DEMOMENUOPTIONS_LANDSCAPE_SPACING 22

//---------------------------------------------------------------------------
	#define DEMOMENU_MP_PORTRAIT_TITLE_OFFSET (-21)

	#define DEMOMENU_MP_PORTRAIT_Y_OFFSET (0 - 6)
	#define DEMOMENU_MP_PORTRAIT_SPACING 22

	#define DEMOMENU_MP_LANDSCAPE_TITLE_OFFSET (-25)

	#define DEMOMENU_MP_LANDSCAPE_Y_OFFSET (-22 - 24)
	#define DEMOMENU_MP_LANDSCAPE_SPACING 22
//---------------------------------------------------------------------------

	#define DEMOMENUGAMEPLAY_PORTRAIT_Y_OFFSET (0 - 6)
	#define DEMOMENUGAMEPLAY_PORTRAIT_SPACING 33

	#define DEMOMENUGAMEPLAY_LANDSCAPE_Y_OFFSET (13 - 24)
	#define DEMOMENUGAMEPLAY_LANDSCAPE_SPACING 30

	#define DEMOMENU_SELECTION_BAR_Y_OFFSET 8

	#define DEMOMENULANGUAGE_PORTRAIT_TITLE_OFFSET		DEMOMENUOPTIONS_PORTRAIT_TITLE_OFFSET
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y (8 - 4)
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y_CHINESE (5 - 4)

	#define DEMOMENULANGUAGE_PORTRAIT_Y 40
	#define DEMOMENULANGUAGE_PORTRAIT_SPACING 21

	#define DEMOMENULANGUAGE_LANDSCAPE_Y 66
	#define DEMOMENULANGUAGE_LANDSCAPE_SPACING 19

	#define DEMOMENUDISPLAYMODE_PORTRAIT_SPACING 21

	#define DEMOMENUDISPLAYMODE_LANDSCAPE_SPACING 21

	#define DEMOMENUCONTROLS_MODE_PORTRAIT_OFFSET_Y (55 - 45)
	#define DEMOMENUCONTROLS_MODE_PORTRAIT_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_OFFSET_Y (12 - 15)
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING 35
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING_CHINESE 44

	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_OFFSET_Y 18
	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_OFFSET_Y (-17)
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING 32
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING_CHINESE 39

	#define CONTROLS_SHOW_ITMES_PORTRAIT_NB	5
	#define CONTROLS_SHOW_ITMES_LANDSCAPE_NB 3

	#define CONTROLS_SHOW_ITMES_NB	3

	#define HELP_Y_TITLE (IS_PORTRAIT ? 20 : 15)
	#define HELP_Y_TEXT_OFFSET (IS_PORTRAIT ? (-20) : (-10))

	#define HELP_LINE_HEIGHT (IS_PORTRAIT ? 8 : 8)
	#define HELP_BLANK_LINE_HEIGHT (IS_PORTRAIT ? 5 : 5)

	#define MENU_INGAME_TITLE_HEIGHT	20
	#define MENU_INGAME_BOTTOM_HEIGHT	10

//geo
	#define MAIN_MENU_X					48//60 //SEFU ((m_dispX / 2) + 10)
	#define MAIN_MENU_PORTRAIT_Y		73//85 //SEFU
	#define MAIN_MENU_LANDSCAPE_Y		104
	#define MAIN_MENU_SELECTION_BAR_DX	(- 16)
	#define MAIN_MENU_ARROW_DX			(- 26)
	#define MAIN_MENU_ARROW_DY			(-2)

	#define INGAME_MENU_BOX_W			120
	#define INGAME_MENU_SEL_W			80
	#define INGAME_MENU_SEL_H			14

	#define PRESS_ANY_KEY_Y				(IS_PORTRAIT ? m_dispY - 30 : m_dispY - 30)

	#define MENU_TITLE_MAX_TEXT_WIDTH	(m_dispX - 30)

#ifdef HAS_MULTIPLAYER

	#define MP_JOIN_MENU_X								(IS_PORTRAIT ? 40 : 70)
	#define MP_MENU_CONFIRM_CONNECTION_TITLE_Y			(IS_PORTRAIT ? 90 : 40) // TODO 
	#define MP_MENU_CONFIRM_CONNECTION_SPACING_Y		(14)
	#define MP_MENU_CONFIRM_CONNECTION_CLIENT_NAME_Y	(MP_MENU_CONFIRM_CONNECTION_TITLE_Y + MP_MENU_CONFIRM_CONNECTION_SPACING_Y)	 // TODO 
	#define MP_MENU_CONFIRM_CONNECTION_Y				(MP_MENU_CONFIRM_CONNECTION_CLIENT_NAME_Y + 2*MP_MENU_CONFIRM_CONNECTION_SPACING_Y)

#endif // HAS_MULTIPLAYER

#elif defined R320x480

	#define SOFTKEY_MARGIN					(20)
	#define SOFTKEY_MARGIN_X				(60)
	#define SOFTKEY_MARGIN_Y				(25)
	
#ifdef USE_TOUCH_SCREEN
	#define SOFTKEY_TOUCH_ZONE_W			120
	#define SOFTKEY_TOUCH_ZONE_H			50
#endif

	#define MENU_FRAME_PORTRAIT_SPLASH_Y	 11
	#define MENU_FRAME_LANDSCAPE_SPLASH_Y	 11

	#define MENU_FRAME_PORTRAIT_FRAME_Y		 11
	#define MENU_FRAME_PORTRAIT_GAMELOFT_Y	(-3)	// --- BOTTOM

	#define MENU_FRAME_LANDSCAPE_FRAME_Y	(-4)
	#define MENU_FRAME_LANDSCAPE_GAMELOFT_Y	(-3)

	#define DEMOMENUMAIN_PORTRAIT_SPACING	50 // 20 // 26 // 24
//	#define DEMOMENUMAIN_PORTRAIT_Y			120 //rax: was 90

	#define DEMOMENUMAIN_LANDSCAPE_SPACING	45 //18 // 22
//	#define DEMOMENUMAIN_LANDSCAPE_Y		59


	#define DARKENAREA_TOP_MARGIN_MAIN_MENU_OFFSET	20
	#define DARKENAREA_TOP_MARGIN_PORTRAIT			60// 80
	#define DARKENAREA_TOP_MARGIN_LANDSCAPE			62

	#define DEMOMENUOPTIONS_PORTRAIT_TITLE_OFFSET (-20)

	#define DEMOMENUOPTIONS_PORTRAIT_Y_OFFSET 0
	#define DEMOMENUOPTIONS_PORTRAIT_SPACING 34

	#define DEMOMENUOPTIONS_LANDSCAPE_TITLE_OFFSET (-20)
	#define DEMOMENUOPTIONS_LANDSCAPE_Y_OFFSET (-22)
	#define DEMOMENUOPTIONS_LANDSCAPE_SPACING 30

//---------------------------------------------------------------------------
	#define DEMOMENU_MP_PORTRAIT_TITLE_OFFSET (-21)

	#define DEMOMENU_MP_PORTRAIT_Y_OFFSET (20)
	#define DEMOMENU_MP_PORTRAIT_SPACING 22

	#define DEMOMENU_MP_LANDSCAPE_TITLE_OFFSET (-25)

	#define DEMOMENU_MP_LANDSCAPE_Y_OFFSET (-22 - 24)
	#define DEMOMENU_MP_LANDSCAPE_SPACING 22
//---------------------------------------------------------------------------


	#define DEMOMENUGAMEPLAY_PORTRAIT_Y_OFFSET 50
	#define DEMOMENUGAMEPLAY_PORTRAIT_SPACING 33

	#define DEMOMENUGAMEPLAY_LANDSCAPE_Y_OFFSET 13
	#define DEMOMENUGAMEPLAY_LANDSCAPE_SPACING 30

	#define DEMOMENU_SELECTION_BAR_Y_OFFSET 12

	#define DEMOMENULANGUAGE_PORTRAIT_TITLE_OFFSET 0
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y 8
	#define DEMOMENULANGUAGE_SELECTIONBAR_OFFSET_Y_CHINESE 5

	#define DEMOMENULANGUAGE_PORTRAIT_Y 90
	#define DEMOMENULANGUAGE_PORTRAIT_SPACING 27

	#define DEMOMENULANGUAGE_LANDSCAPE_Y 45
	#define DEMOMENULANGUAGE_LANDSCAPE_SPACING 25

	#define DEMOMENUDISPLAYMODE_PORTRAIT_SPACING 33

	#define DEMOMENUDISPLAYMODE_LANDSCAPE_SPACING 30

	#define DEMOMENUCONTROLS_MODE_PORTRAIT_OFFSET_Y 55
	#define DEMOMENUCONTROLS_MODE_PORTRAIT_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_OFFSET_Y 12
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING 35
	#define DEMOMENUCONTROLS_KEYS_PORTRAIT_SPACING_CHINESE 44

	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_OFFSET_Y 18
	#define DEMOMENUCONTROLS_MODE_LANDSCAPE_SPACING 23
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_OFFSET_Y (-17)
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING 32
	#define DEMOMENUCONTROLS_KEYS_LANDSCAPE_SPACING_CHINESE 39

	#define CONTROLS_SHOW_ITMES_PORTRAIT_NB	5
	#define CONTROLS_SHOW_ITMES_LANDSCAPE_NB 3

	#define HELP_Y_TITLE (IS_PORTRAIT ? 20 : 15)
	#define HELP_Y_TEXT_OFFSET (IS_PORTRAIT ? (-40) : (-30))

	#define HELP_LINE_HEIGHT (IS_PORTRAIT ? 12 : 10)
	#define HELP_BLANK_LINE_HEIGHT (IS_PORTRAIT ? 10 : 5)

	#define MENU_INGAME_TITLE_HEIGHT	30
	#define MENU_INGAME_BOTTOM_HEIGHT	10

	#define MAIN_MENU_X					80 //SEFU ((m_dispX / 2) + (IS_PORTRAIT ? 10 : 20))
	#define MAIN_MENU_PORTRAIT_Y		120
	#define MAIN_MENU_LANDSCAPE_Y		104
	#define MAIN_MENU_SELECTION_BAR_DX	(- 20)
	#define MAIN_MENU_ARROW_DX			(- 38)
	#define MAIN_MENU_ARROW_DY			(-2)

	#define INGAME_MENU_BOX_W			252
	#define INGAME_MENU_SEL_W			252
	
#ifdef HAS_MENU_BUTTONS
	#define INGAME_MENU_SEL_H			48
#else
	#define INGAME_MENU_SEL_H			16
#endif

	#define PRESS_ANY_KEY_Y				(IS_PORTRAIT ? m_dispY - 36 : m_dispY - 36)
	
	#define MENU_TITLE_MAX_TEXT_WIDTH	(m_dispX - 30)

#ifdef HAS_MULTIPLAYER

#ifdef HAS_MENU_BUTTONS
	#define MP_JOIN_MENU_X								(m_dispX / 2)
#else
	#define MP_JOIN_MENU_X								(IS_PORTRAIT ? 40 : 70)
#endif
	#define MP_MENU_CONFIRM_CONNECTION_TITLE_Y			(IS_PORTRAIT ? 90 : 40) // TODO 
	#define MP_MENU_CONFIRM_CONNECTION_SPACING_Y		(14)
	#define MP_MENU_CONFIRM_CONNECTION_CLIENT_NAME_Y	(MP_MENU_CONFIRM_CONNECTION_TITLE_Y + MP_MENU_CONFIRM_CONNECTION_SPACING_Y)	 // TODO 
	#define MP_MENU_CONFIRM_CONNECTION_Y				(MP_MENU_CONFIRM_CONNECTION_CLIENT_NAME_Y + 2*MP_MENU_CONFIRM_CONNECTION_SPACING_Y)

#endif // HAS_MULTIPLAYER

#else
	#error please define one of them
#endif

#define CONTROLS_SCROLL_ITEMS		(IS_PORTRAIT ? 4 : 3)
#define LANGUAGE_SCROLL_ITEMS		(IS_PORTRAIT ? 9 : 6)
#define MAINMENU_SCROLL_ITEMS		(IS_PORTRAIT ? 9 : 6)

#define MENU_INGAME_HELP_MARGIN_X	8

// Font palettes
#define FONT_LARGE_P_RED			0
#define FONT_LARGE_P_WHITE			1
#define FONT_LARGE_P_BLUE			2

#define FONT_NORMAL_P_BLUE					0
#define FONT_NORMAL_P_WHITE					1
#define FONT_NORMAL_P_RED					2
#define FONT_NORMAL_P_GREEN					3

#define FONT_SMALL_P_GRAY					0
#define FONT_SMALL_P_DARK					1
#define FONT_SMALL_P_WHITE					2

#define TUNNING_PALETTE_BETTER				FONT_NORMAL_P_GREEN
#define TUNNING_PALETTE_WORSE				FONT_NORMAL_P_RED

#define TRACK_SELECTION_P_TRACK_SELECTION_GREY	0

#define MENU_ARROWS_P_MENU_ARROWS_WHITE		0

// Volume sprite
#define VOLUME_SCROLL_P_SEL					0
#define VOLUME_SCROLL_P_BKG					1
#define VOLUME_SCROLL_P_SEL_GRAY			2
#define VOLUME_SCROLL_P_BKG_GRAY			3

// Copyright (hcenter, bottom)
#define COPYRIGHT_X							(lib2d.m_dispX >> 1)

//geo
#if defined R176x208
	#define COPYRIGHT_Y							(lib2d.m_dispY - 10)
#else
	#define COPYRIGHT_Y							(IS_PORTRAIT_REF(lib2d) ? lib2d.m_dispY - 20 : lib2d.m_dispY - 4)
#endif

#define COPYRIGHT_SPLASH_Y						(lib2d.m_dispY - 4)

#endif	// ! DEMO_MENU_DEFINES__INCLUDED__
