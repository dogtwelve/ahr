// Lib2D.cpp: implementation of the CLib2D class.
//
//////////////////////////////////////////////////////////////////////
#include "config.h"

#include "Lib2D.h"
#include "DevUtil.h"
#include "Fsqrt.h"
#include "HG/HighGear.h"
#include "HG/File.h"

//TEST
#include "sprites.h"


#include <string>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLib2D::CLib2D(unsigned short *in_DestPtr, unsigned short in_Width, unsigned short in_Height, const Lib3D::CLib3D& in_Lib3D) :
    m_pLib3D(&in_Lib3D),
	m_clipX(-1),
	m_clipY(-1),
	m_clipWidth(-1),
	m_clipHeight(-1),
	m_pHG(CHighGear::GetInstance()),
	m_dispX(m_pHG->m_dispX),
	m_dispY(m_pHG->m_dispY)
{
	//m_ClippingRects(0,0,in_Width,in_Height),

    SetDestPtr(in_DestPtr, in_Width, in_Height);

	//removed unused 
#ifdef USE_OGL
	m_backBuffer = NULL;
#else //USE_OGL
	m_backBuffer = new unsigned short[in_Width * in_Height];
#endif //USE_OGL

}

void CLib2D::DarkenScreen()
{
	unsigned short * m_screen =  GetDestPtr();
	//-------------
	//MIDDLE PART
	//-------------
	for (int i=0;i<m_dispX*(m_dispY);i++)
	{
		m_screen[i] = CLib2D::FastColorMix50_50(m_screen[i] , 0);

	}
}

CLib2D::~CLib2D()
{
	delete[] m_backBuffer;
}

void CLib2D::BlitBackBuffer()
{
//removed unused buffers
#ifndef USE_OGL
	memcpy(GetDestPtr(), m_backBuffer, (m_DestWidth * m_DestHeight) << 1);
#endif /* USE_OGL */
}

void CLib2D::CopyBufferTo(unsigned short *in_DestPtr) const
{
#ifndef USE_OGL
    memcpy(in_DestPtr, GetDestPtr(), (m_DestWidth*m_DestHeight)<<1);
#endif /* USE_OGL */
}

void CLib2D::SetDestPtr(unsigned short *in_DestPtr, unsigned short in_Width, unsigned short in_Height)
{
    m_DestPtr = in_DestPtr;
    m_DestWidth = in_Width;
    m_DestHeight = in_Height;

    while (!m_ClippingRects.empty()) 
		m_ClippingRects.pop();
    m_ClippingRects.push(ARect(0, 0, in_Width, in_Height));
}



void CLib2D::DrawPixel(int in_DstX, int in_DstY, short in_Color) const
{
	unsigned short nWidth = GetDestWidth();
	unsigned short nHeight = GetDestHeight();
	if (in_DstX >= 0 &&
		in_DstX < nWidth &&
		in_DstY >= 0 &&
		in_DstY < nHeight)
	{
		unsigned short *pDest = (unsigned short *)GetDestPtr();
		pDest[in_DstY*nWidth + in_DstX] = in_Color;
	}
}


void CLib2D::DrawLine(int in_SrcX, int in_SrcY, int in_DstX, int in_DstY, short in_Color) const
{
    int dy = in_DstY - in_SrcY;
    int dx = in_DstX - in_SrcX;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -GetDestWidth(); } else { stepy = GetDestWidth(); }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;

    unsigned short *dest = (unsigned short *)GetDestPtr();
    in_SrcY *= GetDestWidth();
    in_DstY *= GetDestWidth();
    const ARect& clipRect = GetClippingRect();
    const int MIN_Y = clipRect.y1 * GetDestWidth();
    const int MAX_Y = clipRect.y2 * GetDestWidth();
	// We don't verify the source point (there should not be any collision at the source).
    if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2 && 
        in_SrcY >= MIN_Y && in_SrcY < MAX_Y)
    {
        dest[in_SrcX+in_SrcY] = in_Color;
    }
    if (dx > dy)
    {
        int fraction = dy - (dx >> 1);
        while (in_SrcX != in_DstX)
        {
            if (fraction >= 0)
            {
                in_SrcY += stepy;
                fraction -= dx;
            }
            in_SrcX += stepx;
            fraction += dy;
            if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2
                && in_SrcY >= MIN_Y && in_SrcY < MAX_Y) 
            {
                dest[in_SrcX+in_SrcY] = in_Color;
            }
        }
    } 
    else
    {
        int fraction = dx - (dy >> 1);
        while (in_SrcY != in_DstY)
        {
            if (fraction >= 0)
            {
                in_SrcX += stepx;
                fraction -= dy;
            }
            in_SrcY += stepy;
            fraction += dx;
            if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2
                && in_SrcY >= MIN_Y && in_SrcY < MAX_Y) 
            {
                dest[in_SrcX+in_SrcY] = in_Color;
            }
        }
    }
}

void CLib2D::DrawAlphaColorLine(int in_SrcX, int in_SrcY, int in_DstX, int in_DstY, short in_Color) const
{
    int dy = in_DstY - in_SrcY;
    int dx = in_DstX - in_SrcX;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -GetDestWidth(); } else { stepy = GetDestWidth(); }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;

    unsigned short *dest = (unsigned short *)GetDestPtr();
    in_SrcY *= GetDestWidth();
    in_DstY *= GetDestWidth();
    const ARect& clipRect = GetClippingRect();
    const int MIN_Y = clipRect.y1 * GetDestWidth();
    const int MAX_Y = clipRect.y2 * GetDestWidth();
	unsigned short * dest2;
	// We don't verify the source point (there should not be any collision at the source).
    if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2 && 
        in_SrcY >= MIN_Y && in_SrcY < MAX_Y)
    {
			dest2=dest+in_SrcX+in_SrcY;
            *dest2 = MixColor(in_Color&0x0fff,*dest2&0x0fff,(in_Color>>12)&0xf);
    }
    if (dx > dy)
    {
        int fraction = dy - (dx >> 1);
        while (in_SrcX != in_DstX)
        {
            if (fraction >= 0)
            {
                in_SrcY += stepy;
                fraction -= dx;
            }
            in_SrcX += stepx;
            fraction += dy;
            if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2
                && in_SrcY >= MIN_Y && in_SrcY < MAX_Y) 
            {
				dest2=dest+in_SrcX+in_SrcY;
                *dest2 = MixColor(in_Color&0x0fff,*dest2&0x0fff,(in_Color>>12)&0xf);
            }
        }
    } 
    else
    {
        int fraction = dx - (dy >> 1);
        while (in_SrcY != in_DstY)
        {
            if (fraction >= 0)
            {
                in_SrcX += stepx;
                fraction -= dy;
            }
            in_SrcY += stepy;
            fraction += dx;
            if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2
                && in_SrcY >= MIN_Y && in_SrcY < MAX_Y) 
            {
				dest2=dest+in_SrcX+in_SrcY;
                *dest2 = MixColor(in_Color&0x0fff,*dest2&0x0fff,(in_Color>>12)&0xf);
            }
        }
    }
}


void CLib2D::DrawDotLine(int in_SrcX, int in_SrcY, int in_DstX, int in_DstY, unsigned char in_nPattern, short in_Color) const
{
    int dy = in_DstY - in_SrcY;
    int dx = in_DstX - in_SrcX;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -GetDestWidth(); } else { stepy = GetDestWidth(); }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;

    int nCurrentPatternStep = 0;

    unsigned short *dest = (unsigned short *)GetDestPtr();
    in_SrcY *= GetDestWidth();
    in_DstY *= GetDestWidth();
    const ARect& clipRect = GetClippingRect();
    const int MIN_Y = clipRect.y1 * GetDestWidth();
    const int MAX_Y = clipRect.y2 * GetDestWidth();
	// We don't verify the source point (there should not be any collision at the source).
    if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2 && 
        in_SrcY >= MIN_Y && in_SrcY < MAX_Y)
    {
        if (in_nPattern&(1<<nCurrentPatternStep))
            dest[in_SrcX+in_SrcY] = in_Color;
        nCurrentPatternStep = (nCurrentPatternStep+1)&0x07;
    }
    if (dx > dy)
    {
        int fraction = dy - (dx >> 1);
        while (in_SrcX != in_DstX)
        {
            if (fraction >= 0)
            {
                in_SrcY += stepy;
                fraction -= dx;
            }
            in_SrcX += stepx;
            fraction += dy;
            if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2
                && in_SrcY >= MIN_Y && in_SrcY < MAX_Y) 
            {
                if (in_nPattern&(1<<nCurrentPatternStep))
                    dest[in_SrcX+in_SrcY] = in_Color;
                nCurrentPatternStep = (nCurrentPatternStep+1)&0x07;
            }
        }
    } 
    else
    {
        int fraction = dx - (dy >> 1);
        while (in_SrcY != in_DstY)
        {
            if (fraction >= 0)
            {
                in_SrcX += stepx;
                fraction -= dy;
            }
            in_SrcY += stepy;
            fraction += dx;
            if (in_SrcX >= clipRect.x1 && in_SrcX < clipRect.x2
                && in_SrcY >= MIN_Y && in_SrcY < MAX_Y) 
            {
                if (in_nPattern&(1<<nCurrentPatternStep))
                    dest[in_SrcX+in_SrcY] = in_Color;
                nCurrentPatternStep = (nCurrentPatternStep+1)&0x07;
            }
        }
    }
}

void CLib2D::DrawRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, unsigned short in_FillColor, unsigned short in_EdgeColor) const
{
#ifdef USE_OGL

	short x1 = in_nX, y1 = in_nY, x2 = in_nX + in_nWidth - 1, y2 = in_nY + in_nHeight - 1;
	
	if (x1 < m_clipX)
		x1 = m_clipX;
	
	if (y1 < m_clipY)
		y1 = m_clipY;
	
	if (x2 >= m_clipX + m_clipWidth)
		x2 = m_clipX + m_clipWidth - 1;
	
	if (y2 >= m_clipY + m_clipHeight)
		y2 = m_clipY + m_clipHeight - 1;
	

	int color = CONVERT_4444_TO_8888(in_FillColor);
	
	g_lib3DGL->fillRect(x1, y1, x2 - x1 + 1 , y2 - y1 + 1, color);
	
	if ((in_EdgeColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(in_EdgeColor);
		
		if (in_nY >= m_clipY)
			g_lib3DGL->drawLine(x1, y1, x2, y1, color);
			
		if (in_nY + in_nHeight - 1 < m_clipY + m_clipHeight)
			g_lib3DGL->drawLine(x1, y2, x2, y2, color);
		
		if (in_nX >= m_clipX)
			g_lib3DGL->drawLine(x1, y1, x1, y2, color);
		
		if (in_nX + in_nWidth - 1 < m_clipX + m_clipWidth)
			g_lib3DGL->drawLine(x2, y1, x2, y2, color);
	}

#else // OGL

    int nX2 = in_nX + in_nWidth;
    int nY2 = in_nY + in_nHeight;

    A_ASSERT(in_nX >= 0 && in_nY >= 0 && nX2 >= 0 && nY2 >= 0);
    A_ASSERT(in_nX < GetDestWidth() && in_nY < GetDestHeight() && nX2 <= GetDestWidth() && nY2 <= GetDestHeight());
    A_ASSERT(in_nX <= nX2 && in_nY <= nY2);

	A_ASSERT(nX2 <= m_dispX);
	A_ASSERT(nY2 <= m_dispY);

    unsigned short *dest = (unsigned short *)GetDestPtr();

    if (in_EdgeColor == COLOR_TRANSPARENT)
    {
        in_nY *= GetDestWidth();
        nY2 *= GetDestWidth();

        for (; in_nY < nY2; in_nY += GetDestWidth())
        {
            for (int x=in_nX; x < nX2; ++x)
            {
                dest[x+in_nY] = in_FillColor;
            }
        }
    }
    else
    {
        for (int y = in_nY; y < nY2; ++y)
        {
            for (int x = in_nX; x < nX2; ++x)
            {
                if (x == in_nX || x == nX2-1 || y == in_nY || y == nY2-1)
                {
                    dest[x + y*GetDestWidth()] = in_EdgeColor;
                }
                else
                {
                    if (in_FillColor != COLOR_TRANSPARENT)
                    {
                        dest[x + y*GetDestWidth()] = in_FillColor;
                    }
                } 
            }
        }
    }

#endif /* USE_OGL */
}

void CLib2D::DrawAlphaRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, short in_FillColor, int in_nColorWeight) const
{
#ifdef USE_OGL
	// TODO: take in_nColorWeight into account
	int color = 0xA0000000 | CONVERT_565_TO_888(in_FillColor);

	g_lib3DGL->fillRect(in_nX, in_nY, in_nWidth, in_nHeight, color);
	
#else // OGL

	if (in_nX >= m_dispX) return;
	if (in_nY >= m_dispY) return;
	if (in_nX<0) 
	{
		in_nWidth += in_nX;
		in_nX=0;
	}
	if (in_nY<0) 
	{
		in_nHeight += in_nY;
		in_nY=0;
	}
	if (in_nX + in_nWidth > m_dispX )
	{
		in_nWidth = m_dispX - in_nX;
	}
	if (in_nY + in_nHeight > m_dispY )
	{
		in_nHeight = m_dispY - in_nY;
	}

	if (in_nWidth<0) 
	{
		return;
	}
	if (in_nHeight<0) 
	{
		return;
	}


    int nX2 = in_nX + in_nWidth;
    int nY2 = in_nY + in_nHeight;

    A_ASSERT(in_nX >= 0 && in_nY >= 0 && nX2 >= 0 && nY2 >= 0);
    A_ASSERT(in_nX < GetDestWidth() && in_nY < GetDestHeight() && nX2 <= GetDestWidth() && nY2 <= GetDestHeight());
    A_ASSERT(in_nX <= nX2 && in_nY <= nY2);

    unsigned short *dest = (unsigned short *)GetDestPtr();

    in_nY *= GetDestWidth();
    nY2 *= GetDestWidth();

    for (; in_nY < nY2; in_nY += GetDestWidth())
    {
        for (int x=in_nX; x < nX2; ++x)
        {
            unsigned short& nDest= dest[x+in_nY];
            nDest = MixColor(in_FillColor, nDest, in_nColorWeight);
        }
    }

#endif /* USE_OGL */
}


void CLib2D::DrawAlphaColorRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, short in_FillColor, short in_BorderColor) const
{
#ifdef USE_OGL

	int color = /*0xFF000000 |*/ CONVERT_4444_TO_8888(in_FillColor);

	g_lib3DGL->fillRect(in_nX, in_nY, in_nWidth, in_nHeight, color);

#else // OGL

	ARect srcRect(in_nX,in_nY,in_nX+in_nWidth,in_nY+in_nHeight);
	ARect screenRect(0,0,GetDestWidth(),GetDestHeight());

	srcRect=ARect::Intersect(srcRect,screenRect);
	
	in_nX=srcRect.x1;
	in_nY=srcRect.y1;
	in_nWidth=srcRect.width;
	in_nHeight=srcRect.height;


    int nX2 = in_nX + in_nWidth;
    int nY2 = in_nY + in_nHeight;

    A_ASSERT(in_nX >= 0 && in_nY >= 0 && nX2 >= 0 && nY2 >= 0);
    A_ASSERT(in_nX < GetDestWidth() && in_nY < GetDestHeight() && nX2 <= GetDestWidth() && nY2 <= GetDestHeight());
    A_ASSERT(in_nX <= nX2 && in_nY <= nY2);

    unsigned short *dest = (unsigned short *)GetDestPtr();

    in_nY *= GetDestWidth();
	nY2--;
    nY2 *= GetDestWidth();

	short color=in_BorderColor;
	int x;
	// Top border
	for (x=in_nX; x < nX2; ++x)
	{
		unsigned short& nDest= dest[x+in_nY];
#ifdef __565_RENDERING__
		nDest = MixColorBrew444(color&0x0fff, nDest, (in_BorderColor>>12)&0xf);
#else
		nDest = MixColor(color&0x0fff, nDest, (in_BorderColor>>12)&0xf);
#endif
	}


	in_nY+=GetDestWidth();
	
	// Middle section
	for (; in_nY < nY2; in_nY += GetDestWidth())
    {
		color=in_BorderColor;
        for (x=in_nX; x < nX2-1; ++x)
        {
            unsigned short& nDest= dest[x+in_nY];
#ifdef __565_RENDERING__
            nDest = MixColorBrew444(color&0x0fff, nDest, (color>>12)&0xf);
#else
            nDest = MixColor(color&0x0fff, nDest, (color>>12)&0xf);
#endif
			color=in_FillColor;
        }
        unsigned short& nDest= dest[x+in_nY];
#ifdef __565_RENDERING__
        nDest = MixColorBrew444(in_BorderColor&0x0fff, nDest, (in_BorderColor>>12)&0xf);
#else
        nDest = MixColor(in_BorderColor&0x0fff, nDest, (in_BorderColor>>12)&0xf);
#endif
    }

	// Bottom border
    for (x=in_nX; x < nX2; ++x)
    {
        unsigned short& nDest= dest[x+in_nY];
#ifdef __565_RENDERING__
        nDest = MixColorBrew444(in_BorderColor&0x0fff, nDest, (in_BorderColor>>12)&0xf);
#else
        nDest = MixColor(in_BorderColor&0x0fff, nDest, (in_BorderColor>>12)&0xf);
#endif
    }

#endif /* USE_OGL */

}

void CLib2D::DrawString(int intX, int intY, const unsigned short* chrpString, unsigned short in_Color, short shtFontType, short shtAlignment) const
{
    //A_ASSERT(intX >= 0 && intY >= 0);
    //A_ASSERT(intX < GetDestWidth() && intY < GetDestHeight());


    //const CBitFont* bfopFont = 0;

    //if(shtFontType == FONT_SMALL)
    //    bfopFont = m_bfopSmallFont;

    //A_ASSERT(bfopFont && "Unsupported font");


    //unsigned short *dest = (unsigned short *)GetDestPtr();
    //intY *= GetDestWidth();

    //int k;

    //if(shtAlignment == TEXT_ALIGN_RIGHT)
    //{
    //    for(k = 0; chrpString[k] != 0; k++)
    //    {
    //        intX -= bfopFont->GetCharacterWidth(chrpString[k]);
    //    }
    //}
    //else if(shtAlignment == TEXT_ALIGN_CENTER)
    //{
    //    int intTotalWidth = 0;

    //    for(k = 0; chrpString[k] != 0; k++)
    //    {
    //        intTotalWidth += bfopFont->GetCharacterWidth(chrpString[k]);
    //    }

    //    intX -= (intTotalWidth / 2);
    //}
    ////else LEFT is ok like it is

    //unsigned short shtuWidth;
    //unsigned short shtuHeight;
    //unsigned short shtuLines;

    //for(k = 0; chrpString[k] != 0; k++)
    //{

    //    shtuLines = bfopFont->GetCharacterAttribs(chrpString[k], shtuWidth, shtuHeight);
    //    int intYMod = intY;
 
    //    for(int i = 0; i < shtuLines; i++, intYMod += GetDestWidth())
    //    {
    //        int intXMod = intX;
    //        for(int j = 128; j > (128 >> shtuWidth); j = j >> 1, intXMod++)
    //        {
    //            if(((bfopFont->GetCharacterLine(chrpString[k], i)) & j) != 0)
    //            {
    //                dest[intXMod + intYMod] = in_Color;
    //            }
    //        }
    //    }

    //    intX += shtuWidth;
    //}

	//TEST
//#ifdef USE_FANCY_MENUS
//	CFontWrapper *font = MenuContainer::GetInstance()->m_FontNormal;
//#else // USE_FANCY_MENUS
//	CFontWrapper *font = DemoMenu::GetInstance()->m_FontNormal;//m_FontSmall;
//#endif // USE_FANCY_MENUS
//
//	if (font)
//		font->DrawString((CLib2D &)*this, intX, intY, chrpString, shtAlignment);
}

//int CLib2D::GetStringWidth(const char* chrpString, short shtFontType) const
//{
//    const CBitFont* bfopFont = 0;
//
//    if(shtFontType == FONT_SMALL)
//        bfopFont = m_bfopSmallFont;
//
//    A_ASSERT(bfopFont && "Unsupported font");
//
//    int intTotalWidth = 0;
//
//    for(int k = 0; chrpString[k] != 0; k++)
//    {
//        intTotalWidth += bfopFont->GetCharacterWidth(chrpString[k]);
//    }
//
//    return intTotalWidth;
//}

//const char * CLib2D::TruncateString(char * io_pString, int in_nWidth) const
//{
//    std::string strFinal = io_pString;
//
//    if (GetStringWidth(strFinal.c_str()) > in_nWidth)
//    {
//        const int k_nSacrificesWidth = GetStringWidth("...");
//        while(GetStringWidth(strFinal.c_str()) + k_nSacrificesWidth > in_nWidth)
//        {
//            // Remove last character
//            std::string::reverse_iterator it = strFinal.rbegin();
//            while(it != strFinal.rend() && *it == '\0') ++it;
//            if (it == strFinal.rend())
//                break;
//            *it = '\0';
//        }
//        
//        // Append "..."
//        // the std::string::append() function doesnt work because of the previous code
//        std::string::iterator it = strFinal.begin();
//        int nNumPoints = 0;
//        while(nNumPoints < 3 && it != strFinal.end())
//        {
//            if (*it == '\0')
//            {
//                nNumPoints++;
//                *it = '.';
//            }
//            ++it;
//        }
//    }
//
//	strcpy(io_pString, strFinal.c_str());
//    return io_pString;
//}

void CLib2D::DrawArc(int in_nX, int in_nY, int in_nRay, int in_nAngleBegin, int in_nAngleEnd, short in_FillColor, short in_EdgeColor) const
{
#ifdef USE_OGL

	int color;
	if ((in_FillColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(in_FillColor);
		g_lib3DGL->fillArc(in_nX, in_nY, in_nRay, in_nAngleBegin, in_nAngleEnd, color);
	}
	
	if ((in_EdgeColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(in_EdgeColor);
		g_lib3DGL->drawArc(in_nX, in_nY, in_nRay, in_nAngleBegin, in_nAngleEnd, color);
	}

#else /* USE_OGL */

    int nX1 = in_nX - in_nRay;
    int nX2 = in_nX + in_nRay;
    int nY1 = (in_nY - in_nRay);
    int nY2 = (in_nY + in_nRay);
    unsigned short *dest = (unsigned short *)GetDestPtr();

    if (in_EdgeColor == COLOR_TRANSPARENT)
    {
        for (int y = nY1; y < nY2; y ++)
        {
            for (int x = nX1; x < nX2; ++x)
            {
                int nOffsetX = (x-in_nX);
                int nOffsetY = -(y-in_nY);
                int nAngle = Atan2i(nOffsetX, nOffsetY);
                if (in_nAngleBegin <= nAngle && nAngle <= in_nAngleEnd)
                {
                    int nRay2 = in_nRay*in_nRay;
                    int nDist2 = nOffsetX*nOffsetX + nOffsetY*nOffsetY;

                    if (nDist2 < nRay2)
                    {
                        // Fill all arc
                        dest[x + y*GetDestWidth()] = in_FillColor;
                    }
                }
            }
        }
    }
    else
    {
        for (int y = nY1; y < nY2; y ++)
        {
            for (int x = nX1; x < nX2; ++x)
            {
                int nOffsetX = (x-in_nX);
                int nOffsetY = -(y-in_nY);
                int nAngle = Atan2i(nOffsetX, nOffsetY);
                if (in_nAngleBegin <= nAngle && nAngle <= in_nAngleEnd)
                {
                    int nRay2 = in_nRay*in_nRay;
                    int nDist2 = nOffsetX*nOffsetX + nOffsetY*nOffsetY;

                    if (nDist2 < nRay2)
                    {
                         // Check edge
                        if (Lib3D::FSqrt(nDist2) < Lib3D::FSqrt(nRay2)-1)
                        {
                            // Inner arc
                            if (in_FillColor != COLOR_TRANSPARENT)
                            {
                                dest[x + y*GetDestWidth()] = in_FillColor;
                            }
                        }
                        else
                        {
                            // Edge
                            dest[x + y*GetDestWidth()] = in_EdgeColor;
                        }
                    }
                }
            }
        }
    }
#endif /* USE_OGL */
}

void CLib2D::DrawRoundRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nRay, short nFillColor, short nBorderColor/*, int nBorderWidth*/ ) const
{
#ifdef USE_OGL

	bool cliped = false;
	if (in_nX < m_clipX || in_nY < m_clipY || m_clipX + m_clipWidth < in_nX + in_nWidth || m_clipY + m_clipHeight < in_nY + in_nHeight)
	{
		g_lib3DGL->setClip(m_clipX, m_clipY, m_clipWidth, m_clipHeight);
		cliped = true;
	}

	//TEST
	CSprite *spr;
//	CSprite *spr = MenuContainer::GetInstance()->m_menuButtons;
	
	int color;
	if ((nFillColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(nFillColor);
		
		g_lib3DGL->fillRect(in_nX, in_nY + in_nRay, in_nWidth, in_nHeight - 2 * in_nRay, color);
		g_lib3DGL->fillRect(in_nX + in_nRay, in_nY, in_nWidth - 2 * in_nRay, in_nRay, color);
		g_lib3DGL->fillRect(in_nX + in_nRay, in_nY + in_nHeight - in_nRay, in_nWidth - 2 * in_nRay, in_nRay, color);
		
		//TEST
//		if (!spr)
		{
			g_lib3DGL->fillArc(in_nX + in_nRay, in_nY + in_nRay, in_nRay, ANGLEPI, 3 * ANGLEPI / 2, color);
			g_lib3DGL->fillArc(in_nX + in_nWidth - in_nRay, in_nY + in_nRay, in_nRay, 3 * ANGLEPI / 2, 0, color);
			g_lib3DGL->fillArc(in_nX + in_nRay, in_nY + in_nHeight - in_nRay, in_nRay, ANGLEPI / 2, ANGLEPI, color);
			g_lib3DGL->fillArc(in_nX + in_nWidth - in_nRay, in_nY + in_nHeight - in_nRay, in_nRay, 0, ANGLEPI / 2, color);
		}
	}
	
	if ((nBorderColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(nBorderColor);
		
		g_lib3DGL->drawLine(in_nX + in_nRay, in_nY, in_nX + in_nWidth - in_nRay, in_nY, color);
		g_lib3DGL->drawLine(in_nX + in_nRay, in_nY + in_nHeight, in_nX + in_nWidth - in_nRay, in_nY + in_nHeight, color);
		g_lib3DGL->drawLine(in_nX, in_nY + in_nRay, in_nX, in_nY + in_nHeight - in_nRay, color);
		g_lib3DGL->drawLine(in_nX + in_nWidth, in_nY + in_nRay, in_nX + in_nWidth, in_nY + in_nHeight - in_nRay, color);
		
		//TEST
//		if (!spr)
		{
			g_lib3DGL->drawArc(in_nX + in_nRay, in_nY + in_nRay, in_nRay, ANGLEPI, 3 * ANGLEPI / 2, color);
			g_lib3DGL->drawArc(in_nX + in_nWidth - in_nRay, in_nY + in_nRay, in_nRay, 3 * ANGLEPI / 2, 0, color);
			g_lib3DGL->drawArc(in_nX + in_nRay, in_nY + in_nHeight - in_nRay, in_nRay, ANGLEPI / 2, ANGLEPI, color);
			g_lib3DGL->drawArc(in_nX + in_nWidth - in_nRay, in_nY + in_nHeight - in_nRay, in_nRay, 0, ANGLEPI / 2, color);
		}
	}

	//TEST
//	if (spr)
//	{
//		color = CONVERT_4444_TO_8888(nFillColor);
//		g_lib3DGL->setColor(color);
//
//		spr->DrawFrame((CLib2D&)(*this), in_nX, in_nY, MENU_BUTTONS_F_ROUND_EDGE);
//		spr->DrawFrame((CLib2D&)(*this), in_nX + in_nWidth - in_nRay, in_nY, MENU_BUTTONS_F_ROUND_EDGE, CSprite::FLAGS_FLIP_X);
//		spr->DrawFrame((CLib2D&)(*this), in_nX, in_nY + in_nHeight - in_nRay, MENU_BUTTONS_F_ROUND_EDGE, CSprite::FLAGS_FLIP_Y);
//		spr->DrawFrame((CLib2D&)(*this), in_nX + in_nWidth - in_nRay, in_nY + in_nHeight - in_nRay, MENU_BUTTONS_F_ROUND_EDGE, CSprite::FLAGS_FLIP_X | CSprite::FLAGS_FLIP_Y);
//
//		g_lib3DGL->setColor(); // reset the color
//	}
	
	if (cliped)
		g_lib3DGL->clearClip();

#else /* USE_OGL */
	
	unsigned short *dest = (unsigned short *)GetDestPtr();
	const ARect rectClip = GetClippingRect();

	//A_ASSERT(in_nWidth>=2*in_nRay);
	//A_ASSERT(in_nHeight>=2*in_nRay);
	
	int nFillColorWeight = (nFillColor>>12)&0xf;
	nFillColor &= 0x0fff;   
	int nBorderColorWeight = (nBorderColor>>12)&0xf;
	nBorderColor &= 0x0fff;
	
	const int B = nBorderWidth;

	for( int y=0; y<in_nHeight; y++ )
	{
		int Y = in_nY + y;
		if(Y<rectClip.y1)
			continue;
		else if(Y>=rectClip.y2)
			break;

		int dv = 0;
		if( y < in_nRay) 
			dv = in_nRay -Lib3D::FSqrt(in_nRay*in_nRay - (in_nRay-y)*(in_nRay-y)) ;
		else if( (in_nHeight-y) < in_nRay)
			dv =  in_nRay -Lib3D::FSqrt(in_nRay*in_nRay - (in_nRay-in_nHeight+y)*(in_nRay-in_nHeight+y) ) ;

		int xl = in_nX+dv;
		int xr = in_nX+in_nWidth-dv;
		
		// fill line
		for( int X = xl; X < xr; X++ )
		{	 
			if(X<rectClip.x1)
				continue;
			else if(X>=rectClip.x2)
				break;

			unsigned short& nDestColor= dest[X+Y*GetDestWidth()];
			if( B && (y < B || y>(in_nHeight-B) ||X < (xl+B) || X>(xr-B) ))
				nDestColor = MixColor(nBorderColor, nDestColor, nBorderColorWeight);
			else
				nDestColor = MixColor(nFillColor, nDestColor, nFillColorWeight);
		}

	}
#endif /* USE_OGL */
}

void CLib2D::DrawRoundRect(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nRayUp, int in_nRayDown, short nFillColor, short nBorderColor/*, int nBorderWidth*/ ) const
{
#ifdef USE_OGL

	bool cliped = false;
	if (in_nX < m_clipX || in_nY < m_clipY || m_clipX + m_clipWidth < in_nX + in_nWidth || m_clipY + m_clipHeight < in_nY + in_nHeight)
	{
		g_lib3DGL->setClip(m_clipX, m_clipY, m_clipWidth, m_clipHeight);
		cliped = true;
	}

	//TEST
	CSprite *spr;
//	CSprite *spr = MenuContainer::GetInstance()->m_menuButtons;
//	int frame = (nFillColor == (short)0xF000) ? MENU_BUTTONS_F_ROUND_BLACK : MENU_BUTTONS_F_ROUND_EDGE;
	
	int color;
	if ((nFillColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(nFillColor);
		
		g_lib3DGL->fillRect(in_nX, in_nY + in_nRayUp, in_nWidth, in_nHeight - in_nRayUp - in_nRayDown, color);
		g_lib3DGL->fillRect(in_nX + in_nRayUp, in_nY, in_nWidth - 2 * in_nRayUp, in_nRayUp, color);
		g_lib3DGL->fillRect(in_nX + in_nRayDown, in_nY + in_nHeight - in_nRayDown, in_nWidth - 2 * in_nRayDown, in_nRayDown, color);
		
		//TEST
//		if (spr)
//		{
//			//g_lib3DGL->setColor(color);
//
//			if (in_nRayUp > 0)
//			{
//				spr->DrawFrame((CLib2D&)(*this), in_nX, in_nY, frame);
//				spr->DrawFrame((CLib2D&)(*this), in_nX + in_nWidth - in_nRayUp, in_nY, frame, CSprite::FLAGS_FLIP_X);
//			}
//
//			if (in_nRayDown > 0)
//			{
//				spr->DrawFrame((CLib2D&)(*this), in_nX, in_nY + in_nHeight - in_nRayDown, frame, CSprite::FLAGS_FLIP_Y);
//				spr->DrawFrame((CLib2D&)(*this), in_nX + in_nWidth - in_nRayDown, in_nY + in_nHeight - in_nRayDown, frame, CSprite::FLAGS_FLIP_X | CSprite::FLAGS_FLIP_Y);
//			}
//
//			//g_lib3DGL->setColor();
//		}
//		else
		{
			if (in_nRayUp > 0)
			{
				g_lib3DGL->fillArc(in_nX + in_nRayUp, in_nY + in_nRayUp + 1, in_nRayUp, ANGLEPI, 3 * ANGLEPI / 2, color);
				g_lib3DGL->fillArc(in_nX + in_nWidth - in_nRayUp, in_nY + in_nRayUp + 1, in_nRayUp, 3 * ANGLEPI / 2, 0, color);
			}
			
			if (in_nRayDown > 0)
			{
				g_lib3DGL->fillArc(in_nX + in_nRayDown, in_nY + in_nHeight - in_nRayDown, in_nRayDown, ANGLEPI / 2, ANGLEPI, color);
				g_lib3DGL->fillArc(in_nX + in_nWidth - in_nRayDown, in_nY + in_nHeight - in_nRayDown, in_nRayDown, 0, ANGLEPI / 2, color);
			}
		}
	}
	
	if ((nBorderColor & 0xF000) != 0)
	{
		color = CONVERT_4444_TO_8888(nBorderColor);
		
		g_lib3DGL->drawLine(in_nX + in_nRayUp, in_nY, in_nX + in_nWidth - in_nRayUp, in_nY, color);
		g_lib3DGL->drawLine(in_nX, in_nY + in_nRayUp, in_nX, in_nY + in_nHeight - in_nRayDown, color);
		g_lib3DGL->drawLine(in_nX + in_nWidth, in_nY + in_nRayUp, in_nX + in_nWidth, in_nY + in_nHeight - in_nRayDown, color);
		g_lib3DGL->drawLine(in_nX + in_nRayDown, in_nY + in_nHeight, in_nX + in_nWidth - in_nRayDown, in_nY + in_nHeight, color);
		
		
		if (!spr)
		{
			if (in_nRayUp > 0)
			{
				g_lib3DGL->drawArc(in_nX + in_nRayUp, in_nY + in_nRayUp + 1, in_nRayUp, ANGLEPI, 3 * ANGLEPI / 2, color);
				g_lib3DGL->drawArc(in_nX + in_nWidth - in_nRayUp, in_nY + in_nRayUp + 1, in_nRayUp, 3 * ANGLEPI / 2, 0, color);
			}
			
			if (in_nRayDown > 0)
			{
				g_lib3DGL->drawArc(in_nX + in_nRayDown, in_nY + in_nHeight - in_nRayDown, in_nRayDown, ANGLEPI / 2, ANGLEPI, color);
				g_lib3DGL->drawArc(in_nX + in_nWidth - in_nRayDown, in_nY + in_nHeight - in_nRayDown, in_nRayDown, 0, ANGLEPI / 2, color);
			}
		}
	}
	
	if (cliped)
		g_lib3DGL->clearClip();

#endif /* USE_OGL */
}

void CLib2D::DrawArrow(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nAngle, short in_FillColor) const
{
    int nTriangleWidth = 0;
    switch(in_nAngle)
    {
    case k_nArrowUp:
        nTriangleWidth = (in_nWidth>>1) + (in_nWidth&1);
        DrawRectTriangle(in_nX, in_nY, nTriangleWidth, in_nHeight, k_nBottomRight, in_FillColor);
        DrawRectTriangle(in_nX+(in_nWidth>>1), in_nY, nTriangleWidth, in_nHeight, k_nBottomLeft, in_FillColor);
        break;
    case k_nArrowDown:
        nTriangleWidth = (in_nWidth>>1) + (in_nWidth&1);
        DrawRectTriangle(in_nX, in_nY, nTriangleWidth, in_nHeight, k_nTopRight, in_FillColor);
        DrawRectTriangle(in_nX+(in_nWidth>>1), in_nY, nTriangleWidth, in_nHeight, k_nTopLeft, in_FillColor);
        break;
    case k_nArrowLeft:
        nTriangleWidth = (in_nHeight>>1) + (in_nHeight&1);
        DrawRectTriangle(in_nX, in_nY, in_nWidth, nTriangleWidth, k_nBottomRight, in_FillColor);
        DrawRectTriangle(in_nX, in_nY+(in_nHeight>>1), in_nWidth, nTriangleWidth, k_nTopRight, in_FillColor);
        break;
    case k_nArrowRight:
        nTriangleWidth = (in_nHeight>>1) + (in_nHeight&1);
        DrawRectTriangle(in_nX, in_nY, in_nWidth, nTriangleWidth, k_nBottomLeft, in_FillColor);
        DrawRectTriangle(in_nX, in_nY+(in_nHeight>>1), in_nWidth, nTriangleWidth, k_nTopLeft, in_FillColor);
        break;
    default:
        break;
    }
}

void CLib2D::DrawRectTriangle(int in_nX, int in_nY, int in_nWidth, int in_nHeight, int in_nCorner, short in_FillColor) const
{
    A_ASSERT(0 <= in_nX && in_nX+in_nWidth <= GetDestWidth());
    A_ASSERT(0 <= in_nY && in_nY+in_nHeight <= GetDestHeight());

    unsigned short *dest = (unsigned short *)GetDestPtr();

    for (int nY = 0; nY < in_nHeight; ++nY)
    {
        // 0 : bottom-left
        // 1 : top-left
        // 2 : bottom-right
        // 3 : top-right

        // Bottom left corner
        int nX1 = 0;
        int nX2 = nY*in_nWidth/in_nHeight+1;

        // Swap for top
        if (in_nCorner&1) nX2 = (in_nHeight-nY-1)*in_nWidth/in_nHeight+1;

        // Swap for right
        if (in_nCorner&2)
        {
            nX1 = (in_nWidth-nX2);
            nX2 = in_nWidth;
        }

        for (int x = nX1; x < nX2; ++x)
        {
            dest[x+in_nX+(in_nY+nY)*GetDestWidth()] = in_FillColor;
        }
    }
}

void CLib2D::DrawHTriangle( int nXbl, int nXbr,int nYb, int nXt, int nYt, short nFillColor, short nBorderColor) const
{				 
    unsigned short *dest = (unsigned short *)GetDestPtr();

	int nH = Abs( nYb - nYt );
	if (!nH) return;	// nothing to be drawn; let's avoid the divide by zero then
	int nYDir = nYb>nYt? -1 : +1; 

	int nFillColorWeight = (nFillColor>>12)&0xf;
	nFillColor &= 0x0fff;   
	int nBorderColorWeight = (nBorderColor>>12)&0xf;
	nBorderColor &= 0x0fff;
	
	const int kPrecision = 100000;
	int nXlm = (nXt-nXbl)*kPrecision/(nYt - nYb);
	int nXrm = (nXt-nXbr)*kPrecision/(nYt - nYb);

	for( int y=0; y<nH; y++ )
	{
		int Y = nYb + y*nYDir;
		int xl = (nXlm * y * nYDir) / kPrecision + nXbl;
		int xr = (nXrm * y * nYDir) / kPrecision + nXbr;

		// fill line
		for( int X = xl; X < xr; X++ )
		{	 
            unsigned short& nDestColor= dest[X+Y*GetDestWidth()];
            nDestColor = MixColor(nFillColor, nDestColor, nFillColorWeight);
		}

		//draw border
		if(nBorderColorWeight)
		{
			if( y == 0 )
			{
				for( int X = xl; X < xr; X++ )
				{	 
					unsigned short& nDestColor= dest[X+Y*GetDestWidth()];
					nDestColor = MixColor(nBorderColor, nDestColor, nBorderColorWeight);
				} 
			}
			else
			{
				unsigned short& nDestColorL= dest[xl+Y*GetDestWidth()];
				nDestColorL = MixColor(nBorderColor, nDestColorL, nBorderColorWeight);
				unsigned short& nDestColorR= dest[xr+Y*GetDestWidth()];
				nDestColorR = MixColor(nBorderColor, nDestColorR, nBorderColorWeight);
			}
		}
	}
}

//void CLib2D::Attenuate(unsigned char in_nIntensity) const
//{
//    Attenuate(0, 0, GetDestWidth(), GetDestHeight(), in_nIntensity);
//}
//
//void CLib2D::Attenuate(int in_nX, int in_nY, int in_nWidth, int in_nHeight, unsigned char in_nIntensity) const
//{
//    A_ASSERT(0 <= in_nX && in_nX+in_nWidth <= GetDestWidth());
//    A_ASSERT(0 <= in_nY && in_nY+in_nHeight <= GetDestHeight());
//
//    unsigned short *dest = (unsigned short *)GetDestPtr();
//    int nEndX = in_nX + in_nWidth;
//    int nEndY = (in_nY+in_nHeight)*GetDestWidth();
//    for(int y = in_nY*GetDestWidth(); y < nEndY; y += GetDestWidth())
//    {
//        for(int x = in_nX; x < nEndX; x++)
//        {
//            int nPixelIndex = x+y;
//            unsigned short nSrcPixel = dest[nPixelIndex];
//            unsigned short nDesPixel = 0;
//            nDesPixel |= ((((nSrcPixel & 0xF00)>>8)*in_nIntensity)>>8)<<8;
//            nDesPixel |= ((((nSrcPixel & 0x0F0)>>4)*in_nIntensity)>>8)<<4;
//            nDesPixel |= ((((nSrcPixel & 0x00F)>>0)*in_nIntensity)>>8)<<0;
//            dest[nPixelIndex] = nDesPixel;
//        }
//    }
//}

void CLib2D::Clear()
{
    memset(GetDestPtr(), 0, (GetDestWidth()*GetDestHeight())<<1);
}

void CLib2D::setColor(u32 color)
{
	g_lib3DGL->setColor(color);
}
