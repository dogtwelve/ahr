#include "config.h"

#include "hg/HighGear.h"
#include "rasterize.h"

#include "constants.h"
#include "devutil.h"
#include "Board3d.h"
#include "Texture.h"
#include "IMath.h"
#include "lib2d/Lib2d.h"


#ifdef __565_RENDERING__
	#define __BREW__
#endif


#if defined(WIN32) //&& !defined(__BREW__)
	extern int gNbPixelWritten;
	#define PIXEL_WRITTEN gNbPixelWritten++
#else
	#define PIXEL_WRITTEN 
#endif

//const int kMask256	= 0xFF;
//const int kMask128	= 0x7F;
//const int kMask64	= 0x3F;
//const int kMask32	= 0x1F;
//const int kMask2	= 0x01;	// --- TEST
//
//const int kVShift256	= 8;
//const int kVShift128	= 7;
//const int kVShift64		= 6;
//const int kVShift32		= 5;
//const int kVShift2		= 1;	// --- TEST


//LAngelov: Removed, because we have to make Portrait/Landscape fast
/*#if WIN_DEBUG
	static int _t0=0;
	static int _t16=0;
	static int _t32=0;
	static int _t64=0;
	static int _t128=0;
	static int _t256=0;	
	static void _ChecktextureSize(const TTexture* t)
	{

		// this function tell me that only 64 and 128 textures are used here
		switch(t->SizeX())
		{
			case 16: _t16++;break;
			case 32: _t32++;break;
			case 64: _t64++;break;
			case 128: _t128++;break;
			case 256: _t256++;break;
			default: _t0++;break;
		}
		int _breakHere=0;
	}


	// changed to work with the VX9800 - 04-07-06 -cmm
	static int		_pop[m_dispX+1];		// _pop[177];
	static float	_pop2[m_dispX+1];	// _pop2[177];
	static float	_pop3[m_dispX+1];	// _pop3[177];
	static int		_nbr = 0;
	

	static void _CheckLineWidth(unsigned int w)
	{
		if(_nbr==0)
		{			
//			for(int i=0;i<(177);i++)
			for(int i=0;i<(m_dispX+1);i++)	// changed to work with the VX9800 - 04-07-06 -cmm
			{
				_pop[i]=0;
				_pop2[i]=0;
			}
		}

		_nbr++;

		// changed to work with the VX9800 - 04-07-06 -cmm
		if(w > m_dispX)	// 176)
		{
			w = m_dispX;	// 176;
		}

		_pop[w]++;
		_pop2[w] = float(_pop[w])/float(_nbr);

		float total = 0;
		for(unsigned int i=0;i<=w;i++)
		{
			total += _pop2[i];
			_pop3[i] = total;
		}

		int _breakHere=0;
	}

	#define CHECK_TEXTURE_SIZE _ChecktextureSize(board.m_bindTexture)
	#define CHECK_LINE_WIDTH(w) _CheckLineWidth(w);

#else*/ //~LAngelov: Removed, because we have to make Portrait/Landscape fast
	#define CHECK_TEXTURE_SIZE
	#define CHECK_LINE_WIDTH
//#endif





const int kTransparent = 0xFF0F;// Ugly pink

#define TRANSPARENT_ALPHA_BITS		0xF000


using namespace Lib3D;



#ifdef BILINEAR_FILTERING_TEST
unsigned short GetPixelUsingBilinerFilter(const unsigned short * data, int u, int v, int draw_mask, int vshift)
{
	if (CLib3D::bEnableBilinearFiltering)
	{
		unsigned int x = (u >> Lib3D::DIV_SHIFT) & draw_mask;
		unsigned int y = (v >> Lib3D::DIV_SHIFT) & draw_mask;

		if (x >= draw_mask)
		{
			x = draw_mask - 1;
		}
		if (y >= draw_mask)
		{
			y = draw_mask - 1;
		}

		unsigned int i = (u & Lib3D::DIV_MASK) >> (Lib3D::DIV_SHIFT - 8);
		unsigned int h = (v & Lib3D::DIV_MASK) >> (Lib3D::DIV_SHIFT - 8);

		const unsigned int c1 = data[((y + 0) << vshift) | (x + 0)];
		const unsigned int c2 = data[((y + 0) << vshift) | (x + 1)];
		const unsigned int c3 = data[((y + 1) << vshift) | (x + 0)];
		const unsigned int c4 = data[((y + 1) << vshift) | (x + 1)];

		const int d1 = (0x100 - i) * (0x100 - h);
		const int d2 = (0x000 + i) * (0x100 - h);
		const int d3 = (0x100 - i) * (0x000 + h);
		//const int d4 = (0x000 + i) * (0x000 + h);
		const int d4 = 65536 - d1 - d2 - d3;

		const unsigned int R = 0xF8000000 & (((c1 & 0x00F800) * d1) + ((c2 & 0x00F800) * d2) + ((c3 & 0x00F800) * d3) + ((c4 & 0x00F800) * d4));
		const unsigned int G = 0x07E00000 & (((c1 & 0x0007E0) * d1) + ((c2 & 0x0007E0) * d2) + ((c3 & 0x0007E0) * d3) + ((c4 & 0x0007E0) * d4));
		const unsigned int B = 0x001F0000 & (((c1 & 0x00001F) * d1) + ((c2 & 0x00001F) * d2) + ((c3 & 0x00001F) * d3) + ((c4 & 0x00001F) * d4));

		return ( (R|G|B) >> 16 );
	}
	else
	{
		return data[ (((v>>Lib3D::DIV_SHIFT)&draw_mask) << vshift) | ((u>>Lib3D::DIV_SHIFT)&draw_mask)];
	}
}
#endif

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
inline unsigned short*	LineEnd(int xStop,unsigned short* base, CHighGear *pHG)
{
	if (xStop > pHG->m_dispX)
		return base + pHG->m_dispX;
	else
		return base + xStop;
}



// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
inline void WritePixel(unsigned short color,unsigned short* offp)
{	
	//if(color != kTransparent)	// Test is the pixel is transparent (MAGENTA)
	// -> should be quicker in ASM :) 
#ifdef __BREW__
	if (color == 0)
	{
	}
	else
#else
	if (!(color & TRANSPARENT_ALPHA_BITS))	// Test is the pixel is transparent (no bits to 1 in the higher 4)
#endif
	{
		*offp = color;
		PIXEL_WRITTEN;
	}
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
inline void WritePixelNoKeycol(unsigned short color,unsigned short* offp)
{	
	//if(color != kTransparent)	// Test is the pixel is transparent (MAGENTA)
	// -> should be quicker in ASM :) 
	{
		*offp = color;
		PIXEL_WRITTEN;
	}
}

inline unsigned short FastMod( unsigned int val, int level )
{
	if ((int)val < level)
		return val;
	else
		return val - level;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Lib3D::DrawScanLineZCorrected(CBoard3D& board,int yDraw,const LineParamZ& i_param,int xStop)
{
	const int xStart = DownShift16(i_param.x);

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);
	const unsigned short* offpEnd = LineEnd(xStop ,offp, board.m_pHG);

	A_ASSERT(offpEnd>=offp && offpEnd<= (offp + board.m_dispX));

	const int	du = board.m_du;
	const int	dv = board.m_dv;
	int			dh = board.m_dh;

	int u = i_param.u;
	int v = i_param.v;
	int h = i_param.h;
	

	if (xStart < 0)
	{
		const int jump = -xStart;	// number of pixel to jump
		u += du * jump;
		v += dv * jump;
		h += dh * jump;
	}
	else
		offp += xStart;

	const int* const invTablePtr = board.m_invRhTable;

#ifdef __BREW__
	// no dither in brew
	unsigned short* textureData0 = board.m_bindTexture->Data();
#else
	unsigned short* textureData0;
	unsigned short* textureData1;

	if((yDraw & 0x1) ^ (((unsigned long)offp & 0x2)>>1))
	{
		textureData0 = board.m_bindTexture->Data();
		textureData1 = board.m_bindTexture->DataDither();		
	}
	else
	{
		textureData0 = board.m_bindTexture->DataDither();
		textureData1 = board.m_bindTexture->Data();
	}
#endif

	// REMOVE INVERT BUG  // TBD REALLY NECESSARY ???
	if ((h >> 12) < 0 || (h >> 12) >= CBoard3D::DX_RANGE+ 32)
	{
		h = 0;
		dh = 0;
	}
	CHECK_TEXTURE_SIZE;
	CHECK_LINE_WIDTH(offpEnd - offp);

	const int			kIntervalShift		= 3; // rax was 4;
	const int			kIntervalWidth		= 1 << kIntervalShift;

	const int			duInterval			= du << kIntervalShift;
	const int			dvInterval			= dv << kIntervalShift;
	const int			dhInterval			= dh << kIntervalShift;

	const int			lineWidth			= offpEnd - offp;
	const int			nbSegments			= lineWidth >> kIntervalShift;	// Linear segment 16 in length segments
	const unsigned short* lastSegmentPixel	= offp + (nbSegments << kIntervalShift);


	#define CORRECT_TEX_COORD(u,cor) ((((u) >> (TTexture::TEX_UV_SHIFT + 6)) * cor))


#ifdef BILINEAR_FILTERING_TEST
	
	#define WRITE_PIXEL_FROM_UV16(TEXTURE_DATA, DRAW_MASK, VSHIFT)	WritePixel(GetPixelUsingBilinerFilter(TEXTURE_DATA, u0, v0, DRAW_MASK, VSHIFT), offp);

#else

	#define WRITE_PIXEL_FROM_UV16(texture, maskX, maskY, vShift)				WritePixel(texture[ ((u0 >> 16) & maskX) | (((v0 >> 16) & maskY) << vShift)], offp );
	#define WRITE_PIXEL_FROM_UV16_NO_KEYCOL(texture, maskX, maskY, vShift)		WritePixelNoKeycol(texture[ ((u0 >> 16) & maskX) | (((v0 >> 16) & maskY) << vShift)], offp );
	#define GET_PIXEL_FROM_UV16(texture, maskX, maskY, vShift)					texture[ ((u0 >> 16) & maskX) | (((v0 >> 16) & maskY) << vShift)]
	//#define WRITE_PIXEL_FROM_UV16(texture, maskX, maskY, vShift)	\
	//{\
	//	int uu0 = u0 >> 16;\
	//	if (uu0 >= textureSizeX)\
	//		uu0 = textureSizeX - 1;\
	//	int vv0 = v0 >> 16;\
	//	if (vv0 >= textureSizeY)\
	//		vv0 = textureSizeY - 1;\
	//	WritePixel(texture[ (uu0 & maskX) | ((vv0 & maskY) << vShift)], offp );\
	//}

#endif

	const int maskX = board.m_bindTexture->DrawMaskX();
	const int maskY = board.m_bindTexture->DrawMaskY();
	const int vShift = board.m_bindTexture->VShift();

//-------------
// road fx (SLOOOOW)
//#define USE_ROAD_FX

#if USE_ROAD_FX == 1

	#define WRITE_ROADPIXEL_FROM_UV16(texture, maskX, maskY, shift)		{\
						randSeed = 1664525 * randSeed + 1013904223;	\
						unsigned short fx = FastMod( (randSeed >> 16) & levelMask, roadFxLevel );	\
						*offp = CLib2D::FastColorAdd( texture[ ((u0 >> 16) & maskX) | (((v0 >> 16) & maskY) << shift)], (fx << (5+6)) | (fx << 6) | fx );	}

	//#define WRITE_ROADPIXEL_FROM_UV16_T256(texture)	WRITE_ROADPIXEL_FROM_UV16(texture,kMask256,kVShift256)
	//#define WRITE_ROADPIXEL_FROM_UV16_T128(texture)	WRITE_ROADPIXEL_FROM_UV16(texture,kMask128,kVShift128)
	//#define WRITE_ROADPIXEL_FROM_UV16_T64(texture)	WRITE_ROADPIXEL_FROM_UV16(texture,kMask64,kVShift64)
	//#define WRITE_ROADPIXEL_FROM_UV16_T32(texture)	WRITE_ROADPIXEL_FROM_UV16(texture,kMask32,kVShift32)

	#define ROAD_FX_MAX				(board.m_roadFxMax) //4	//15
	#define ROAD_FX_HEIGHT			(IS_PORTRAIT_REF(board) ? 150 : 120)
	#define ROAD_FX_END_Y			board.m_dispY

	//int	roadFxLevel = (ROAD_FX_HEIGHT - (ROAD_FX_END_Y - yDraw)) / (ROAD_FX_HEIGHT / ROAD_FX_MAX);
	int	roadFxLevel = (yDraw - ROAD_FX_HEIGHT) / 8;
	if (roadFxLevel < 1)
		roadFxLevel = 0;
	if (roadFxLevel > ROAD_FX_MAX)
		roadFxLevel = ROAD_FX_MAX;

	static const int	LevelMask[16] = { 0x0, 0x1, 0x3, 0x3, 0x7, 0x7, 0x7, 0x7, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf };

	int levelMask = LevelMask[roadFxLevel];

	unsigned int	randSeed = board.m_RoadFXRandSeed;
	//Jogy
	//Reinitialize the random seed every frame to stop the moving of noise when 
	// the picture is not moving

	if (roadFxLevel && (board.m_TextureFXIdx == CBoard3D::kTextureFxModeRoad))
	{
		while(offp < lastSegmentPixel)
		{
			int correction = invTablePtr[h>>12];

			int u0 = CORRECT_TEX_COORD(u,correction);
			int v0 = CORRECT_TEX_COORD(v,correction);

			u += duInterval;
			v += dvInterval;
			h += dhInterval;

			correction = invTablePtr[h>>12];

			int u1 = CORRECT_TEX_COORD(u,correction);
			int v1 = CORRECT_TEX_COORD(v,correction);

			// u0,u1 idu,ivd etc are upshifted by 16
			const int idu = (u1-u0) >> kIntervalShift;
			const int idv = (v1-v0) >> kIntervalShift;

			const unsigned short* intervalEnd = offp + kIntervalWidth;

			#define NEXT_PIXEL	u0 += idu; v0 += idv; ++offp;

			A_ASSERT(offp != intervalEnd);
			do
			{
				WRITE_ROADPIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
				NEXT_PIXEL
			}while(offp != intervalEnd);			

			#undef NEXT_PIXEL
		}

		// last < 16 pixels...
		{
			const int intervalWidth = offpEnd - offp;

			if(intervalWidth==0)
			{
				// save new value
				board.m_RoadFXRandSeed = randSeed;
				return;
			}
			else if(intervalWidth==1)
			{
				// Only one pixel, happend often enough to justify skipping all the interpolation stuff
				int correction = invTablePtr[h>>12];
				
				int u0 = CORRECT_TEX_COORD(u,correction);
				int v0 = CORRECT_TEX_COORD(v,correction);

				WRITE_ROADPIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)				
			}
			else
			{
				const int correction0 = invTablePtr[h>>12];
				const int correction1 = invTablePtr[(h + (dh * intervalWidth))>>12];
				
				int u0 = CORRECT_TEX_COORD(u,correction0);
				int u1 = CORRECT_TEX_COORD(u + (du * intervalWidth),correction1);
				const int div = board.m_256Div16[intervalWidth];
				const int idu = ((u1-u0) * div) >> 8;


				int v0 = CORRECT_TEX_COORD(v,correction0);
				int v1 = CORRECT_TEX_COORD(v+ (dv * intervalWidth),correction1);
				const int idv = ((v1-v0) * div) >> 8;

				// u0,u1 idu,ivd etc are upshifted by 16

				#define NEXT_PIXEL	++offp;if(offp >= offpEnd){board.m_RoadFXRandSeed = randSeed;return;}	u0+=idu;v0+=idv;

				while(1)
				{
					WRITE_ROADPIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
					NEXT_PIXEL
				}

				#undef NEXT_PIXEL
			}
		}
	}
	else
	{
#endif
//-------------

#if USE_Z_BUFFER
	int* zBuffer = board.m_zBuffer + (offp - board.m_screen);
	//const int zBufferDir = board.m_zBufferDir;
#endif

	while (offp < lastSegmentPixel)
	{
		int correction = invTablePtr[h >> 12];

		int u0 = CORRECT_TEX_COORD(u,correction);
		int v0 = CORRECT_TEX_COORD(v,correction);

		u += duInterval;
		v += dvInterval;
		h += dhInterval;

		correction = invTablePtr[h>>12];

		int u1 = CORRECT_TEX_COORD(u,correction);
		int v1 = CORRECT_TEX_COORD(v,correction);

		// u0,u1 idu,ivd etc are upshifted by 16
		const int idu = (u1-u0) >> kIntervalShift;
		const int idv = (v1-v0) >> kIntervalShift;

		const unsigned short* intervalEnd = offp + kIntervalWidth;

		#define NEXT_PIXEL	u0 += idu; v0 +=idv; ++offp;

		//const int hz = h * zBufferDir;

		A_ASSERT(offp != intervalEnd);
		do
		{
#if USE_Z_BUFFER
			//if ((zBufferDir > 0 && *zBuffer <= hz) || (zBufferDir < 0 && *zBuffer >= hz))
			if (*zBuffer <= h && GET_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift) != 0)
			{
				*zBuffer = h;
				WRITE_PIXEL_FROM_UV16_NO_KEYCOL(textureData0, maskX, maskY, vShift)
			}
#else
			WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
#endif // USE_Z_BUFFER

#if USE_Z_BUFFER
			++zBuffer;
#endif
			NEXT_PIXEL

#ifndef __BREW__
			WRITE_PIXEL_FROM_UV16(textureData1, maskX, maskY, vShift)
			NEXT_PIXEL
#endif
		}while(offp != intervalEnd);
	
		#undef NEXT_PIXEL
	}

	// last < 16 pixels...
	{
		const int intervalWidth = offpEnd - offp;

		if (intervalWidth==0)
		{
			return;
		}
		
		if (intervalWidth == 1)
		{
			// Only one pixel, happend often enough to justify skipping all the interpolation stuff
			int correction = invTablePtr[h>>12];
			
			int u0 = CORRECT_TEX_COORD(u,correction);
			int v0 = CORRECT_TEX_COORD(v,correction);

#if USE_Z_BUFFER
			//if ((zBufferDir > 0 && *zBuffer <= h) || (zBufferDir < 0 && *zBuffer >= -h))
			if (*zBuffer <= h && GET_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift) != 0)
			{
				*zBuffer = h;
				WRITE_PIXEL_FROM_UV16_NO_KEYCOL(textureData0, maskX, maskY, vShift)
			}
#else
			WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
#endif
		}
		else
		{
			// win debug!!!! - check it
			if (((h + (dh * intervalWidth))>>12) < 0 )
				return;
			const int correction0 = invTablePtr[h>>12];
			const int correction1 = invTablePtr[(h + (dh * intervalWidth))>>12];
			
			int u0 = CORRECT_TEX_COORD(u,correction0);
			int u1 = CORRECT_TEX_COORD(u + (du * intervalWidth),correction1);
			const int div = board.m_256Div16[intervalWidth];
			const int idu = ((u1-u0) * div) >> 8;

			int v0 = CORRECT_TEX_COORD(v,correction0);
			int v1 = CORRECT_TEX_COORD(v+ (dv * intervalWidth),correction1);
			const int idv = ((v1-v0) * div) >> 8;

			// u0,u1 idu,ivd etc are upshifted by 16

			#define NEXT_PIXEL	++offp; if (offp >= offpEnd) return; u0 += idu; v0 += idv;

			//const int hz = h * zBufferDir;

			while(1)
			{				
#if USE_Z_BUFFER
				//if ((zBufferDir > 0 && *zBuffer <= h) || (zBufferDir < 0 && *zBuffer >= hz))
				if (*zBuffer <= h && GET_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift) != 0)
				{
					*zBuffer = h;
					WRITE_PIXEL_FROM_UV16_NO_KEYCOL(textureData0, maskX, maskY, vShift)
				}
#else
				WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift);
#endif

#if USE_Z_BUFFER
				++zBuffer;
#endif
				NEXT_PIXEL

#ifndef __BREW__
				WRITE_PIXEL_FROM_UV16(textureData1, maskX, maskY, vShift);
				NEXT_PIXEL
#endif
			}

			#undef NEXT_PIXEL
		}
	}


#if USE_ROAD_FX == 1
	}
	// save new value
	board.m_RoadFXRandSeed = randSeed;
#endif
}


//////////////////////////////////////////////////////////////////////////////

void Lib3D::DrawScanLineZCorrectedCar(CBoard3D& board,int yDraw,const LineParamZ& i_param, int xStop)
{
//	const int xStart = DownShift16(i_param.x);
//
//	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
//	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);
//	const unsigned short* offpEnd = LineEnd(xStop ,offp, board.m_pHG);
//
//	A_ASSERT(offpEnd>=offp && offpEnd<= (offp + board.m_dispX));
//
//	const int	du = board.m_du;
//	const int	dv = board.m_dv;
//	int			dh = board.m_dh;
//
//	int u = i_param.u;
//	int v = i_param.v;
//	int h = i_param.h;
//	
//
//	if (xStart < 0)
//	{
//		const int jump = -xStart;	// number of pixel to jump
//		u += du * jump;
//		v += dv * jump;
//		h += dh * jump;
//	}
//	else
//		offp += xStart;
//
//	const int* const invTablePtr = board.m_invRhTable;
//
//	unsigned short* textureData0 = board.m_bindTexture->Data();
//
//	// REMOVE INVERT BUG  // TBD REALLY NECESSARY ???
//	if ((h>>12) <0 || (h>>12) >= CBoard3D::DX_RANGE+ 32)
//	{
//		h = 0;
//		dh = 0;
//	}
//	CHECK_TEXTURE_SIZE;
//	CHECK_LINE_WIDTH(offpEnd - offp);
//
//	const int			kIntervalShift		= 2; // rax was 4;
//	const int			kIntervalWidth		= 1 << kIntervalShift;
//
//	const int			duInterval			= du << kIntervalShift;
//	const int			dvInterval			= dv << kIntervalShift;
//	const int			dhInterval			= dh << kIntervalShift;
//
//	const int			lineWidth			= offpEnd - offp;
//	const int			nbSegments			= lineWidth >> kIntervalShift;	// Linear segment 16 in length segments
//	const unsigned short* lastSegmentPixel	= offp + (nbSegments << kIntervalShift);
//
//
//	#define CORRECT_TEX_COORD(u,cor) ((((u) >> (TTexture::TEX_UV_SHIFT + 6)) * cor))
//
//
//	#define WRITE_PIXEL_FROM_UV16(texture, maskX, maskY, vShift)				WritePixel(texture[ ((u0 >> 16) & maskX) | (((v0 >> 16) & maskY) << vShift)], offp );
//	//#define WRITE_PIXEL_FROM_UV16(texture, maskX, maskY, vShift)	\
//	//{\
//	//	int uu0 = u0 >> 16;\
//	//	if (uu0 >= textureSizeX)\
//	//		uu0 = textureSizeX - 1;\
//	//	int vv0 = v0 >> 16;\
//	//	if (vv0 >= textureSizeY)\
//	//		vv0 = textureSizeY - 1;\
//	//	WritePixel(texture[ (uu0 & maskX) | ((vv0 & maskY) << vShift)], offp );\
//	//}
//
//	const int maskX = board.m_bindTexture->DrawMaskX();
//	const int maskY = board.m_bindTexture->DrawMaskY();
//	const int vShift = board.m_bindTexture->VShift();
//
//	unsigned short* textureDataScreen = (unsigned short*)board.m_bindFxEnvmapScreen;
//	unsigned short* textureDataCoord = (unsigned short*)board.m_bindFxEnvmapCoord;
//
//	if (board.m_dispX < board.m_dispY)
//	{
//		//LAngelov: Move reflection map slightly down in case of portrait mode
//		// in big resolutions too much of the sky gets into the original map
//		//textureDataScreen += (30 * board.m_dispX);
//		//rax - change in resource, for now, lower here:
//		textureDataScreen += (40 * board.m_dispX);
//	}
//
////-------------
//
//#if USE_Z_BUFFER
//	int* zBuffer = board.m_zBufferCar + (offp - board.m_screen);
//#endif
//
//	while (offp < lastSegmentPixel)
//	{
//		int correction = invTablePtr[h >> 12];
//
//		int u0 = CORRECT_TEX_COORD(u,correction);
//		int v0 = CORRECT_TEX_COORD(v,correction);
//
//		u += duInterval;
//		v += dvInterval;
//		h += dhInterval;
//
//		correction = invTablePtr[h>>12];
//
//		int u1 = CORRECT_TEX_COORD(u,correction);
//		int v1 = CORRECT_TEX_COORD(v,correction);
//
//		// u0,u1 idu,ivd etc are upshifted by 16
//		const int idu = (u1-u0) >> kIntervalShift;
//		const int idv = (v1-v0) >> kIntervalShift;
//
//		const unsigned short* intervalEnd = offp + kIntervalWidth;
//
//		#define NEXT_PIXEL	u0 += idu; v0 +=idv; ++offp;
//
//		A_ASSERT(offp != intervalEnd);
//		do
//		{
//#if USE_Z_BUFFER
//			//if (*zBuffer > h)
//			//	return;
//			if (*zBuffer <= h)
//			{
//				*zBuffer = h;
//
//				const int textureIdx = (((u0 >> (Lib3D::DIV_SHIFT)) & maskX) | (((v0 >> (Lib3D::DIV_SHIFT)) & maskY) << vShift));
//				const unsigned int textureColour = textureData0[textureIdx];
//				if (textureColour)
//				{
//					A_ASSERT(textureDataScreen + textureIdx < (unsigned short*)board.m_bindFxEnvmapScreen + board.m_dispX * board.m_dispY);
//					const int envmapCoordIdx = textureDataCoord[textureIdx];
//
//					if (envmapCoordIdx)
//					{
//						unsigned int	screenColor = (textureDataScreen[envmapCoordIdx] & (~0x821)) >> 1;
//						screenColor = (screenColor & (~0x821)) >> 1;
//
//						*offp = CLib2D::ApproximateColorAdd(textureColour, screenColor);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
//						//*offp = CLib2D::FastColorMix75_25(textureColour, screenColor);
//
////LAngelov: Check where we get reflection coord from
//					}
//					else
//					{
//						*offp = textureColour;
//					}
//				}
//
//				//WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)				
//			}
//#else
//			// TODO //WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
//#endif // USE_Z_BUFFER
//
//#if USE_Z_BUFFER
//			++zBuffer;
//#endif
//			NEXT_PIXEL
//
////#ifndef __BREW__
////			WRITE_PIXEL_FROM_UV16(textureData1, maskX, maskY, vShift)
////			NEXT_PIXEL
////#endif
//		}while(offp != intervalEnd);
//	
//		#undef NEXT_PIXEL
//	}
//
//	// last < 16 pixels...
//	{
//		const int intervalWidth = offpEnd - offp;
//
//		if (intervalWidth==0)
//		{
//			return;
//		}
//		
//		if (intervalWidth == 1)
//		{
//			// Only one pixel, happend often enough to justify skipping all the interpolation stuff
//			int correction = invTablePtr[h>>12];
//			
//			int u0 = CORRECT_TEX_COORD(u,correction);
//			int v0 = CORRECT_TEX_COORD(v,correction);
//
//#if USE_Z_BUFFER
//			if (*zBuffer <= h)
//			{
//				*zBuffer = h;
//								
//				const int textureIdx = (((u0 >> (Lib3D::DIV_SHIFT)) & maskX) | (((v0 >> (Lib3D::DIV_SHIFT)) & maskY) << vShift));
//				const unsigned int textureColour = textureData0[textureIdx];
//				if (textureColour)
//				{
//					A_ASSERT(textureDataScreen + textureIdx < (unsigned short*)board.m_bindFxEnvmapScreen + board.m_dispX * board.m_dispY);
//					const int envmapCoordIdx = textureDataCoord[textureIdx];
//
//					if (envmapCoordIdx)
//					{
//						unsigned int	screenColor = (textureDataScreen[envmapCoordIdx] & (~0x821)) >> 1;
//						screenColor = (screenColor & (~0x821)) >> 1;
//
//						*offp = CLib2D::ApproximateColorAdd(textureColour, screenColor);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
//						//*offp = CLib2D::FastColorMix75_25(textureColour, screenColor);
//
////LAngelov: Check where we get reflection coord from
//					}
//					else
//					{
//						*offp = textureColour;
//					}
//				}
//
//				//WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
//			}
//#else
//			WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
//#endif
//		}
//		else
//		{
//			// win debug!!!! - check it
//			if (((h + (dh * intervalWidth))>>12) < 0 )
//				return;
//			const int correction0 = invTablePtr[h>>12];
//			const int correction1 = invTablePtr[(h + (dh * intervalWidth))>>12];
//			
//			int u0 = CORRECT_TEX_COORD(u,correction0);
//			int u1 = CORRECT_TEX_COORD(u + (du * intervalWidth),correction1);
//			const int div = board.m_256Div16[intervalWidth];
//			const int idu = ((u1-u0) * div) >> 8;
//
//			int v0 = CORRECT_TEX_COORD(v,correction0);
//			int v1 = CORRECT_TEX_COORD(v+ (dv * intervalWidth),correction1);
//			const int idv = ((v1-v0) * div) >> 8;
//
//			// u0,u1 idu,ivd etc are upshifted by 16
//
//			#define NEXT_PIXEL	++offp; if (offp >= offpEnd) return; u0 += idu; v0 += idv;
//
//			while(1)
//			{				
//#if USE_Z_BUFFER
//				if (*zBuffer <= h)
//				{
//					*zBuffer = h;
//					
//				const int textureIdx = (((u0 >> (Lib3D::DIV_SHIFT)) & maskX) | (((v0 >> (Lib3D::DIV_SHIFT)) & maskY) << vShift));
//				const unsigned int textureColour = textureData0[textureIdx];
//				if (textureColour)
//				{
//					A_ASSERT(textureDataScreen + textureIdx < (unsigned short*)board.m_bindFxEnvmapScreen + board.m_dispX * board.m_dispY);
//					const int envmapCoordIdx = textureDataCoord[textureIdx];
//
//					if (envmapCoordIdx)
//					{
//						unsigned int	screenColor = (textureDataScreen[envmapCoordIdx] & (~0x821)) >> 1;
//						screenColor = (screenColor & (~0x821)) >> 1;
//
//						*offp = CLib2D::ApproximateColorAdd(textureColour, screenColor);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
//						//*offp = CLib2D::FastColorMix75_25(textureColour, screenColor);
//
////LAngelov: Check where we get reflection coord from
//					}
//					else
//					{
//						*offp = textureColour;
//					}
//				}
//					//WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift)
//				}
//#else
//				WRITE_PIXEL_FROM_UV16(textureData0, maskX, maskY, vShift);
//#endif
//
//#if USE_Z_BUFFER
//				++zBuffer;
//#endif
//				NEXT_PIXEL
//
//#ifndef __BREW__
//				WRITE_PIXEL_FROM_UV16(textureData1, maskX, maskY, vShift);
//				NEXT_PIXEL
//#endif
//			}
//
//			#undef NEXT_PIXEL
//		}
//	}
}

// ---------------------------------------------------------------------------
//	board:		The structure containing the context information and the screen buffer
//	yDraw			Current line number (y) in the screen buffer
//	x,				Interpolated x value: current column (x) shifted left 16
//	z,				Interpolated z value: Depth of the first pixel, shifted left 16
//	u,				U and V are texture coordinate, both are interpolated from the polygon vertex, both are shifted left 16
//	v,
//	xStop,		One past the last column (x) to draw (not shifted)

//	Since the texture buffer are allway square and heve a size that is a power of 2
//	the drawmask map the texture coordinate in the available texturespace, vShift 
//	actualy multiply the v by the size of the texture

// rax - now we have support for rectangle textures (power of 2); TODO: change comment

// ---------------------------------------------------------------------------


void Lib3D::DrawScanLine(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	using namespace Lib3D;

	const int xStart = i_param.x >> 16;	// The first pixel to draw, 

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer
	const unsigned short* offpEnd	= LineEnd(xStop,offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen

	A_ASSERT(offpEnd >= offp && offpEnd <= (offp + board.m_dispX));

	const int du = board.m_du;			// du,dv and dz are the amount of change (delta) to interpolate the values from pixel to pixel
	const int dv = board.m_dv;		
	
	int u = i_param.u;
	int v = i_param.v;	

	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
	{
		const int jump  =  0-xStart; // number of pixel to jump

		u += du * jump;
		v += dv * jump;
	}
	else
	{
		 offp +=  xStart;
	}

	const TTexture *texture = board.m_bindTexture;
	const unsigned short* textureData	= texture->Data();

	const int maskX = board.m_bindTexture->DrawMaskX();
	const int maskY = board.m_bindTexture->DrawMaskY();
	const int vShift = board.m_bindTexture->VShift();

	#define CALC_IDX()					(((u >> (Lib3D::DIV_SHIFT)) & maskX) | (((v >> (Lib3D::DIV_SHIFT)) & maskY) << vShift))
	//#define CALC_IDX(idx)					\
	//{\
	//	int uu = u >> (Lib3D::DIV_SHIFT);\
	//	int vv = v >> (Lib3D::DIV_SHIFT);\
	//	if (uu >= board.m_bindTexture->SizeX())\
	//		uu = board.m_bindTexture->SizeX() - 1;\
	//	if (vv >= board.m_bindTexture->SizeY())\
	//		vv = board.m_bindTexture->SizeY() - 1;\
	//	idx = (uu & maskX) | ((vv & maskY) << vShift);\
	//}
	
	#define NEXT_PIXEL					++offp;if(offp >= offpEnd)return;u += du;v += dv;

	//if (board.m_bindTexture->SizeX() < board.m_bindTexture->SizeY())
	//	int a = 0; // debug

	switch(board.m_TextureFXIdx)
	{
	case CBoard3D::kTextureFxModeEnvMap:
		{			
			unsigned short* textureDataScreen = (unsigned short*)board.m_bindFxEnvmapScreen;
			unsigned short* textureDataCoord = (unsigned short*)board.m_bindFxEnvmapCoord;

			if (board.m_dispX <= board.m_dispY)
			{
				//LAngelov: Move reflection map slightly down in case of portrait mode
				// in big resolutions too much of the sky gets into the original map
				//textureDataScreen += (30 * board.m_dispX);
				//rax - change in resource, for now, lower here:
				textureDataScreen += (40 * board.m_dispX);
			}

			while (1)
			{
				//Jogy, big models
				const int textureIdx = CALC_IDX();
				//int textureIdx;
				//CALC_IDX(textureIdx);

				const unsigned int textureColour = textureData[textureIdx];
#ifdef __BREW__
				if (textureColour)
#else
				if (!(textureColour & TRANSPARENT_ALPHA_BITS))
#endif
				{
					//const int idx2 = (idx + envmapuv) & 0xFFFF; // BECAUSE 256*256 TEXTURE TBD TBD
#ifndef R176x208
					A_ASSERT(textureDataScreen + textureIdx < (unsigned short*)board.m_bindFxEnvmapScreen + board.m_dispX * board.m_dispY);
#endif
					const int envmapCoordIdx = textureDataCoord[textureIdx];


					//*offp = CLib2D::FastColorAdd(colour, textureData2[idx2]);
#ifdef __BREW__
					if (envmapCoordIdx)
					{
						unsigned int	screenColor = (textureDataScreen[envmapCoordIdx] & (~0x821)) >> 1;
						screenColor = (screenColor & (~0x821)) >> 1;
						//unsigned int	screenColor = textureDataScreen[envmapCoordIdx];

						*offp = CLib2D::ApproximateColorAdd(textureColour, screenColor);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
						//*offp = CLib2D::FastColorMix75_25(textureColour, screenColor);

						//if (textureIdx >= 256*128)
						//	*offp = textureColour;
						//else 
						//*offp = textureDataScreen[envmapCoordIdx];

//LAngelov: Check where we get reflection coord from
					}
					else
					{
						*offp = textureColour;
					}

					//*offp = textureColour;
#else
					*offp = CLib2D::ApproximateColorAdd(colour, textureDataScreen[envmapCoordIdx]);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
#endif
				}

				NEXT_PIXEL
				PIXEL_WRITTEN;
			}
		}
		break;

	case CBoard3D::kTextureFxModeOldEnvMap:
		{
			//break;
			//Jogy, big models
//			A_ASSERT(board.m_bindTexture->SizeX() == 256 && board.m_bindTexture->SizeY() == 256);
//			A_ASSERT(board.m_bindTexture->SizeX() == 128 && board.m_bindTexture->SizeY() == 128);
			// EnvMap rendering
			const TTexture*		FxTexture = board.m_bindTextureFX;
			const unsigned short* textureData2 = FxTexture->Data();
			const int envmapuv = (board.m_TextureFXParam2 << FxTexture->VShift()) + (board.m_TextureFXParam1 & FxTexture->DrawMaskX()); 
		

			while (1)
			{
			//Jogy, big models
				const int idx = CALC_IDX();
				//int idx;
				//CALC_IDX(idx);

				//if (idx < 0 || idx > board.m_bindTexture->SizeX() * board.m_bindTexture->SizeY() )
				//{
				//	NEXT_PIXEL
				//	continue;
				//}

				const unsigned int colour = textureData[idx];
#ifdef __BREW__
				if (colour)
#else
				if (!(colour & TRANSPARENT_ALPHA_BITS))
#endif
				{
					const int idx2 = (idx + envmapuv) & 0x3FFF; // BECAUSE 128*128 TEXTURE TBD TBD


#ifdef __BREW__
//					*offp = CLib2D::CrappyColorAdd(colour, textureData2[idx2]);	// quicker, and gives enough quality since envmap texture is very dark
					*offp = CLib2D::ApproximateColorAdd(colour, textureData2[idx2]);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
#else
					*offp = CLib2D::ApproximateColorAdd(colour, textureData2[idx2]);	// still pretty quick and error is tolerable (4.5% instead of 46.7%)
#endif
				}

				NEXT_PIXEL
			}
		}
		break;

	case CBoard3D::kTextureFxModeGhost:
		{
			// ghost car rendering: The env-map is replaced by a solid colour
			const int fxColour = board.m_GhostColour;
			while (1)
			{
				const int idx = CALC_IDX();
				//int idx;
				//CALC_IDX(idx);
				const int colour = textureData[idx];
#ifdef __BREW__
				if (colour)
#else
				if (!(colour & TRANSPARENT_ALPHA_BITS))
#endif
					*offp = CLib2D::ApproximateColorAdd(colour, fxColour);

				NEXT_PIXEL
//~rax
			}
		}
		break;

	case CBoard3D::kTextureFxModeNormal:	// Normal rendering
	default:
		{
			while (1)
			{
				const int idx = CALC_IDX();
				//int idx;
				//CALC_IDX(idx);
				const unsigned int colour = textureData[idx];
#ifdef __BREW__
				if (colour)
#else
				if (!(colour & TRANSPARENT_ALPHA_BITS))
#endif
					*offp = colour; //0x1F; // blue

				NEXT_PIXEL
			}
		}
		break;

		break;
	}

	#undef  CALC_IDX
	#undef  NEXT_PIXEL
}

void Lib3D::DrawScanLineFlat(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	using namespace Lib3D;

	const int colour = (unsigned int)board.m_bindTexture;
#ifdef __BREW__
	if (!colour)
#else
	if (colour & TRANSPARENT_ALPHA_BITS)
#endif
		return;

	const int xStart = i_param.x>>16;	// The first pixel to draw, 

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);

	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	const unsigned short* const offpEnd	= LineEnd( xStop, offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen

	A_ASSERT(offpEnd >= offp && offpEnd <= (offp + board.m_dispX));

	if (xStart > 0)
		offp += xStart;

	while(offp < offpEnd)
	{
		*offp = colour;
		++offp;		
	}
}


void Lib3D::DrawScanLineFlatTrans(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	using namespace Lib3D;

	typedef struct
	{
		int				col;
		unsigned short	*offp, *offpEnd;
		int				dither;
	} PARAMASMFLATTRANS;

	PARAMASMFLATTRANS	param;		// must be first

	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	param.offp			= board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	param.offpEnd	= LineEnd(xStop,param.offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen

	A_ASSERT(param.offpEnd>=param.offp && param.offpEnd <= (param.offp + board.m_dispX));


	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
;//		const int jump  =  0-xStart;	// number of pixel to jump
	else
		 param.offp +=  xStart;	
//Jogy//
	param.col = (unsigned short)(((unsigned int)board.m_bindTexture) & 0xFFFF);

#if 1	// PLAYER_CAR_NICE_SHADOW
	// draw first pixel, if odd
	if (int(param.offp) & 0x2)
	{
		*param.offp = CLib2D::ApproximateColorMix50_50(*param.offp, param.col);
		param.offp ++;
	}
	// draw last pixel, if odd
	if (int(param.offpEnd) & 0x2)
	{
		-- param.offpEnd;
		*param.offpEnd = CLib2D::ApproximateColorMix50_50(*param.offpEnd, param.col);
	}
	// draw middle pixels 2 by 2
	unsigned long const col2Pixel = param.col | (param.col << 16);
	unsigned long * offp_32 = (unsigned long *)param.offp;
	unsigned long * const offpEnd_32 = (unsigned long *)param.offpEnd;
	// she don't get much faster than this:
	while (offp_32 != offpEnd_32)
	{
		*(offp_32 ++) = CLib2D::ApproximateColorMix2Pixel50_50(*offp_32, col2Pixel);
	}
#else
	if ((yDraw & 0x1) ^ (((unsigned long)param.offp & 0x2)>>1))
		param.dither = 1;
	else
		param.dither = 0;

#ifdef USE_ARM_ASM
	{
		// r0 col /  r1 offp / r2 offpend / r3 dither

		__asm	(	"ldr	r0, [sp, #0]" );
		__asm	(	"ldr	r1, [sp, #4]" );
		__asm	(	"ldr	r2, [sp, #8]" );
		__asm	(	"ldr	r3, [sp, #12]" );

		__asm	(	"loopflattrans:"	);

		__asm	(	"cmp	r3, #0" );					// if (dither == 0)
		__asm	(	"moveq	r3, #1" );					// dither = !dither // TBD optimize this part
		__asm	(	"movne	r3, #0" );

		__asm	(	"strneh	r0, [r1, #0]"	);			// *offp = col

		__asm	(	"add	r1, r1, #2"	);				// offp++
		__asm	(	"cmp	r1, r2"	);					// cmp offp / offpEnd
		__asm	(	"blt loopflattrans"	);				// goto loop
	}
#else
/*	while (1)
	{
		if (param.dither)
			WritePixel(param.col,param.offp);
		++param.offp;	// next pixel in the screen buffer

		if(param.offp >= param.offpEnd)
			return;

		param.dither = !param.dither;
	}
*/


	//first align the write pointer to 32 bits

//	if(param.offp == param.offpEnd)
//		return;

	if(int(param.offp) & 0x2)
	{
		//let's do this pixel the slow way
		if(param.dither)
		{
			WritePixel(0,param.offp);
		}
		param.dither = !param.dither;
		param.offp++;
	}
	//now compute the number of 32 bits block we can write
    int scan_length = int(param.offpEnd) - int(param.offp);
	const int last_pixel = scan_length & 2;
	scan_length >>=2;
	unsigned long * offp_32 = (unsigned long *)param.offp;
	if(scan_length)
	{
		if(param.dither)
		{
			do
			{
				*offp_32++ &= 0xFFFF0000;
			}
			while(--scan_length);
		}
		else
		{
			do
			{
				*offp_32++ &= 0x0000FFFF;
			}
			while(--scan_length);
		}
	}
	if(last_pixel && param.dither)
	{
		*offp_32++ &= 0xFFFF0000;
	}
#endif
#endif
}


void Lib3D::DrawScanLineFlatTransNeon(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	using namespace Lib3D;

	typedef struct
	{
		int				col;
		unsigned short	*offp, *offpEnd;
		int				dither;
	} PARAMASMFLATTRANS;

	PARAMASMFLATTRANS	param;		// must be first

	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	param.offp			= board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	param.offpEnd	= LineEnd(xStop,param.offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen

	A_ASSERT(param.offpEnd>=param.offp && param.offpEnd <= (param.offp + board.m_dispX));


	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
;//		const int jump  =  0-xStart;	// number of pixel to jump
	else
		 param.offp +=  xStart;	
//Jogy//
	param.col = (unsigned short)((unsigned int)board.m_bindTexture);

	if ((yDraw & 0x1) ^ (((unsigned long)param.offp & 0x2)>>1))
		param.dither = 1;
	else
		param.dither = 0;


	if(int(param.offp) & 0x2)
	{
		//let's do this pixel the slow way
		if(param.dither)
		{
			WritePixel(param.col,param.offp);
		}
		param.dither = !param.dither;
		param.offp++;
	}
	//now compute the number of 32 bits block we can write
    int scan_length = int(param.offpEnd) - int(param.offp);
	const int last_pixel = scan_length & 2;
	scan_length >>=2;
	unsigned long * offp_32 = (unsigned long *)param.offp;
	if(scan_length)
	{
		if(param.dither)
		{
			do
			{
				*offp_32 &= 0xFFFF0000;
				*offp_32++ |= param.col;
			}
			while(--scan_length);
		}
		else
		{
			do
			{
				*offp_32 &= 0x0000FFFF;
				*offp_32++ |= param.col<<16;
			}
			while(--scan_length);
		}
	}
	if(last_pixel && param.dither)
	{
		*offp_32 &= 0xFFFF0000;
		*offp_32 |= param.col;
	}
}



// ---------------------------------------------------------------------------
//	board:		The structure containing the context information and the screen buffer
//	yDraw			Current line number (y) in the screen buffer
//	x,				Interpolated x value: current column (x) shifted left 16
//	z,				Interpolated z value: Depth of the first pixel, shifted left 16
//	u,				U and V are texture coordinate, both are interpolated from the polygon vertex, both are shifted left 16
//	v,
//	xStop,		One past the last column (x) to draw (not shifted)
// ---------------------------------------------------------------------------


void Lib3D::DrawScanLineAdditive(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	//using namespace Lib3D;
	//typedef struct
	//{
	//	int				u, v, du, dv, drawMask, vShift;
	//	unsigned short*	textureData, *offp, *offpEnd;
	//	unsigned short* textureData2;
	//	int				envmapuv;
	//} PARAMASM;

	//PARAMASM	param;		// must be first

	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	const unsigned short* const offpEnd	= LineEnd(xStop,offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen
	A_ASSERT(offpEnd>=offp && offpEnd <= (offp + board.m_dispX));

	const int du = board.m_du;			// du,dv and dz are the amount of change (delta) to interpolate the values from pixel to pixel
	const int dv = board.m_dv;		

	
	int u = i_param.u;
	int v = i_param.v;
	
	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
	{
		const int jump  =  0-xStart;	// number of pixel to jump

		u += du * jump;
		v += dv * jump;
	}
	else
		 offp +=  xStart;

	const unsigned short* const textureData = board.m_bindTexture->Data();
	const unsigned int drawMaskX =	board.m_bindTexture->DrawMaskX();
	const unsigned int drawMaskY =	board.m_bindTexture->DrawMaskY();
	const unsigned int vShift = 		board.m_bindTexture->VShift();

	while (1)
	{	
		const int	idx = (((u >> Lib3D::DIV_SHIFT) & drawMaskX) | (((v >> Lib3D::DIV_SHIFT) & drawMaskY) << vShift));

		*offp = CLib2D::FastColorAdd(*offp,textureData[idx]);

		++offp;	// next pixel in the screen buffer

		if(offp >= offpEnd)
			return;

		u += du;
		v += dv;
	}
}

void Lib3D::DrawScanLineAdditiveAlpha(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	const unsigned short* const offpEnd	= LineEnd(xStop,offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen
	A_ASSERT(offpEnd>=offp && offpEnd <= (offp + board.m_dispX));

	const int du = board.m_du;			// du,dv and dz are the amount of change (delta) to interpolate the values from pixel to pixel
	const int dv = board.m_dv;		

	
	int u = i_param.u;
	int v = i_param.v;
	
	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
	{
		const int jump  =  0-xStart;	// number of pixel to jump

		u += du * jump;
		v += dv * jump;
	}
	else
		 offp +=  xStart;

	const unsigned short* const textureData = board.m_bindTexture->Data();
	const unsigned int drawMaskX =	board.m_bindTexture->DrawMaskX();
	const unsigned int drawMaskY =	board.m_bindTexture->DrawMaskY();
	const unsigned int vShift = 	board.m_bindTexture->VShift();
	const int alpha = board.m_bindTexture->m_globalAlpha >> 4;

	while (1)
	{	
		const int	idx = (((u >> Lib3D::DIV_SHIFT) & drawMaskX) | (((v >> Lib3D::DIV_SHIFT) & drawMaskY) << vShift));
		const int color = textureData[idx];
		//*offp = CLib2D::FastColorAdd(*offp,textureData[idx]);
		if (color)
			*offp = CLib2D::MixColor(color, *offp, alpha);

		++offp;	// next pixel in the screen buffer

		if(offp >= offpEnd)
			return;

		u += du;
		v += dv;
	}
}

void Lib3D::DrawScanLineAlphaMask(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	const unsigned short* const offpEnd	= LineEnd(xStop,offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen
	A_ASSERT(offpEnd>=offp && offpEnd <= (offp + board.m_dispX));

	const int du = board.m_du;			// du,dv and dz are the amount of change (delta) to interpolate the values from pixel to pixel
	const int dv = board.m_dv;		

	
	int u = i_param.u;
	int v = i_param.v;
	
	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
	{
		const int jump  =  0-xStart;	// number of pixel to jump

		u += du * jump;
		v += dv * jump;
	}
	else
		 offp +=  xStart;

	const unsigned short* const textureData = board.m_bindTexture->Data();
	const unsigned int drawMaskX =	board.m_bindTexture->DrawMaskX();
	const unsigned int drawMaskY =	board.m_bindTexture->DrawMaskY();
	const unsigned int vShift = 	board.m_bindTexture->VShift();
//	const int alpha = board.m_bindTexture->m_globalAlpha >> 4;
	unsigned char *mask = board.m_bindTexture->m_mask;

	while (1)
	{	
		const int	idx = (((u >> Lib3D::DIV_SHIFT) & drawMaskX) | (((v >> Lib3D::DIV_SHIFT) & drawMaskY) << vShift));
		const int color = textureData[idx];
		*offp = CLib2D::MixColor(color, *offp, (mask[idx]) >> 4);
		//*offp = color;
		++offp;	// next pixel in the screen buffer

		if(offp >= offpEnd)
			return;

		u += du;
		v += dv;
	}
}

void Lib3D::DrawScanLineShadow(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZUV& i_param,int xStop)
{
	//unsigned short  * lib2dDestPtr	= CHighGear::GetInstance()->GetLib2D().GetDestPtr();
	unsigned char  * shadowPtr = CHighGear::GetInstance()->GetLib3D().m_ShadowPtr;
	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);
	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	const unsigned short* const offpEnd	= LineEnd(xStop,offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen
	A_ASSERT(offpEnd>=offp && offpEnd <= (offp + board.m_dispX));

	if (xStart > 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
		offp +=  xStart;

	shadowPtr += offp - board.m_screen;
	const int shadowConst = CHighGear::GetInstance()->GetLib3D().shadowConst;
	while (offp < offpEnd)
	{	
		if (*shadowPtr != shadowConst)
		{
			*offp = CLib2D::MixBlack16(*offp, CAR_SHADOW_COLOR);
			*shadowPtr = shadowConst;
		}

		++offp;	// next pixel in the screen buffer
		++shadowPtr;
	}
}

// ---------------------------------------------------------------------------
//	board:		The structure containing the context information and the screen buffer
//	yDraw			Current line number (y) in the screen buffer
//	x,				Interpolated x value: current column (x) shifted left 16
//	z,				Interpolated z value: Depth of the first pixel, shifted left 16
//	u,				U and V are texture coordinate, both are interpolated from the polygon vertex, both are shifted left 16
//	v,
//	xStop,		One past the last column (x) to draw (not shifted)
// ---------------------------------------------------------------------------


void Lib3D::DrawScanLineSubstractive(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop)
{
	using namespace Lib3D;
	unsigned short  * lib2dDestPtr	= CHighGear::GetInstance()->GetLib2D().GetDestPtr();
	//PARAMASM	param;		// must be first

	const int xStart = DownShift16(i_param.x);		// [NOTE] properly rounded to avoid overlaps and cracks

	A_ASSERT(yDraw >=0 && yDraw <= board.m_dispY);

	unsigned short* offp = board.m_screen + (yDraw * board.m_dispX);	// pointer to the pixel line to be drawn in the screen buffer

	const unsigned short* const offpEnd	= LineEnd(xStop,offp, board.m_pHG);		// One past the last pixel to draw, clamped to the edge of the screen

	A_ASSERT(offpEnd>=offp && offpEnd <= (offp + board.m_dispX));

	const int du = board.m_du; // du,dv and dz are the amount of change (delta) to interpolate the values from pixel to pixel
	const int dv = board.m_dv;		

	int u = i_param.u;
	int v = i_param.v;

	if(xStart < 0)	// If the line start before the left edge of the screen, skip all the pixels untils the edge
	{
		const int jump  =  0-xStart;	// number of pixel to jump

		u += du * jump;
		v += dv * jump;
	}
	else
		offp +=  xStart;	

	const unsigned short* textureData	= board.m_bindTexture->Data();
	unsigned int drawMaskX				= board.m_bindTexture->DrawMaskX();
	unsigned int drawMaskY				= board.m_bindTexture->DrawMaskY();
	
	if (drawMaskX==0x0F)
	{
		// 16X16 texture for the shadow, hardcode some values
		while (1)
		{
			const int	idx = ((u >> Lib3D::DIV_SHIFT) & 0x0F) | (((v >> (Lib3D::DIV_SHIFT - 4)) & (0x0F<<4) ));

			*offp = CLib2D::MixBlack16(*offp, 16 - (textureData[idx] & 0xf));	// [NOTE] could be faster by storing the texture this way
			++offp;	// next pixel in the screen buffer

			if(offp >= offpEnd)
				return;

			CHK_ADD(u,du);
			CHK_ADD(v,dv);
			u += du;
			v += dv;
		}
	}
	else
	{
		const int	vShift					= board.m_bindTexture->VShift();
		while (1)
		{
			const int coord_u = (u >> Lib3D::DIV_SHIFT); 
			const int coord_v = (v >> Lib3D::DIV_SHIFT);
			//	Since the texture buffer are allway square and heve a size that is a power of 2
			//	the drawmask map the texture coordinate in the available texturespace, vShift 
			//	actualy multiply the v by the size of the texture
			const int	idx = ((coord_u & drawMaskX) | ((coord_v & drawMaskY) << vShift));
			// [FIXME] test which one is faster
			//*param.offp = CLib2D::FastColorSub(*param.offp,param.textureData[idx]);
			
			unsigned short * CurrentPixelInShadowBuffer = lib2dDestPtr + (offp - board.m_screen);
			if (*CurrentPixelInShadowBuffer==0)
			{
				*offp = CLib2D::MixBlack16(*offp, 16 - (textureData[idx] & 0xf));	// [NOTE] could be faster by storing the texture this way
				*CurrentPixelInShadowBuffer = 1;
			}
			++offp;	// next pixel in the screen buffer

			if(offp >= offpEnd)
				return;

			CHK_ADD(u,du);
			CHK_ADD(v,dv);
			u += du;
			v += dv;
		}
	}
}





