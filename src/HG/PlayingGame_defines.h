#ifndef PLAYING_GAME__DEFINES__H__INCLUDED__
#define PLAYING_GAME__DEFINES__H__INCLUDED__

#include "Config.h"


#define VPAD_X	((m_dispX >> 1) - 120)
#define VPAD_Y	(m_dispY - 120)
#define VPAD_FIRE_X	((m_dispX >> 1) + 80)
#define VPAD_FIRE_Y	(m_dispY - 80)

#define ZONEID_PAD_LEFT		1
#define ZONEID_PAD_RIGHT	2
#define ZONEID_PAD_FIRE		3

#define MC_XCOORD 2 
#define MC_YCOORD 6

#define LEVEL_UNIT_WIDTH	5
#define LEVEL_UNIT_HEIGHT	7
#define LEVEL_PIXEL_HEIGHT	45
#define LEVEL_PIXEL_WIDTH_SHORT	72
#define LEVEL_PIXEL_WIDTH_LONG	116
#define LEVEL_X_CENTER		(INITIAL_DISP_X >> 1)
#define LEVEL_Y_START		(INITIAL_DISP_Y >> 1) - 57
#define LEVEL_Y_END			(INITIAL_DISP_Y >> 1) - 12


#endif	// PLAYING_GAME__DEFINES__H__INCLUDED__
