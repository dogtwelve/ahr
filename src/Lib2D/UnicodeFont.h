/******************************************************************************
@Author: 		Nguyen Trung Hung
@Email:			hungtrung.nguyen@gameloft.com
@Date create: 	2006-10-20
@Modified by:	
@Date modify:	
@Description:	This class is created to make the using of new bitmap font easier
@
@
@ NOTE : The original class is for java, this is a C++ version
@
******************************************************************************/

#ifndef HG_LIB2D_UNICODE_FONT_H
#define HG_LIB2D_UNICODE_FONT_H

#include "Lib2D/Lib2D.h"

#define kUnicodeFontTransparentColor 0xF81F

#define FAKE_ZERO			(0x1)
#define FAKE_ZERO_SHIFTED	(FAKE_ZERO<<8)

#define TO_UNICODE(c)		(((unsigned short)(c))|FAKE_ZERO_SHIFTED)
#define FROM_UNICODE(c)		((char)(c & 0x00FF))

#define FAKE_ZERO_CHINESE	(0xFF)

#define SPACE_CHAR			(FAKE_ZERO_SHIFTED | ' ')
#define NEW_LINE_CHAR		(FAKE_ZERO_SHIFTED | '\n')

#define DEFAULT_LINE_SPACING	(-2)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

class CUnicodeFont 
{
public:
	enum Align 
	{
		HALIGN_LEFT		= 0x00,
		HALIGN_CENTER	= 0x01,
		HALIGN_RIGHT	= 0x02,

		VALIGN_TOP		= 0x00,
		VALIGN_CENTER	= 0x10,
		VALIGN_BOTTOM	= 0x20
	};

	enum Color
	{
		CLR_WHITE = 0,
		CLR_AQUA,
		CLR_PINK,
		CLR_BLACK,
		CLR_DARK_GRAY,
		CLR_GRAY,
		CLR_GREEN,
		CLR_LIGHT_GRAY,
		CLR_RED,
		CLR_GOLDEN,		
		NUMBER_COLORS,

		CLR_DEFAULT = CLR_WHITE		
	};

	CUnicodeFont();
	CUnicodeFont(const char* strPackageName);
	~CUnicodeFont();

	bool LoadFont(const char* strPackageName);

	void SetColor(unsigned short sNewColor, unsigned short sNewBorderColor);
	void SetColor(Color bitmapColor);

	int DrawChar(CLib2D& lib2d, unsigned short in_sChar, int x, int y);

	int GetFontHeight();

	int GetFontCharWidth(unsigned short in_sChar);

	int GetStringWidth(const unsigned short * in_cStr);
	int GetStringHeight(const unsigned short * in_cStr);

	static int GetStringLength(const unsigned short* in_sStr);

	int	GetStringHeightLimited(const unsigned short* s, int maxWidth, bool ignoreNewLines);
	int	GetStringWidthLimited(const unsigned short* s, int maxWidth, bool ignoreNewLines);

	void DecodeFont(int in_charID, unsigned short* out_sPixelData);

	void DrawString(CLib2D& lib2d, const unsigned short* in_sStr, int x, int y, Align align = Align(HALIGN_LEFT|VALIGN_TOP));
	void DrawString(CLib2D& lib2d, const unsigned short* in_sStr, const unsigned short* in_sStrEnd, int x, int y, Align align = Align(HALIGN_LEFT|VALIGN_TOP));

	int DrawStringWithLimitedWidth(CLib2D& lib2d, const unsigned short* in_sStr, int x, int y, int maxWidth, bool ignoreNewLines, Align align = Align(HALIGN_LEFT|VALIGN_TOP));
	int FormatStringWithLimitedWidth(const unsigned short* s, unsigned short* buf, int maxWidth, bool ignoreNewLines);

	inline static	void	GetClip(short &x, short &y, short &w, short &h) 
	{
		x = CUnicodeFont::clipX; y = CUnicodeFont::clipY; w = CUnicodeFont::clipWidth; h = CUnicodeFont::clipHeight;
	}
	inline static void	SetClip(short x, short y, short w, short h) 
	{
		CUnicodeFont::clipX = x; CUnicodeFont::clipY = y; CUnicodeFont::clipWidth = w; CUnicodeFont::clipHeight = h; 
	}
	inline static void	ResetClip() { CUnicodeFont::clipX = -1; }

	inline	short	GetLineSpacing() const { return lineSpacing; }
	inline	short	GetCharSpacing() const { return charSpacing; }

	inline	void	SetLineSpacing(short _lineSpacing) { lineSpacing = _lineSpacing; }
	inline	void	SetCharSpacing(short _charSpacing) { charSpacing = _charSpacing; }

	inline	void	ResetCharSpacing() { charSpacing = DEFAULT_CHAR_SPACING; }

	inline	short	GetGlobalAlpha()						{ return globalAlpha; }
	inline	void	SetGlobalAlpha(short _globalAlpha)		{ globalAlpha = _globalAlpha; }

	static short		clipX;
	static short		clipY;
	static short		clipWidth;
	static short		clipHeight;

	static const int	SPACE_WIDTH = 3;
	static const int	DEFAULT_CHAR_SPACING = 1;
	
	static bool			m_bDontUseUnicodeFont;

protected:
	int MapChar(unsigned short in_sChar);

	void MeasureString(const unsigned short* in_sStr, const unsigned short* in_lastChar = 0);
	//int FormatStringWithLimitedWidth(const unsigned short* s, unsigned short* buf, int maxWidth, bool ignoreNewLines);

	void CreateColorTable();
	void InitColorWithGradient(int idx, int c1, int c2, int c3, int backColor);

	int				m_lastStringWidth;
	int				m_lastStringHeight;

	int				m_fontHeight;

	unsigned short	m_frontColor;
	unsigned short	m_borderColor;

	int					m_currentColor;
	unsigned short**	m_colorGradients;
	unsigned short*		m_colorBorders;

	short			lineSpacing;
	short			charSpacing;

	short			globalAlpha;

	int		m_fontCharCount;
	unsigned short*	m_sCharEnum;		//array store all chars in game
	unsigned short*	m_sCharImageData;
	char*	m_fontCharWidth;	//array store width of all chars in game
	char**	m_fontPackageData;	//array store font data of all chars in game
	
	int _BITS_SYSTEM_;
	int _BIT_SHIFT_;
	int _BIT_AND_;
	int _BIT_CHECK_;
	int _SCALE_;
};

#endif

