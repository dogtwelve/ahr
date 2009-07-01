/******************************************************************************
@Author: 		Nguyen Trung Hung
@Email:			hungtrung.nguyen@gameloft.com
@Date create: 	2006-10-20
@Modified by:	Radoslav Daskalov
@Date modify:	
@Description:	This class is created to make the using of new bitmap font easier
@
@
@ NOTE : The original class is for java, this is a C++ version
@
******************************************************************************/

#include "GenDef.h"

#include "UnicodeFont.h"
#include "HG/HighGear.h"


CUnicodeFont::CUnicodeFont() :
	_BITS_SYSTEM_(1),
	_BIT_SHIFT_(0),
	_BIT_AND_(0),
	_BIT_CHECK_(0),
	_SCALE_(0),
	m_fontHeight(0),
	m_frontColor(0),
	m_borderColor(0),
	m_currentColor(0),
	lineSpacing(DEFAULT_LINE_SPACING),
	charSpacing(0),
	globalAlpha(0xF)
{
}

CUnicodeFont::CUnicodeFont(const char* strPackageName) :
	_BITS_SYSTEM_(1),
	_BIT_SHIFT_(0),
	_BIT_AND_(0),
	_BIT_CHECK_(0),
	_SCALE_(0),
	m_fontHeight(0),
	m_frontColor(0),
	m_borderColor(0),
	m_currentColor(0),
	lineSpacing(DEFAULT_LINE_SPACING),
	charSpacing(0),
	globalAlpha(0xF)
{
	LoadFont(strPackageName);
}

short CUnicodeFont::clipX		= -1;
short CUnicodeFont::clipY		= -1;
short CUnicodeFont::clipWidth	= -1;
short CUnicodeFont::clipHeight	= -1;

bool CUnicodeFont::m_bDontUseUnicodeFont = false;

CUnicodeFont::~CUnicodeFont()
{
	if (m_sCharEnum)
	{
		DELETE_ARRAY m_sCharEnum;
	}
	if (m_fontCharWidth)
	{
		DELETE_ARRAY m_fontCharWidth;
	}
	if (m_sCharImageData)
	{
		DELETE_ARRAY m_sCharImageData;
	}
	if (m_fontPackageData)
	{
		for (int i = 0; i < m_fontCharCount; i++)
		{
			DELETE_ARRAY m_fontPackageData[i];
		}
		DELETE_ARRAY m_fontPackageData;
	}
	if (m_colorGradients)
	{
		for (int i = 0; i < NUMBER_COLORS; i++)
		{
			DELETE_ARRAY m_colorGradients[i];
		}
		DELETE_ARRAY m_colorGradients;
	}
	if (m_colorBorders)
	{
		DELETE_ARRAY m_colorBorders;
	}
}

bool CUnicodeFont::LoadFont(const char* strPackageName)
{
	A_IFile* inputFile = A_IFile::Open(strPackageName, A_IFile::OPEN_BINARY | A_IFile::OPEN_READ, false);
	A_ASSERT(inputFile);
	
	if (inputFile)
	{
		char readChar;
		short readShort;

		//get num of character
		inputFile->Read(&readShort, sizeof(readShort));
		m_fontCharCount = readShort;
		
		//allocate mem
		m_sCharEnum = NEW unsigned short [m_fontCharCount];
		m_fontCharWidth = NEW char [m_fontCharCount];
		m_fontPackageData = NEW char* [m_fontCharCount];
		
		//get height of font
		inputFile->Read(&readChar, sizeof(readChar));
		m_fontHeight = readChar;

		//allocate mem for decoded data buffer
		m_sCharImageData = NEW unsigned short[m_fontHeight*m_fontHeight*2];
		
		//get bits system
		inputFile->Read(&readChar, sizeof(readChar));
		_BITS_SYSTEM_ = readChar;
		
		_BIT_SHIFT_ = 4 - _BITS_SYSTEM_;
		if (_BITS_SYSTEM_ == 1)
		{
			_BIT_AND_ = 0x07;
			_BIT_CHECK_ = 0x01;
			_SCALE_ = 0;
		}
		else
		{
			_BIT_AND_ = 0x03;
			_BIT_CHECK_ = 0x03;
			_SCALE_ = 1;
		}
		
		//get data of each character
		int dataSize;

		for (int i=0 ; i < m_fontCharCount ; i++)
		{
			//Read Unicode of character
			inputFile->Read(&readShort, sizeof(readShort));
			m_sCharEnum[i] = readShort;

			
			//Read width of character in this font
			inputFile->Read(&readChar, sizeof(readChar));
			m_fontCharWidth[i] = readChar;

			//Read font data of character
			dataSize = 1 + ((m_fontCharWidth[i]*m_fontHeight)>>_BIT_SHIFT_);

			m_fontPackageData[i] = NEW char[dataSize];

			inputFile->Read(m_fontPackageData[i], dataSize);
		}
	}
	A_IFile::Close(inputFile);

	CreateColorTable();
	SetColor(CUnicodeFont::CLR_DEFAULT);
	ResetCharSpacing();
	return true;
}

void CUnicodeFont::SetColor(unsigned short sNewColor, unsigned short sNewBorderColor)
{
	m_currentColor = -1;
	m_frontColor = sNewColor;
	m_borderColor = sNewBorderColor;
}

void CUnicodeFont::SetColor(Color bitmapColor)
{
	m_currentColor = bitmapColor;
}

int CUnicodeFont::DrawChar(CLib2D& lib2d, unsigned short in_sChar, int x, int y)
{
	if ((in_sChar>>8) == FAKE_ZERO)
	{
		in_sChar = in_sChar & 0x00FF;
	}
	if ((in_sChar&0xFF) == FAKE_ZERO_CHINESE)
	{
		in_sChar = in_sChar & 0xFF00;
	}

	int int_lCharIndex = MapChar(in_sChar);
	
	if (int_lCharIndex == -1)
	{
		//TRACE2("%d char not found", in_sChar);
		return 0;
	}

	int int_lCharWidth = m_fontCharWidth[int_lCharIndex];

	int srcStartX = 0;
	int srcStartY = 0;
	int srcWidth	= int_lCharWidth;
	int srcHeight	= m_fontHeight;

	int destWidth	= lib2d.GetDestWidth();
	int destHeight	= lib2d.GetDestHeight();

	if(CUnicodeFont::clipX == -1) 
	{
		CUnicodeFont::clipX		= 0;
		CUnicodeFont::clipY		= 0;
		CUnicodeFont::clipWidth	= destWidth;
		CUnicodeFont::clipHeight= destHeight;
	}

	int clipStartX = CUnicodeFont::clipX, clipEndX = CUnicodeFont::clipX + CUnicodeFont::clipWidth;
	int clipStartY = CUnicodeFont::clipY, clipEndY = CUnicodeFont::clipY + CUnicodeFont::clipHeight;

	if(x < clipStartX) 
	{
		int clipped = (clipStartX - x);
		srcWidth -= clipped;
		srcStartX += clipped;
		x = clipStartX;
	}

	if(y < clipStartY) 
	{
		int clipped = (clipStartY - y);
		srcHeight -= clipped;
		srcStartY += clipped;
		y = clipStartY;
	}

	if(x + srcWidth > clipEndX)
	{
		int clipped = ((x + srcWidth) - clipEndX);
		srcWidth -= clipped;
	}

	if(y + srcHeight > clipEndY) 
	{
		int clipped = ((y + srcHeight) - clipEndY);
		srcHeight -= clipped;
	}

	// Completely outside of the screen
	if((srcWidth <= 0) || (srcHeight <= 0))
		return int_lCharWidth;

	//////////////////////////////////////////////////////////////////////////////////////////
	
	DecodeFont(int_lCharIndex, m_sCharImageData);

	unsigned short * pScreen = (unsigned short *) lib2d.GetDestPtr();
	unsigned short * pImage  = m_sCharImageData;

	pScreen += y * destWidth + x;
	pImage += srcStartY * int_lCharWidth + srcStartX;

	for (int i = 0; i < srcHeight; i++)
	{
		for (int j = 0; j < srcWidth; j++)
		{
			if (pImage[j] != kUnicodeFontTransparentColor)
			{
				pScreen[j] = CLib2D::MixColor(pImage[j], pScreen[j], globalAlpha);
			}
		}
		pScreen	+= destWidth;
		pImage  += int_lCharWidth;
	}
	
	return int_lCharWidth;
}

int CUnicodeFont::GetFontHeight()
{
	return m_fontHeight;
}

int CUnicodeFont::GetFontCharWidth(unsigned short in_sChar)
{
	if ((in_sChar>>8) == FAKE_ZERO)
	{
		in_sChar = in_sChar & 0xFF;
	}
	if ((in_sChar&0xFF) == FAKE_ZERO_CHINESE)
	{
		in_sChar = in_sChar & 0xFF00;
	}
	int int_lCharIndex = MapChar(in_sChar);
	
	if(in_sChar == SPACE_CHAR)
	{
		return SPACE_WIDTH;
	}

	if (int_lCharIndex == -1)
	{
		return 0;
	}
	
	return m_fontCharWidth[int_lCharIndex];
}

int CUnicodeFont::GetStringWidth(const unsigned short* in_cStr) 
{
	if (*in_cStr)
	{
		MeasureString(in_cStr);
		return m_lastStringWidth;
	}
	return 0;
}

int CUnicodeFont::GetStringHeight(const unsigned short* in_cStr) 
{
	if (*in_cStr)
	{
		MeasureString(in_cStr);
		return m_lastStringHeight;
	}
	return 0;
}

void CUnicodeFont::MeasureString(const unsigned short* in_sStr, const unsigned short* in_lastChar)
{
	int w = 0, lines = 1;

	m_lastStringWidth = 0;

	const unsigned short* s = in_sStr;

	unsigned short c = NEW_LINE_CHAR;

	while ((s != in_lastChar) && (*s) & 0xFF00 && (*s) & 0x00FF)
	{
		if ( *s == SPACE_CHAR)
		{
			w += (SPACE_WIDTH + charSpacing);
		}
		else if (*s == NEW_LINE_CHAR)
		{
			m_lastStringWidth = max(m_lastStringWidth, w);
			w = 0;
			lines++;
		}
		else
		{
			w += GetFontCharWidth(*s) + charSpacing;
		}
		s++;
	}

	m_lastStringWidth = max(m_lastStringWidth, w) - charSpacing;
	m_lastStringHeight = lines * GetFontHeight() + (lines - 1) * lineSpacing;
}

#undef  MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 512

// Returns the number of lines
int CUnicodeFont::FormatStringWithLimitedWidth(const unsigned short* s, unsigned short* buf, int maxWidth, bool ignoreNewLines)
{
	int nLines = 1;

	int charLength = GetStringLength(s);
	A_ASSERT(charLength < MAX_STRING_LENGTH);
	memcpy(buf, s, (charLength + 1) * sizeof(unsigned short));

	const int wordSpacing = SPACE_WIDTH + charSpacing;

	int lineWidth = -wordSpacing;
	for (int i = 0, j = -1; i <= charLength; i++)	
	{
		if (buf[i] == SPACE_CHAR || buf[i] == 0 || buf[i] == NEW_LINE_CHAR)
		{
			MeasureString(buf + j + 1, buf + i);
			lineWidth += m_lastStringWidth + charSpacing + wordSpacing;
			if (lineWidth > maxWidth)
			{
				if(j!=-1)
				{
					buf[j] = NEW_LINE_CHAR;
					++nLines;
				}
				lineWidth = m_lastStringWidth + charSpacing;
			}

			if (buf[i] == NEW_LINE_CHAR)
			{
				if (ignoreNewLines)
				{
					//i>0 is computed before the buf[i-1] part
					if (i > 0 && buf[i-1] == SPACE_CHAR)
					{
						//Deletes the new line(s)
						while (buf[i] == NEW_LINE_CHAR)
						{
							for (int idxMove = i; idxMove < charLength; idxMove++)
							{
								buf[idxMove] = buf[idxMove+1];
							}
							charLength--;
						}
					}
					else
					{
						buf[i] = SPACE_CHAR;
					}
				}
				else
				{
					lineWidth = -wordSpacing;
					++nLines;
				}
			}
			j = i;
		}
	}

	return nLines;
}

int CUnicodeFont::GetStringWidthLimited(const unsigned short* s, int maxWidth, bool ignoreNewLines)
{
	unsigned short buf[MAX_STRING_LENGTH];
	FormatStringWithLimitedWidth(s, buf, maxWidth, ignoreNewLines);
	return GetStringWidth(buf);
}

int CUnicodeFont::GetStringHeightLimited(const unsigned short* s, int maxWidth, bool ignoreNewLines)
{
	unsigned short buf[MAX_STRING_LENGTH];
	FormatStringWithLimitedWidth(s, buf, maxWidth, ignoreNewLines);
	return GetStringHeight(buf);
}

int CUnicodeFont::GetStringLength(const unsigned short * in_cStr)
{
	int len = 0;
	while (*in_cStr++)
		len++;
	return len;
}

int CUnicodeFont::MapChar(unsigned short in_sChar)
{
	int char_lLeft = 0;
	int char_lRight = m_fontCharCount-1;
	int char_lMid = 0;
	
	while (char_lLeft <= char_lRight)
	{
		char_lMid = (char_lLeft + char_lRight)>>1;
		
		if (m_sCharEnum[char_lMid] == in_sChar)
		{
			return char_lMid;
		}
		else if (m_sCharEnum[char_lMid] < in_sChar)
		{
			char_lLeft = char_lMid+1;
		}
		else
		{
			char_lRight = char_lMid-1;
		}
	}

	return -1;
}

void CUnicodeFont::DecodeFont(int in_charID, unsigned short* out_sPixelData)
{
	int size = m_fontHeight * m_fontCharWidth[in_charID];
	int pack_byte_index = 0;
	int bit_index = 0;

	A_ASSERT(out_sPixelData);
	

	const unsigned short * color = (m_currentColor != -1) ? m_colorGradients[m_currentColor] : &m_frontColor; 
	const unsigned short * border = (m_currentColor != -1) ? m_colorBorders + m_currentColor : &m_borderColor; 

	for (int i = 0, j = 0; i < size; i++, j++)
	{
		if (m_currentColor != -1 && j == m_fontCharWidth[in_charID])
		{
			j = 0;
			color++;
		}

		pack_byte_index = i>>_BIT_SHIFT_;
		bit_index = ((i&_BIT_AND_)<<_SCALE_);
		
		if (((m_fontPackageData[in_charID][pack_byte_index]>>bit_index)&_BIT_CHECK_) == 1)
		{
			out_sPixelData[i] = *color;
		}
		else if (((m_fontPackageData[in_charID][pack_byte_index]>>bit_index)&_BIT_CHECK_) == 2)
		{
			out_sPixelData[i] = *border;
		}
		else
		{
			out_sPixelData[i] = kUnicodeFontTransparentColor;
		}
	}
}

void CUnicodeFont::DrawString(CLib2D& lib2d, const unsigned short* in_sStr, int x, int y, Align align)
{
	const unsigned short * stringStart = in_sStr;
	const unsigned short * stringEnd = in_sStr;
	int yy = y;

	int lineCount = 1;
	while (*stringStart != 0)
	{
		if (*stringStart == NEW_LINE_CHAR)
			lineCount++;

		stringStart++;
	}

	if(align & VALIGN_CENTER)
		yy -= (lineCount - 1) * (GetFontHeight() + lineSpacing) / 2;
	else if(align & VALIGN_BOTTOM)
		yy -= (lineCount - 1) * (GetFontHeight() + lineSpacing);

	stringStart = in_sStr;

	while (*stringStart != 0)
	{
		while(*stringEnd && *stringEnd != NEW_LINE_CHAR)
			  stringEnd++;

		DrawString(lib2d, stringStart, stringEnd, x, yy, align);
		yy += GetFontHeight() + lineSpacing;

		if (*stringEnd == 0) 
			break;

		stringEnd++;
		stringStart = stringEnd;
	}
}

void CUnicodeFont::DrawString(CLib2D& lib2d, const unsigned short* in_sStr, const unsigned short* in_sStrEnd, int x, int y, Align align)
{
	MeasureString(in_sStr, in_sStrEnd);
	int text_w = m_lastStringWidth;
	int text_h = m_lastStringHeight;

	// compute new x coordinate.
	if ((align & HALIGN_RIGHT) != 0) 		{ x -= text_w - 1; }
	else if ((align & HALIGN_CENTER) != 0) { x -= text_w >> 1; }

	// compute new y coordinate.
	if ((align & VALIGN_BOTTOM) != 0) 		{ y -= text_h; }
	else if ((align & VALIGN_CENTER) != 0) { y -= text_h >> 1;}
	
	int x_offset = 0;

	// draw string
	const unsigned short* s = in_sStr;
	while ((s != in_sStrEnd) && (*s) & 0xFF00 && (*s) & 0x00FF)
	{
		if (*s == SPACE_CHAR)
		{
			x_offset += (SPACE_WIDTH + charSpacing);
		}
		else if (*s == NEW_LINE_CHAR)
		{
			y += (lineSpacing + m_fontHeight);
			x_offset = 0;
		}
		else
		{
			x_offset += DrawChar(lib2d, *s, x + x_offset, y) + charSpacing;
		}

		s++;
	}
}

int CUnicodeFont::DrawStringWithLimitedWidth(CLib2D& lib2d, const unsigned short* in_sStr, int x, int y, int maxWidth, bool ignoreNewLines, Align align)
{
	unsigned short buf[MAX_STRING_LENGTH];
    
	int nLines = FormatStringWithLimitedWidth(in_sStr, buf, maxWidth, ignoreNewLines);
	
	DrawString(lib2d, buf, x, y, align);

	return nLines;
}

void CUnicodeFont::CreateColorTable()
{
	m_colorGradients = NEW unsigned short * [NUMBER_COLORS];
	m_colorBorders = NEW unsigned short [NUMBER_COLORS];

	for (int i = 0; i < NUMBER_COLORS; i++)
	{
		m_colorGradients[i] = NEW unsigned short [m_fontHeight];
	}

	InitColorWithGradient(CLR_WHITE,		0xFFFFFF, 0xFFFFFF, 0xFFFFFF,	0x000000);
	InitColorWithGradient(CLR_PINK,			0xFFDBFF, 0xFFCBFF, 0xFFB2FF,	0x8C188C);
	InitColorWithGradient(CLR_BLACK,		0x000000, 0x000000, 0x000000,	0x000000);
	InitColorWithGradient(CLR_DARK_GRAY,	0x444444, 0x444444, 0x444444,	0x000000);
	InitColorWithGradient(CLR_GRAY,			0x888888, 0x888888, 0x888888,	0x303030);
	InitColorWithGradient(CLR_GREEN,		0xBBFFBB, 0x99FF99, 0x88FF88,	0x005500);
	InitColorWithGradient(CLR_LIGHT_GRAY,	0xCCCCCC, 0xCCCCCC, 0xCCCCCC,	0x303030);
	InitColorWithGradient(CLR_RED,			0xFF0000, 0xFF0000, 0xFF0000,	0x000000);
	InitColorWithGradient(CLR_GOLDEN,		0xFFFFFF, 0xFFBA31, 0xFFC321,	0x000000);
	InitColorWithGradient(CLR_AQUA,			0xFFFFFF, 0xFFFFFF, 0xA5FFFF,	0x4292D6);
}

void CUnicodeFont::InitColorWithGradient(int idx, int c1, int c2, int c3, int borderColor)
{
	m_colorBorders[idx] = RGB888TO565(borderColor);

	int r1 = ( c1 & 0xFF0000 ) >> 8;
	int g1 = ( c1 & 0x00FF00 ) >> 0;
	int b1 = ( c1 & 0x0000FF ) << 8;
	int r2 = ( c2 & 0xFF0000 ) >> 8;
	int g2 = ( c2 & 0x00FF00 ) >> 0;
	int b2 = ( c2 & 0x0000FF ) << 8;
	int r3 = ( c3 & 0xFF0000 ) >> 8;
	int g3 = ( c3 & 0x00FF00 ) >> 0;
	int b3 = ( c3 & 0x0000FF ) << 8;

	int dr1 = ( r2 - r1 ) * 2 / m_fontHeight;
	int dg1 = ( g2 - g1 ) * 2 / m_fontHeight;
	int db1 = ( b2 - b1 ) * 2 / m_fontHeight;
	int dr2 = ( r3 - r2 ) * 2 / m_fontHeight;
	int dg2 = ( g3 - g2 ) * 2 / m_fontHeight;
	int db2 = ( b3 - b2 ) * 2 / m_fontHeight;

	int cr = r1;
	int cg = g1;
	int cb = b1;
	
	int i = 0;
	for( ; i < (m_fontHeight >> 1); i++)
	{
		m_colorGradients[idx][i] = ((cr >> 0) & 0xF800) | ((cg >> 5) & 0x07E0) | ((cb >> 11) & 0x001F);
		cr += dr1;
		cg += dg1;
		cb += db1;
	}
	for( ; i < m_fontHeight; i++)
	{
		m_colorGradients[idx][i] = ((cr >> 0) & 0xF800) | ((cg >> 5) & 0x07E0) | ((cb >> 11) & 0x001F);
		cr += dr2;
		cg += dg2;
		cb += db2;
	}
}

