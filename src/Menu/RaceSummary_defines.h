#ifndef RACE_SUMMARY__DEFINES__H__INCLUDED__
#define RACE_SUMMARY__DEFINES__H__INCLUDED__

#include "config.h"

#ifdef R176x208

	#define BOTTOMBARINFO_Y (IS_PORTRAIT_REF(lib2d) ? lib2d.m_dispY - 34 : lib2d.m_dispY - 34)

	#define PAGESTATS_Y 30
	#define PAGESTATS_H 140

	#define UNLOCKED_Y			(30)
//geo
	#define UNLOCKED_STEP_1		(22)	// step between texts
//geo
	#define UNLOCKED_STEP_2		(25)	// step between texts
	#define UNLOCKED_STEP_3		24 // step between tuning items
	#define UNLOCKED_STEP_EVENTS 13//16
	#define UNLOCKED_STEP_TUNING 24
	#define UNLOCKED_STEP_BIG	(32)
	#define UNLOCKED_TOTAL_MONEY_Y (lib2d.m_dispY - 60)

	#define SUMMARY_STEP_Y	(12)//(18)
	#define RACE_MONEY_Y	(lib2d.m_dispY - 10 - 3 * SUMMARY_STEP_Y)
	#define TOTAL_MONEY_Y	(RACE_MONEY_Y + SUMMARY_STEP_Y)
	#define PROGRESSION_Y	(TOTAL_MONEY_Y + SUMMARY_STEP_Y)

#else	// ! R176x208

	#define BOTTOMBARINFO_Y (IS_PORTRAIT_REF(lib2d) ? lib2d.m_dispY - 33 : lib2d.m_dispY - 26)

	#define PAGESTATS_Y (IS_PORTRAIT_REF(lib2d) ? 170 : 97)
	#define PAGESTATS_H (IS_PORTRAIT_REF(lib2d) ? 230 : 154)

	#define UNLOCKED_Y			(IS_PORTRAIT_REF(lib2d) ? 30 : 25)
	#define UNLOCKED_STEP_1		(IS_PORTRAIT_REF(lib2d) ? 26 : 25)	// step between texts
	#define UNLOCKED_STEP_2		(IS_PORTRAIT_REF(lib2d) ? 30 : 25)	// step between texts
	#define UNLOCKED_STEP_3		30 // (IS_PORTRAIT_REF(lib2d) ? 26 : 22) // step between tuning items
	#define UNLOCKED_STEP_EVENTS (IS_PORTRAIT_REF(lib2d) ? 20 : 24)
	#define UNLOCKED_STEP_TUNING (IS_PORTRAIT_REF(lib2d) ? 32 : 40)
	#define UNLOCKED_STEP_BIG (IS_PORTRAIT_REF(lib2d) ? 52 : 44)
	#define UNLOCKED_TOTAL_MONEY_Y (IS_PORTRAIT_REF(lib2d) ? lib2d.m_dispY - 60: lib2d.m_dispY - 40)

	#define SUMMARY_STEP_Y	(IS_PORTRAIT_REF(lib2d) ? 18 : 14)
	#define RACE_MONEY_Y	(IS_PORTRAIT_REF(lib2d) ? (lib2d.m_dispY - 48 - 3 * SUMMARY_STEP_Y) : (lib2d.m_dispY - 56 - 3 * SUMMARY_STEP_Y))
	#define TOTAL_MONEY_Y	(IS_PORTRAIT_REF(lib2d) ? 415 : RACE_MONEY_Y + 36)
	#define PROGRESSION_Y	(IS_PORTRAIT_REF(lib2d) ? TOTAL_MONEY_Y + SUMMARY_STEP_Y : TOTAL_MONEY_Y + 13)

	#define TEXT_OFFSET_COL1_X	(IS_PORTRAIT_REF(lib2d) ? lib2d.m_dispX/2 : 5)
	#define TEXT_ANCHOR_COL1			(IS_PORTRAIT_REF(lib2d) ? ALIGN_CENTERED_TEXT : ALIGN_LEFT_TEXT)

	#define TEXT_OFFSET_COL2_X	(IS_PORTRAIT_REF(lib2d) ? lib2d.m_dispX/2 : lib2d.m_dispX/2 + lib2d.m_dispX/4 + 8)
	#define TEXT_OFFSET_COL2_Y		96
	
	#define TUNING_UNLOCKED_ICONS_X		(IS_PORTRAIT_REF(lib2d) ? 0 : 132)


#endif	// ! R176x208

#define SOFTKEYS_P_OK_BACK_PINK			0

#endif	// ! RACE_SUMMARY__DEFINES__H__INCLUDED__
