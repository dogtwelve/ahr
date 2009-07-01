// Lib2D.h: interface for the CLib2D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_Lib2D_H__253C33E5_ABF9_4651_A3C2_308EC5031717__INCLUDED_)
#define AFX_Lib2D_H__253C33E5_ABF9_4651_A3C2_308EC5031717__INCLUDED_

#include "DevUtil.h"
#include "Lib2D/Rect.h"
#include "Lib3D.h"
#include <stack>
#include <string>
#include <map>

#define COLOR_BLACK      0
#define COLOR_RED        0xF00
#define COLOR_LIGHTRED   0xF55
#define COLOR_DARKRED    0x700
#define COLOR_GREEN      0x0F0
#define COLOR_LIGHTGREEN 0x5F5
#define COLOR_DARKGREEN  0x070
#define COLOR_BLUE       0x00F
#define COLOR_AIM_BLUE   0x8DF
#define COLOR_LIGHTBLUE  0x55F
#define COLOR_DARKBLUE   0x007
#define COLOR_YELLOW     0xFF0
#define COLOR_GRAY       0x888
#define COLOR_LIGHTGRAY  0xCCC
#define COLOR_DARKGRAY   0x555
#define COLOR_WHITE      0xFFF
#define COLOR_MAGENTA    0xF0F
#define COLOR_TRANSPARENT 0x1000

// font defines

#define SMALL_FONT_CHAR_SPACING			(1)
#define NORMAL_FONT_CHAR_SPACING		(1)
#define LARGE_FONT_CHAR_SPACING			(-1)

#define NORMAL_FONT_JP_CHAR_SPACING		(1)
#define INGAME_FONT_JP_CHAR_SPACING		(-3)
#define SOFT_KEYS_FONT_CHAR_SPACING		1

#define COLOR_444(r,g,b) (((r>>4)<<8) | ((g>>4)<<4) | (b>>4))
#define COLOR_565(r,g,b) (((r>>3)<<11) | ((g>>2)<<5) | (b>>3))

#define TEXT_ALIGN_LEFT   0
#define TEXT_ALIGN_CENTER 1
#define TEXT_ALIGN_RIGHT  2

#define FONT_SMALL 0


#ifdef __565_RENDERING__
	// 565
	#define COLOR_MASK_R		0xF800		// 1111100000000000
	#define COLOR_MASK_G		0x07E0		// 0000011111100000
	#define COLOR_MASK_B		0x001F		// 0000000000011111
#else
	// 444

	#define COLOR_MASK_R		0x0F00
	#define COLOR_MASK_G		0x00F0
	#define COLOR_MASK_B		0x000F
#endif

class CBitFont;
namespace Lib3D
{
	class CLib3D;
}

class L2Font;

//namespace Lib2D{

    class CLib2D
    {
    public:
        CLib2D(unsigned short *in_DestPtr, unsigned short in_Width, unsigned short in_Height, const Lib3D::CLib3D& in_Lib3D);
    	virtual ~CLib2D();
		void DarkenScreen();

		void BlitBackBuffer();
		inline unsigned short* GetBackBuffer() { return m_backBuffer; }

		void DrawPixel(int in_DstX, int in_DstY, short in_Color) const;
        void DrawLine(int in_SrcX, int in_SrcY, int in_DstX, int in_DstY, short in_Color) const;
        void DrawDotLine(int in_SrcX, int in_SrcY, int in_DstX, int in_DstY, unsigned char in_nPattern, short in_Color) const;
        void DrawRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, unsigned short in_FillColor, unsigned short in_EdgeColor = COLOR_TRANSPARENT) const;
        void DrawAlphaRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, short in_FillColor, int in_nColorWeight) const;
		void DrawAlphaColorRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, short in_FillColor, short in_BorderColor) const;
        void DrawAlphaColorLine(int in_SrcX, int in_SrcY, int in_DstX, int in_DstY, short in_Color) const;
        
		void DrawHTriangle( int nXbl, int nXbr,int nYb, int nXt, int nYt, short nFillColor, short nBorderColor = 0 ) const;

        void DrawString(int intX, int intY, const unsigned short* chrpString, unsigned short in_Color, short shtFontType = FONT_SMALL, short shtAlignment = TEXT_ALIGN_LEFT) const;

		void DrawArc(int in_nX, int in_nY, int in_nRay, int in_nAngleBegin, int in_nAngleEnd, short in_FillColor, short in_EdgeColor = COLOR_TRANSPARENT) const;
        void DrawRoundRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nRay, short nFillColor, short nBorderColor = COLOR_TRANSPARENT/*, int nBorderWidth = 0*/ ) const;
        void DrawRoundRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nRayUp, int in_nRayDown, short nFillColor, short nBorderColor/*, int nBorderWidth = 0*/ ) const;

        enum{ k_nArrowUp=0, k_nArrowDown, k_nArrowLeft, k_nArrowRight};
        void DrawArrow(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nAngle, short in_FillColor) const;

        enum{ k_nBottomLeft=0, k_nTopLeft, k_nBottomRight, k_nTopRight};
        void DrawRectTriangle(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nCorner, short in_FillColor) const;
        //void Attenuate(unsigned char in_nIntensity) const;
        //void Attenuate(int in_nX, int in_nY, int in_nWidth, int in_nHeight, unsigned char in_nIntensity) const;
        void Clear();
        
        void setColor(u32 color = 0xFFFFFFFF);

		inline short GetClipX() const { return m_clipX; }
		inline short GetClipY() const { return m_clipY; }
		inline short GetClipWidth() const { return m_clipWidth; }
		inline short GetClipHeight() const { return m_clipHeight; }
		inline void GetClip(short &x, short &y, short &w, short &h)
		{
			x = m_clipX;
			y = m_clipY;
			w = m_clipWidth;
			h = m_clipHeight;
		}
		inline void SetClip(short x, short y, short w, short h)
		{
			m_clipX = x;
			m_clipY = y;
			m_clipWidth = w;
			m_clipHeight = h;
		}
		inline void ResetClip() { m_clipX = -1; }

		template <typename T1, typename T2> static inline int FastColorSub(T1 const a, T2 const b)
		{
			/*
			int const r = (x & 0xf00) - (y & 0xf00);
			int const g = (x & 0xf0)  -	(y & 0xf0);
			int const b = (x & 0xf)   - (y & 0xf);
			int const a0g0r0b = (r &0xf00) | (b & 0xf) | ((g & 0xf0)<< 12);
			int const underflow = (r & 0x1000) | (b & 0x10) | ((g & 0x100)<< 12) ;
			int const c0g0r0b = a0g0r0b & (~(underflow - (underflow >> 4)));
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
			*/
#ifdef __565_RENDERING__
			int const cr = (a & 0xf800) - (b & 0xf800);
			int const cg = (a & 0x7E0)  -	(b & 0x7e0);
			int const cb = (a & 0x1f)   - (b & 0x1f);
			int const a0g0r0b = (cr &0xf800) | (cb & 0x1f) | ((cg & 0x7e0)<< 16);
			int const underflow = (cr & 0x10000) | (cb & 0x20) | ((cg & 0x800)<< 16) ;
			int const c0g0r0b = a0g0r0b & (~(underflow - (underflow >> 4)));
			return (c0g0r0b & 0xf81f) | ((c0g0r0b & 0x07e000000) >> 16);
#else
			int const a0g0r0b = (a & 0xf0f) | ((a & 0x0f0) << 12);
			int const b0g0r0b = (b & 0xf0f) | ((b & 0x0f0) << 12);
			int const c1g1r1b = 0x101010 + a0g0r0b - b0g0r0b;
			int const overflow = c1g1r1b & 0x101010;
			int const c0g0r0b = c1g1r1b & (overflow - (overflow >> 4));
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
#endif
		}

		//// color addition (with overflow handling) ////
/*#if defined(USE_ARM_ASM_LIB2D) && defined(__SYMBIAN32__)
		static inline unsigned int FastColorAdd(unsigned int const a, unsigned int const b)
		{
			unsigned int result;
			asm volatile
			(
				"
				bic r2, %1, #61440
				bic %0, %2, #61440
				add r2, r2, %0
				eor r1, %1, %2
				eor r0, r2, r1
				and r0, r0, #16
				rsb r2, r0, r2
				eor r1, r2, r1
				and r1, r1, #256
				rsb r2, r1, r2
				and %0, r2, #4096
				orr %0, %0, r1
				orr %0, %0, r0
				sub %0, %0, %0, asr #4
				orr %0, %0, r2			@ [NOTE] don't care about the resulting alpha :P
				"
				: "=r" (result)		// outputs (%0)
				: "r" (a), "r" (b)	// inputs (%1,%2)
				: "r0", "r1", "r2"	// clobbers
			);
			return result;
		}
#else*/
		template <typename T1, typename T2> static inline int FastColorAdd(T1 const a, T2 const b)
		{
#if 0		// [FIXME] Fred, which one is faster on ARM?  Can we do a nice #ifdef to choose which to use?
			int const a0g0r0b = (a & 0xf0f) | ((a & 0x0f0) << 12);
			int const b0g0r0b = (b & 0xf0f) | ((b & 0x0f0) << 12);
			int const c1g1r1b = a0g0r0b + b0g0r0b;
			int const overflow = c1g1r1b & 0x101010;
			int const c0g0r0b = c1g1r1b | (overflow - (overflow >> 4));
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
#else
	#ifdef __565_RENDERING__
/*			int const a0g0r0b = ((int)a & 0xf81f) | (((int)a & 0x07c0) << 15);
			int const b0g0r0b = ((int)b & 0xf81f) | (((int)b & 0x07c0) << 15);
			int const c1g1r1b = a0g0r0b + b0g0r0b;
			int const overflow = c1g1r1b & 0x4001020;
			int const c0g0r0b = c1g1r1b | (overflow - (overflow >> 5));
			return (c0g0r0b & 0xf81f) | ((c0g0r0b & 0x03f00000) >> 15);*/
			int sum = ((int)a) + ((int)b);
			int const product = ((int)a) ^ ((int)b);
			int const carry1 = (sum ^ product) & 0x0020;
			int const carry2 = ((sum -= carry1) ^ product) & 0x0800;
			int const carries = ((sum -= carry2) & 0x10000) | carry2 | carry1;
			//return (sum & ~0xf000) | (carries - (carries >> 4));
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha
	#else
			int sum = (a & ~0xf000) + (b & ~0xf000);
			int const product = a ^ b;
			int const carry1 = (sum ^ product) & 0x0010;
			int const carry2 = ((sum -= carry1) ^ product) & 0x0100;
			int const carries = ((sum -= carry2) & 0x1000) | carry2 | carry1;
			//return (sum & ~0xf000) | (carries - (carries >> 4));
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha
	#endif
#endif
		}
//#endif
		template <typename T> static inline T FastColorAdd2Pixel(T const a, T const b)
		{
#if 0		// [FIXME] Fred, which one is faster on ARM?  Can we do a nice #ifdef to choose which to use?
			T const a0g0r0b = (a & (T)0x000f000f) | ((a & (T)0x00f000f0) << 1) | ((a & (T)0x0f000f00) << 2);
			T const b0g0r0b = (b & (T)0x000f000f) | ((b & (T)0x00f000f0) << 1) | ((b & (T)0x0f000f00) << 2);
			T const c1g1r1b = a0g0r0b + b0g0r0b;
			T const overflow = c1g1r1b & (T)0x42104210;
			T const c0g0r0b = c1g1r1b | (overflow - (overflow >> 4));
			return (c0g0r0b & (T)0x000f000f) | ((c0g0r0b >> 1) & (T)0x00f000f0) | ((c0g0r0b >> 2) & (T)0x0f000f00);
#else
			T sum = (a & (T)~0xf000f000) + (b & (T)~0xf000f000);
			T const product = a ^ b;
			T const carry1 = (sum ^ product) & (T)0x00100010;
			T const carry2 = ((sum -= carry1) ^ product) & (T)0x01000100;
			T const carries = ((sum -= carry2) & (T)0x10001000) | carry2 | carry1;
			//return (sum & (T)~0xf000f000) | (carries - (carries >> 4));
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha
#endif
		}
		template <typename T1, typename T2> static inline int ApproximateColorAdd(T1 const a, T2 const b)
		{
#ifdef __565_RENDERING__
			int sum = a + b;
			int const carries = (sum ^ a ^ b) & 0x10820;
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha
#else
			// [NOTE] error is +/- 4.5%
			int sum = a + b;
			int const carries = (sum ^ a ^ b) & 0x1110;
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha
#endif
		}
		template <typename T> static inline T ApproximateColorAdd2Pixel(T const a, T const b)
		{
			// [NOTE] error is +/- 4.5%
			T const sum = (a & (T)~0xF000) + (b & (T)~0xF000);
			T const carries = (sum ^ a ^ b) & (T)0x11101110;
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha
		}
		template <typename T1, typename T2> static inline int CrappyColorAdd(T1 const a, T2 const b)
		{
			// [NOTE] error is +/- 46.7%
			return ( a | b );
		}
		template <typename T> static inline T CrappyColorAdd2Pixel(T const a, T const b)
		{
			// [NOTE] error is +/- 46.7%
			//        for dark colors (50% brightness or less) error is still +/- 46.7%
			//        for very dark colors (25% brightness or less) error is +/- 26.7%
			// [NOTE] this works only with very dark 2nd texture, and is really crappy (only used for envmap)
			return ( a | b );
		}
		//// color addition with high dynamic range alpha (with overflow handling) ////
		template <typename T1, typename T2, typename T3> static inline int FastColorAddAlphaHD(T1 const a, T2 const b, T3 const alpha)
		{
			// [NOTE] accepts alpha from 0x00 to 0x30 (i.e. 3x bright)
#ifdef __565_RENDERING__
/*			int sum = MixBlack16( b, alpha ) + a;
			int const carries = (sum ^ a ^ b) & 0x10820;
			return sum | (carries - (carries >> 4));	// [NOTE] don't care about the resulting alpha*/
			// not sure
			int const a0g0r0b = (a & 0xf81f) | ((a & 0x07e0) << 16);
			int const b0g0r0b = (b & 0xf81f) | ((b & 0x07e0) << 16);
			int const c1g1r1b = b0g0r0b + (((a0g0r0b * alpha) >> 4) & 0x7e0f81f);
			int const overflow = (c1g1r1b & 0x8010020) | ((c1g1r1b & 0x10020040) >> 1);
			int const c0g0r0b = c1g1r1b | (overflow - (overflow >> 4));
			return (c0g0r0b & 0xf81f) | ((c0g0r0b & 0x7e00000) >> 16);
#else
			int const a0g0r0b = (a & 0xf0f) | ((a & 0x0f0) << 12);
			int const b0g0r0b = (b & 0xf0f) | ((b & 0x0f0) << 12);
			int const c1g1r1b = b0g0r0b + (((a0g0r0b * alpha) >> 4) & 0x0f0f0f);
			int const overflow = (c1g1r1b & 0x101010) | ((c1g1r1b & 0x202020) >> 1);
			int const c0g0r0b = c1g1r1b | (overflow - (overflow >> 4));
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
#endif
		}
		//// color mixing with alpha from 0x0 (just b) to 0xf (just a) ////
		template <typename T1, typename T2, typename T3> static inline int MixColor(T1 const a, T2 const b, T3 const alpha)
		{
#ifdef __565_RENDERING__
			// [NOTE] expects alpha from 0x0 (just b) to 0xf (just a)
			//int const alpha10 = (alpha > 0x8) ? (alpha + 1) : alpha;
			int const a0g0r0b = ((a & 0xf81f) | ((a & 0x7e0) << 16));
			int const b0g0r0b = ((b & 0xf81f) | ((b & 0x7e0) << 16));
			//int const cg0r0b0 = (a0g0r0b - b0g0r0b) * alpha10 + (b0g0r0b << 4);
			int const cg0r0b0 = (a0g0r0b - b0g0r0b) * alpha + (b0g0r0b << 4);
			return ((cg0r0b0 & 0xf81f0) >> 4) | ((cg0r0b0 & 0x7e000000) >> (20));
#else
			// [NOTE] expects alpha from 0x0 (just b) to 0xf (just a)
			int const alpha10 = (alpha > 0x8) ? (alpha + 1) : alpha;
			int const a0g0r0b = ((a & 0xf0f) | ((a & 0x0f0) << 12));
			int const b0g0r0b = ((b & 0xf0f) | ((b & 0x0f0) << 12));
			int const cg0r0b0 = (a0g0r0b - b0g0r0b) * alpha10 + (b0g0r0b << 4);
			return ((cg0r0b0 & 0xf0f0) >> 4) | ((cg0r0b0 & 0xf00000) >> 16);
#endif
		}

#ifdef __565_RENDERING__
		//// color mixing for brew from 444 color with alpha from 0x0 (just b) to 0xf (just a) ////
		template <typename T1, typename T2, typename T3> static inline int MixColorBrew444(T1 const a, T2 const b, T3 const alpha)
		{
			// [NOTE] expects alpha from 0x0 (just b) to 0xf (just a)
			int const alpha10 = (alpha > 0x8) ? (alpha + 1) : alpha;
			int const a0g0r0b = ((a & 0xF00) << 4) | ((a & 0xF) << 1) | ((a & 0xF0) << (3+16));//((a & 0xf81f) | ((a & 0x7e0) << 16));
			int const b0g0r0b = ((b & 0xf81f) | ((b & 0x7e0) << 16));
			int const cg0r0b0 = (a0g0r0b - b0g0r0b) * alpha10 + (b0g0r0b << 4);
			return ((cg0r0b0 & 0xf81f0) >> 4) | ((cg0r0b0 & 0x7e000000) >> (16+4));
		}
#endif

		//// color mixing with alpha from 0x00 (just b) to 0x10 (just a) ////
/*#if defined(USE_ARM_ASM_LIB2D) && defined(__SYMBIAN32__)
		static inline unsigned int MixColor16(unsigned int const a, unsigned int const b, unsigned int const alpha)
		{
			unsigned int result;
			asm volatile
			(
				"
				mov r0, #3840
				orr r0, r0, #15				@ r0 = F0F
				and r1, %1, r0				@ r1 = a & 0x0f0f
				and	%0, %1, #240			@ %0 = a & 0x00f0
				orr	r1, r1, %0, lsl #12		@ r1 = a0g0r0b
				and r2, %2, r0				@ r2 = b & 0x0f0f
				and	%0, %2, #240			@ %0 = b & 0x00f0
				orr	r2, r2, %0, lsl #12		@ r2 = b0g0r0b
				sub	r1, r1, r2				@ r1 = a0g0r0b - b0g0r0b
				mov	r2, r2, lsl #4			@ r2 = b0g0r0b << 4
				mla	%0, r1, %3, r2			@ %0 = cg0r0b0
				and	r0, r0, %0, lsr #4		@ r0 = (0x0f0f & (cg0r0b0 >> 4))
				and r1, %0, #15728640		@ r1 = cg0r0b0 & 0xf00000
				orr	%0, r0, r1, lsr #16		@ return (0x0f0f & (cg0r0b0 >> 4)) | ((cg0r0b0 & 0xf00000) >> 16)
				"
				: "=r" (result)					// outputs (%0)
				: "r" (a), "r" (b), "r" (alpha)	// inputs (%1,%2,%3)
				: "r0", "r1", "r2"				// clobbers
			);
			return result;
		}
#else*/
		template <typename T1, typename T2, typename T3> static inline int MixColor16(T1 const a, T2 const b, T3 const alpha)
		{
#ifdef __565_RENDERING__
			int const a0g0r0b = ((a & 0xf81f) | ((a & 0x07e0) << 16));
			int const b0g0r0b = ((b & 0xf81f) | ((b & 0x07e0) << 16));
			int const cg0r0b0 = (a0g0r0b - b0g0r0b) * alpha + (b0g0r0b << 4);
			return ((cg0r0b0 & 0xf81f0) >> 4) | ((cg0r0b0 & 0x7e000000) >> (16+4));
#else
			// [NOTE] expects alpha from 0x00 (just b) to 0x10 (just a)
			int const a0g0r0b = ((a & 0xf0f) | ((a & 0x0f0) << 12));
			int const b0g0r0b = ((b & 0xf0f) | ((b & 0x0f0) << 12));
			int const cg0r0b0 = (a0g0r0b - b0g0r0b) * alpha + (b0g0r0b << 4);
			return ((cg0r0b0 & 0xf0f0) >> 4) | ((cg0r0b0 & 0xf00000) >> 16);
#endif
		}
//#endif
		//// black mixing with alpha from 0x00 (just black) to 0x10 (just b) ////
/*#if defined(USE_ARM_ASM_LIB2D) && defined(__SYMBIAN32__)
		static inline unsigned int MixBlack16(unsigned int const b, unsigned int const alpha)
		{
			unsigned int result;
			asm volatile
			(
				"
				mov r0, #3840
				orr r0, r0, #15				@ r0 = F0F
				and r1, %1, r0				@ r1 = b & 0x0f0f
				and %0, %1, #240			@ %0 = b & 0x00f0
				orr r1, r1, %0, lsl #12		@ r1 = b0g0r0b
				mul r1, r1, %2				@ r1 = cg0r0b0
				and r0, r0, r1, lsr #4		@ r0 = (0x0f0f & (cg0r0b0 >> 4))
				and r1, r1, #15728640		@ r1 = cg0r0b0 & 0xf00000
				orr %0, r0, r1, lsr #16		@ return (0x0f0f & (cg0r0b0 >> 4)) | ((cg0r0b0 & 0xf00000) >> 16)
				"
				: "=r" (result)				// outputs (%0)
				: "r" (b), "r" (alpha)		// inputs (%1,%2)
				: "r0", "r1"				// clobbers
			);
			return result;
		}
#else*/
		template <typename T1, typename T2> static inline int MixBlack16(T1 const b, T2 const alpha)
		{
#ifdef __565_RENDERING__
			//int const b0g0r0b = ((b & 0xf81f) | ((b & 0x7e0) << 16));
			int const cg0r0b0 = (((b & 0xf81f) | ((b & 0x7e0) << 16))) * alpha;
			return ((cg0r0b0 & 0xf81f0) >> 4) | ((cg0r0b0 & 0x7e000000) >> (20));
#else
			// [NOTE] expects alpha from 0x00 (just black) to 0x10 (just b)
			int const b0g0r0b = ((b & 0xf0f) | ((b & 0x0f0) << 12));
			int const cg0r0b0 = b0g0r0b * alpha;
			return ((cg0r0b0 & 0xf0f0) >> 4) | ((cg0r0b0 & 0xf00000) >> 16);
#endif
		}
//#endif
		//// color mixing with specific alpha ////
		template <typename T1, typename T2> static inline int FastColorMix88_12(T1 const a, T2 const b)
		{
#ifdef __565_RENDERING__
			int const a0g0r0b = (a & 0xf81f) | ((a & 0x07e0) << 16);
			int const b0g0r0b = (b & 0xf81f) | ((b & 0x07e0) << 16);
			int const c0g0r0b = ((a0g0r0b<<2) + (a0g0r0b<<1) + a0g0r0b + b0g0r0b)>>3;
			return (c0g0r0b & 0xf81f) | ((c0g0r0b & 0x7e00000) >> 16);
#else
			int const a0g0r0b = (a & 0xf0f) | ((a & 0x0f0) << 12);
			int const b0g0r0b = (b & 0xf0f) | ((b & 0x0f0) << 12);
			int const c0g0r0b = ((a0g0r0b<<2) + (a0g0r0b<<1) + a0g0r0b + b0g0r0b)>>3;
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
#endif
		}
		template <typename T1, typename T2> static inline int FastColorMix75_25(T1 const a, T2 const b)
		{
#ifdef __565_RENDERING__
			int const a0g0r0b = (a & 0xf81f) | ((a & 0x07e0) << 16);
			int const b0g0r0b = (b & 0xf81f) | ((b & 0x07e0) << 16);
			int const c0g0r0b = ((a0g0r0b<<1)+ a0g0r0b + b0g0r0b)>>2;
			return (c0g0r0b & 0xf81f) | ((c0g0r0b & 0x7e00000) >> 16);
#else
			int const a0g0r0b = (a & 0xf0f) | ((a & 0x0f0) << 12);
			int const b0g0r0b = (b & 0xf0f) | ((b & 0x0f0) << 12);
			int const c0g0r0b = ((a0g0r0b<<1)+ a0g0r0b + b0g0r0b)>>2;
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
#endif
		}
		template <typename T1, typename T2> static inline int FastColorMix63_37(T1 const a, T2 const b)
		{
#ifdef __565_RENDERING__
			int const a0g0r0b = (a & 0xf81f) | ((a & 0x07e0) << 16);
			int const b0g0r0b = (b & 0xf81f) | ((b & 0x07e0) << 16);
			int const c0g0r0b = ((a0g0r0b<<2)+ a0g0r0b + (b0g0r0b<<1) + b0g0r0b)>>3;
			return (c0g0r0b & 0xf81f) | ((c0g0r0b & 0x7e00000) >> 16);
#else
			int const a0g0r0b = (a & 0xf0f) | ((a & 0x0f0) << 12);
			int const b0g0r0b = (b & 0xf0f) | ((b & 0x0f0) << 12);
			int const c0g0r0b = ((a0g0r0b<<2)+ a0g0r0b + (b0g0r0b<<1) + b0g0r0b)>>3;
			return (c0g0r0b & 0xf0f) | ((c0g0r0b & 0x0f0000) >> 12);
#endif
		}
		template <typename T1, typename T2> static inline int FastColorMix50_50(T1 const a, T2 const b)
		{
#ifdef __565_RENDERING__
			int const sum = a + b;
			int const units = (a ^ b) & 0x0820;
			return (sum - units) >> 1;
#else
			int const sum = (a & ~0xf000) + (b & ~0xf000);
			int const units = (a ^ b) & 0x0110;
			return (sum - units) >> 1;
#endif
		}
		template <typename T> static inline T FastColorMix2Pixel50_50(T const a, T const b)
		{
			T const sum = (a & (T)~0xf000f000) + (b & (T)~0xf000f000);
			T const units = (a ^ b) & (T)0x01100110;
			return (sum - units) >> 1;
		}
		template <typename T> static inline T FastColorMix2PixelNoMask50_50(T const a, T const b)
		{
			return (a + b - ((a ^ b) & (T)0x01100110)) >> 1;
		}
		template <typename T1, typename T2> static inline int ApproximateColorMix50_50(T1 const a, T2 const b)
		{
			return ((a & 0x0eee) + (b & 0x0eee)) >> 1;
		}
		template <typename T> static inline T ApproximateColorMix2Pixel50_50(T const a, T const b)
		{
			return ((a & (T)0x0eee0eee) + (b & (T)0x0eee0eee)) >> 1;
		}

inline		const Lib3D::CLib3D& GetLib3D() const {return *m_pLib3D;}
inline		void SetLib3D(  Lib3D::CLib3D* pLib) {m_pLib3D = pLib;}

        void CopyBufferTo(unsigned short *in_DestPtr) const;

        void SetDestPtr(unsigned short *in_DestPtr, unsigned short in_Width, unsigned short in_Height);
        void PushClippingRect(const ARect& in_ClippingRect);
        void PopClippingRect();

		unsigned short* GetDestPtr() const;
        unsigned short GetDestWidth() const;
        unsigned short GetDestHeight() const;
        const ARect& GetClippingRect() const;

    protected:
		
		//typedef map<string, L2Font*, less<string>, allocator<L2Font*> > FontMap;
		//FontMap m_Fonts;

		unsigned short *m_DestPtr;
        unsigned short m_DestWidth, m_DestHeight;

        std::stack<ARect> m_ClippingRects;
		//ARect		m_ClippingRects;
//        CBitFont* m_bfopSmallFont;

        // Lib3d is used for color table
        const Lib3D::CLib3D* m_pLib3D;

	private:
		short m_clipX, m_clipY, m_clipWidth, m_clipHeight;

		unsigned short* m_backBuffer;

	public:
		CBitFont* m_bfopSmallFont;
		CHighGear *m_pHG;
		int& m_dispX;
		int& m_dispY;
    };

//}

// Inline functions

inline unsigned short* CLib2D::GetDestPtr() const
{
    return m_DestPtr;
}

inline unsigned short CLib2D::GetDestWidth() const
{
    return m_DestWidth;
}

inline unsigned short CLib2D::GetDestHeight() const
{
    return m_DestHeight;
}

inline const ARect& CLib2D::GetClippingRect() const
{
    return m_ClippingRects.top();
	//return m_ClippingRects;
}

#endif // !defined(AFX_Lib2D_H__253C33E5_ABF9_4651_A3C2_308EC5031717__INCLUDED_)
