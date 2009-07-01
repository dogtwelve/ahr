#ifndef __AALINE_H
#define __AALINE_H

#include "Lib2D/Image.h"
class Vector2s;

template <typename T> static inline void my_swap(T& a, T& b)
{
	T swapper;
	swapper = a;
	a = b;
	b = swapper;
}

enum AALINE_MODE{MIX,ADDITIVE};
extern bool DrawAALine(Image& buffer, 
						const Vector2s& start_at_in,
						const Vector2s& end_at_in,
						const Vector2s& clipping_min,
						const Vector2s& clipping_max,
						unsigned short LineColor,
						AALINE_MODE mode = MIX);

bool DrawAALineGradient(Image& buffer, 
						const Vector2s& start_at_in,
						const Vector2s& end_at_in,
						const Vector2s& clipping_min,
						const Vector2s& clipping_max,
						unsigned short* GradientStart,
						int GradientNum,
						AALINE_MODE mode = MIX);

bool ClipLine(Vector2s& start,
			  Vector2s& end,
			  const Vector2s& pos_min,
			  const Vector2s& pos_max);

#endif