#ifndef PLAYING_GAME__DEFINES__H__INCLUDED__
#define PLAYING_GAME__DEFINES__H__INCLUDED__

#include "Config.h"

#define GAMESPRITE_MC		m_gameSprite[0]
#define GAMESPRITE_MUMMY	m_gameSprite[1]
#define GAMESPRITE_VAMPIRE	m_gameSprite[2]
#define GAMESPRITE_SKULL	m_gameSprite[3]
#define GAMESPRITE_MCBULLET	m_gameSprite[4]

#define MAINCHAR			m_actors[0]
#define VPAD_X	((m_dispX >> 1) - 120)
#define VPAD_Y	(m_dispY - 120)
#define VPAD_FIRE_X	((m_dispX >> 1) + 80)
#define VPAD_FIRE_Y	(m_dispY - 80)

#define ZONEID_PAD_LEFT		1
#define ZONEID_PAD_RIGHT	2
#define ZONEID_PAD_FIRE		3



#define LEVEL_UNIT_WIDTH	11
#define LEVEL_UNIT_HEIGHT	21
#define LEVEL_PIXEL_HEIGHT	125
#define LEVEL_PIXEL_WIDTH_SHORT	184
#define LEVEL_PIXEL_WIDTH_LONG	300
#define LEVEL_X_CENTER		(INITIAL_DISP_X >> 1)
#define LEVEL_Y_START		(INITIAL_DISP_Y >> 1) - LEVEL_PIXEL_HEIGHT + 24
#define LEVEL_Y_END			(INITIAL_DISP_Y >> 1)

#define MC_XCOORD (LEVEL_UNIT_WIDTH / 2) 
#define MC_YCOORD (LEVEL_UNIT_HEIGHT - 1)

#define ENEMY_HP_BAR_OFFSET_Y	(- 30)
#define ENEMY_HP_BAR_WIDTH		20
#define ENEMY_HP_BAR_HEIGHT		4
#define ENEMY_HP_BAR_COLOR_BORDER	0xFFFF
#define ENEMY_HP_BAR_COLOR			0xFF00
#define ENEMY_HP_BAR_COLOR_EMPTY	0x0

#define TIME_LIMIT	300000

#define MAX_ENEMY	3
#define MAX_VILLAGE	100
#endif	// PLAYING_GAME__DEFINES__H__INCLUDED__
