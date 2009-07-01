#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include "config.h"

namespace Lib3D
{
	enum
	{
		kInterfaceHeigth = 0,

		// clipping, remenber that camera look on -z for direct axis orientation, these values cannot be changed

		NEAR_CLIP_MIN		= -64,		// MIN = -64, z near depth face draw limit, -64 = min value to avoid overflow in perspective correction

		Z_BUFFER_BITS		= 15,
        FAR_CLIP_DISTANCE 	= 30000, // (1<<Z_BUFFER_BITS),
		FAR_CLIP_MAX		= -FAR_CLIP_DISTANCE,  // MAX = -4096, z far depth draw limit for faces

		DIV_SHIFT	= 16,                           // fixed float bits convertion accuracy
		DIV_MASK	= ((1 << DIV_SHIFT) - 1),

		BLUE_BITS	= 4,					// bits count in video ram
		GREEN_BITS	= 4,
		RED_BITS	= 4,

		BLUE_MASK  = (( 1 << BLUE_BITS ) - 1),    // video memory color mask bits
		GREEN_MASK = (((1 << GREEN_BITS) - 1) <<  BLUE_BITS),
		RED_MASK   = (((1 << RED_BITS  ) - 1) << (BLUE_BITS + GREEN_BITS)),

		NEAR_CLIP	= NEAR_CLIP_MIN,
		FAR_CLIP	= FAR_CLIP_MAX + 100,

		//offset to clip a little more big image than screen, this is require due to fixed pointacy limitations

		CLIP_REJECT		= (4 << 16),                    // clip max allowed error before to reject from frustrum
		
		NFRUST_SHIFT	= 16,                          // shifted value of frustrum normals, must be 16
		SUBDIV_TLR		= 0,                             // activate top/left/right subdivision (do not activate, for debug only)
	};

//extern int kSizeX;
//extern int kSizeY;

}//namespace
#endif // _CONSTANTS_H_
