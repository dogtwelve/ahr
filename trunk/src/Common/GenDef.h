#ifndef __GENDEF_H__
#define __GENDEF_H__

#include "Config.h"
#include "Gapi.h"
#include "DevUtil.h"

typedef unsigned int	u32;
typedef signed int		s32;
typedef unsigned short	u16;
typedef signed short	s16;
typedef unsigned char	u8;
typedef signed char		s8;
typedef float			f32;


	#ifdef __SYMBIAN32__
#ifdef NGI
	// hack to get rid of the warnings that are generated when 
	// e32std.h and standarddefines.h are both included
	#if defined TRUE
		#undef TRUE
	#endif
	#if defined FALSE
		#undef FALSE
	#endif
#endif // NGI

#ifdef SYMBIAN9
	#include <E32CMN.H>
#endif  // SYMBIAN9
	#include <E32BASE.H>
	#include <E32STD.H>

#ifdef NGI
	#include <standarddefines.h>
#endif // NGI

#endif // __SYMBIAN32__

#endif // ifndef __GENDEF_H__
