#include "HG/HighGear.h"

#include "Lib2D/FontWrapper.h"
#include "Lib2D/AuroraSprite.h"
#include "str_utils.h"

CFontWrapper::CFontWrapper(const char * _spriteName)
{	
	spriteFont = NEW CSprite(_spriteName);
}

CFontWrapper::~CFontWrapper()
{
	MM_DELETE spriteFont;
}

void CFontWrapper::DrawFrame(CLib2D& lib2d, int x, int y, int frame) const
{
	spriteFont->DrawFrame(lib2d, x, y, frame);
}

short CFontWrapper::GetStringWidth(const unsigned short* s)
{
	return spriteFont->GetStringWidth(s);
}

short CFontWrapper::GetStringHeight(const unsigned short* s)
{
	return spriteFont->GetStringHeight(s);
}

short CFontWrapper::GetFontHeight() const 
{
	return spriteFont->GetFontHeight();
}

short CFontWrapper::GetFontWidth() const 
{
	return spriteFont->GetFontWidth();
}

short CFontWrapper::GetLineSpacing() const 
{
	return spriteFont->GetLineSpacing();
}

short CFontWrapper::GetCharSpacing() const
{
	return spriteFont->GetCharSpacing();
}

void CFontWrapper::SetLineSpacing(short _lineSpacing) 
{
	spriteFont->SetLineSpacing(_lineSpacing);
}

void CFontWrapper::SetCharSpacing(short _charSpacing)
{
	spriteFont->SetCharSpacing(_charSpacing);
	spriteFont->ReplaceWidthChar(_charSpacing);
}

short CFontWrapper::GetGlobalAlpha()
{
	return spriteFont->GetGlobalAlpha();
}

void CFontWrapper::SetMapping(const unsigned short* mapping)
{
	spriteFont->SetMapping(mapping);
}

void CFontWrapper::DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, int align)
{
	spriteFont->DrawString(lib2d, x, y, s, CSprite::Align(align));
}

void CFontWrapper::DrawString(CLib2D& lib2d, int x, int y, const int number, int align)
{
	unsigned short sTmp [256];
	sprintf(sTmp, "%d", number);

	spriteFont->DrawString(lib2d, x, y, sTmp, CSprite::Align(align));
}

void CFontWrapper::DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, const unsigned short* lastCharacter, int align)
{
	spriteFont->DrawString(lib2d, x, y, s, lastCharacter, CSprite::Align(align));
}

int CFontWrapper::DrawStringWithLimitedWidth(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth , bool ignoreNewLines, int align, bool __ignoreSpaces)
{
	return spriteFont->DrawStringWithLimitedWidth(lib2d, x, y, s, maxWidth, ignoreNewLines, CSprite::Align(align), __ignoreSpaces);
}

int CFontWrapper::GetStringWithLimitedWidthLinesNumber(const unsigned short* s, int maxWidth , bool ignoreNewLines)
{
	unsigned short buf[MAX_STRING_LENGTH];
	int nLines = spriteFont->FormatStringWithLimitedWidth(s, buf, maxWidth,ignoreNewLines);

	return nLines;
}

int CFontWrapper::FormatStringWithLimitedWidth(const unsigned short* s, unsigned short* buf, int maxWidth, bool ignoreNewLines)
{
	return spriteFont->FormatStringWithLimitedWidth(s, buf, maxWidth,ignoreNewLines);
}

void CFontWrapper::DrawStringWithLimitedWidthByAdjustingCharSpacing(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth ,  int align)
{
	spriteFont->DrawStringWithLimitedWidthByAdjustingCharSpacing(lib2d, x, y, s, maxWidth, CSprite::Align(align));
}

void CFontWrapper::SetGlobalAlpha(short _globalAlpha)
{
	spriteFont->SetGlobalAlpha(_globalAlpha);

}

void CFontWrapper::SetPalette(short _palette)
{
	spriteFont->SetPalette(_palette);
}

short CFontWrapper::GetPalette()
{
	return spriteFont->GetPalette();
}

int CFontWrapper::GetStringWidthLimited(const unsigned short* s, int maxWidth, bool ignoreNewLines)
{
	return spriteFont->GetStringWidthLimited(s, maxWidth, ignoreNewLines);
}

int	CFontWrapper::GetStringHeightLimited(const unsigned short* s, int maxWidth, bool ignoreNewLines)
{
	return spriteFont->GetStringHeightLimited(s, maxWidth, ignoreNewLines);
}
