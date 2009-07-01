#ifndef _COLOR_H_
#define _COLOR_H_

#include "config.h"

namespace Lib3D
{

// ---------------------------------------------------------------------------
//	Colour Manipulation functions
//
//
//	Note: The colour Format is:
//		16 bits:		gd/a			4 bits, can be alpha (in the texture) or gouraud shading in the screen buffer
//								r,g,b			4 bits/chanel. The rgb triplet is located in the lower part of the 
//													value	for compatibility with the Simbian frame buffer
// ---------------------------------------------------------------------------
class CColor
{
public:
	typedef unsigned int Type;

	enum
	{
		kMaskA = 0xF000,
		kMaskR = 0x0F00,
		kMaskG = 0x00F0,
		kMaskB = 0x000F
	};

	enum
	{
		kMaxintensity = 0x0F
	};

	enum
	{
		kNbColourShade = 1<<16,
	};

	inline static Type	ClampIntensity(int i)			{if (i >= kMaxintensity) return kMaxintensity;else if(i <= 0)return 0; else return i;}
	inline static Type	MaxIntensity()					{return	kMaxintensity;}
	inline static void	SetIntensity(Type& col,int i)	{if(i>=0){if(i>=kMaxintensity) i=kMaxintensity; col = (col&0x0FFF) | (i<<12);}}
	inline static void	SetMaxIntensity(Type& col)		{col = (col&0x0FFF) | (unsigned short)(kMaxintensity<<12);}

	// Create a 16 bits colour from a 24bits RGB 
	inline static Type	GetColor(const unsigned char* data)	{return GetColor(data,0xFF);}
	inline static Type	GetColorAlpha(const unsigned char* data)	{return GetColor(data,data[3]);}

	inline static Type	GetColor(const unsigned char* data,unsigned char i)
	{
		return	((i      &0xF0) << 8) | 
						((data[0]&0xF0) << 4) | 
						((data[1]&0xF0)     ) |
						((data[2]&0xF0) >>4 );
	}

	inline static Type GetRawColour(const Type c)	{return c & 0x0FFF;}

	inline	static Type Mix(Type a,Type b,Type c,Type d)
	{
		// TODO: Alpha

		Type r1 = ((a & kMaskR) +(b & kMaskR) +(c & kMaskR) + (d & kMaskR)) >> 2;
		Type g1 = ((a & kMaskG) +(b & kMaskG) +(c & kMaskG) + (d & kMaskG)) >> 2;
		Type b1 = ((a & kMaskB) +(b & kMaskB) +(c & kMaskB) + (d & kMaskB)) >> 2;
		Type a1 = ((a & kMaskA)>>2) + ((b & kMaskA)>>2) +((c & kMaskA)>>2) + ((d & kMaskA)>>2);

		return (a1 & kMaskA)| (r1 & kMaskR) | (g1&kMaskG) | b1;
	}

	inline	static Type Mix(Type a,Type b)
	{
		// TODO: Alpha
		Type r1 = ((a & kMaskR) +(b & kMaskR)) >>1;
		Type g1 = ((a & kMaskG) +(b & kMaskG)) >>1;
		Type b1 = ((a & kMaskB) +(b & kMaskB)) >>1;
		Type a1 = ((a & kMaskA)>>1) + ((b & kMaskA)>>1);

			return (a1 & kMaskA)| (r1 & kMaskR) | (g1&kMaskG) | b1;
	}
	
	static inline void MaskAlpha(Type& c) {}
};

}//namepsace
#endif // _COLOR_H_
