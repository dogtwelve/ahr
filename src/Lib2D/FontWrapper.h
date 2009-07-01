#ifndef __FONT_WRAPPER_H__
#define __FONT_WRAPPER_H__

#include "lib2d/AuroraSprite.h"

class CLib2D;

#define ALIGN_HLEFT		0x00
#define ALIGN_HCENTER	0x01
#define ALIGN_HRIGHT	0x02

#define ALIGN_VTOP		0x00
#define ALIGN_VCENTER	0x10
#define ALIGN_VBOTTOM	0x20

#define UNICODE_COLOR_TABLE_SIZE	10

class CFontWrapper 
{
public:
	CFontWrapper(const char * _spriteName);

	virtual ~CFontWrapper();

	void			DrawFrame (CLib2D& lib2d, int x, int y, int frame) const;

	int				GetStringWidthLimited(const unsigned short* s, int maxWidth ,  bool ignoreNewLines);
	int				GetStringHeightLimited(const unsigned short* s, int maxWidth ,  bool ignoreNewLines);

	int				GetStringWithLimitedWidthLinesNumber(const unsigned short* s, int maxWidth , bool ignoreNewLines);

	int				DrawStringWithLimitedWidth(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth ,  bool ignoreNewLines, int align = 0, bool __ignoreSpaces = false);
	int				FormatStringWithLimitedWidth(const unsigned short* s, unsigned short* buf, int maxWidth, bool ignoreNewLines);
	void			DrawStringWithLimitedWidthByAdjustingCharSpacing(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth ,  int align = 0);
	
	void			SetMapping(const unsigned short* mapping);

	void			DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, int align = 0);
	void			DrawString(CLib2D& lib2d, int x, int y, int number, int align = 0);
	void			DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, const unsigned short* lastCharacter, int align = 0);

	short			GetStringWidth (const unsigned short* s);
	short			GetStringHeight(const unsigned short* s);

	void			SetPalette  (short _palette);
	short			GetPalette  ();

	short			GetFontHeight () const;
	short			GetFontWidth () const;

	short			GetLineSpacing() const;
	short			GetCharSpacing() const;

	inline	void	SetFontFrame  (short _fontFrame)   { spriteFont->SetFontFrame(_fontFrame); }
	void			SetLineSpacing(short _lineSpacing);
	void			SetCharSpacing(short _charSpacing);

	short			GetGlobalAlpha();
	void			SetGlobalAlpha(short _globalAlpha);

//	void			SetUnicodeFontColor(int _palette, CUnicodeFont::Color _color);
/*
	static void		SetChineseLanguage(bool _chinese)
#ifdef SYMBIAN8
		;
#else	// ! SYMBIAN8
	{
		chinese = _chinese;
	}
#endif	// ! SYMBIAN8

	static bool		GetChineseLanguage()
#ifdef SYMBIAN8
		;
#else	// ! SYMBIAN8
	{
		return chinese;
	}
#endif	// ! SYMBIAN8

protected:

#if !defined SYMBIAN8
	static bool		chinese;
#endif	// ! SYMBIAN8
*/
public:

	CSprite*		spriteFont;
	bool			bChinese;
};


#endif
