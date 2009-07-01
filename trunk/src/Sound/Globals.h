///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2006, 2007 Gameloft. All rights reserved.
//			by
//		Plamen Chernev
//
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

#ifndef __SOUND_GLOBALS_H__
#define __SOUND_GLOBALS_H__

///////////////////////////////////////////////////////////////////////////////
///  Sound engine constants
///////////////////////////////////////////////////////////////////////////////
/// Lowest value for volume in percents
#define MIN_VOLUME  					0

/// Highest value for volume
#define MAX_VOLUME  					100

/// Highest value for volume of SFX stream from MAX_VOLUME in precents
#define MAX_SFX_VOLUME					100

/// Highest value for volume of music stream from MAX_VOLUME in percents
#define MAX_MUSIC_VOLUME				100

/// Lowest value for volume of SFX stream from MAX_VOLUME in precents
#define MIN_SFX_VOLUME					40

/// Lowest value for volume of music stream from MAX_VOLUME in percents
#define MIN_MUSIC_VOLUME				1

#define MAX_MUSIC_VOLUME_DEVICE			75

/// Default value for volume in percents
//#ifdef NGI
//	#define DEFAULT_MASTER_VOLUME 			30
//#else
//	#ifdef WINDOWS_MOBILE_WAV_MUSIC
//		#define DEFAULT_MASTER_VOLUME 			80
//	#else
//		#define DEFAULT_MASTER_VOLUME 			50
//	#endif //WINDOWS_MOBILE_WAV_MUSIC
//#endif //NGI
#define DEFAULT_MASTER_VOLUME 			100

/// Default value for volume from MAX_SFX_VOLUME in percents
#define DEFAULT_SFX_VOLUME 				100

/// Default value for volume from MAX_MUSIC_VOLUME in percents
#define DEFAULT_MUSIC_VOLUME 			MAX_MUSIC_VOLUME

/// Lowest value for balance.
#define MIN_BALANCE 					(-100)

/// Highest value for balance.
#define MAX_BALANCE 					100

/// Default value for balance.
#define DEFAULT_BALANCE 				0

/// Lowest value for frequency in Hz.
#define MIN_FREQUENCY 					1000

/// Highest value for frequency in Hz.
#define MAX_FREQUENCY 					16000
#define DEFAULT_FREQUENCY 				(MAX_FREQUENCY / 2)

/////////////////////////////////////////////////////

#endif // __SOUND_GLOBALS_H__
