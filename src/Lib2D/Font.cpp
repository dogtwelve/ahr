#include "HG/HighGear.h"
#include "devutil.h"
#include "Font.h"
#include "File.h"
#include "lib2d/Lib2d.h"

//TEST
#include "util.h"

#include <string.h> // for strcat/strcpy
#include <string> 

#ifdef __565_RENDERING__
	#define __BREW__
#endif

///////////////////////////////////////////////////////////
// digit font


DigitFont::DigitFont(const char* font_bitmap, unsigned nb_digits) :
	m_dispX(CHighGear::GetInstance()->m_dispX),
	m_dispY(CHighGear::GetInstance()->m_dispY)
{
//	bitmap		= NEW Image(font_bitmap);
	texture		= NEW TTexture(font_bitmap, MASK_TEXTURE_NEAREST_FREE_BUFFER); 
	font_width	= texture->SizeX() / nb_digits;
	font_height	= texture->SizeY();
	m_pCharMap	= 0;
	m_iSpacing	= 0;
}

DigitFont::~DigitFont()
{
	MM_DELETE texture;

	if(m_pCharMap)
		MM_DELETE m_pCharMap;
}
	
void DigitFont::LoadCharMap( const unsigned char * sCharOrder, unsigned char iDefaultIndex )
{
	m_pCharMap = NEW unsigned char[256];
	memset( m_pCharMap, iDefaultIndex, 256);
	for( unsigned char  i = 0; (i < 256) && (sCharOrder[i]); i ++)
	{
		m_pCharMap[ sCharOrder[i] ] = i;
	}
}
	
void DigitFont::draw_digit(Image& buffer, unsigned char digit,
						   unsigned x, unsigned y)
{
	// [FIXME] Currently clipping the entire letter
	if (x < m_dispX-font_width)
	{
#ifdef USE_OGL

		f32 uv[4];

		uv[0] = (f32)(digit * font_width) / texture->m_pow2Width;
		uv[1] = (f32)texture->SizeY() / texture->m_pow2Height;
		uv[2] = (f32)((digit + 1) * font_width) / texture->m_pow2Width;
		uv[3] = 0.0f;

		g_lib3DGL->paint2DModule(x, y, font_width, font_height, texture->m_glTextureName, uv, 0);

#else /* USE_OGL */

		pixel_type*	dest = &buffer(x, y + texture->SizeY() - 1);
		pixel_type*	src  = texture->Data() + digit * font_width;
		unsigned	src_stride  = texture->SizeX() - font_width;
		unsigned	dest_stride = buffer.width()  + font_width;

		unsigned j = texture->SizeY();
		while (j--)
		{
			unsigned len = font_width;
			while (len--)
			{
				if (*src)
					*dest = *src;
				++dest;
				++src;
			}
			src  += src_stride;
			dest -= dest_stride;
		}

#endif /* USE_OGL */
	}
}


void DigitFont::draw_digit_shine(	Image& buffer, 
									unsigned char digit, 
									unsigned x, 
									unsigned y,
									int shine_pos,
									int shine_width,
									int shine_shift)
{
#ifdef USE_OGL

	f32 uv[4];

	uv[0] = (f32)(digit * font_width) / texture->m_pow2Width;
	uv[1] = (f32)texture->SizeY() / texture->m_pow2Height;
	uv[2] = (f32)((digit + 1) * font_width) / texture->m_pow2Width;
	uv[3] = 0.0f;

	g_lib3DGL->paint2DModule(x, y, font_width, font_height, texture->m_glTextureName, uv, 0);

#else /* USE_OGL */

	pixel_type*	dest = &buffer(x, y + texture->SizeY() - 1);
	pixel_type*	src  = texture->Data() + digit * font_width;
	unsigned	src_stride  = texture->SizeX() - font_width;
	unsigned	dest_stride = buffer.width()  + font_width;

	int shine_start = shine_pos - x;
	int shine_end = shine_start + shine_width;

	unsigned j = texture->SizeY();
	while (j--)
	{
		unsigned len = font_width;
		int shine_count = 0;
		while (len--)
		{
			if (*src)
			{
				if(shine_count > shine_start && shine_count < shine_end)
#ifdef __BREW__
					*dest = CLib2D::FastColorAdd(*src,0xFFFF);	// TBD
#else
					*dest = CLib2D::FastColorAdd(*src,0xCCC);
#endif
				else
					*dest = *src;
			}
			++dest;
			++src;
			shine_count++;
		}
		src  += src_stride;
		dest -= dest_stride;
		shine_start -= shine_shift;
		shine_end -= shine_shift;
	}

#endif /* USE_OGL */
}

void DigitFont::draw_digit_additive(Image& buffer, unsigned char digit,
									unsigned x, unsigned y)
{
#ifdef USE_OGL

	f32 uv[4];

	uv[0] = (f32)(digit * font_width) / texture->m_pow2Width;
	uv[1] = (f32)texture->SizeY() / texture->m_pow2Height;
	uv[2] = (f32)((digit + 1) * font_width) / texture->m_pow2Width;
	uv[3] = 0.0f;

	g_lib3DGL->paint2DModule(x, y, font_width, font_height, texture->m_glTextureName, uv, 0);

#else /* USE_OGL */

	pixel_type*	dest = &buffer(x, y + texture->SizeY() - 1);
	pixel_type*	src  = texture->Data() + digit * font_width;
	unsigned	src_stride  = texture->SizeX() - font_width;
	unsigned	dest_stride = buffer.width()  + font_width;

	unsigned j = texture->SizeY();
	if(j)
	do
	{
		unsigned len = font_width;
		if(len)
		do 
		{
			*dest++ = CLib2D::FastColorAdd(*dest,*src++);
		}
		while (--len);
		src  += src_stride;
		dest -= dest_stride;
	}
	while (--j);

#endif /* USE_OGL */
}


void DigitFont::draw_digit_mix(Image& buffer, unsigned char digit,unsigned x, unsigned y,int mixfactor)
{

#ifdef USE_OGL

	f32 uv[4];

	uv[0] = (f32)(digit * font_width) / texture->m_pow2Width;
	uv[1] = (f32) texture->SizeY() / texture->m_pow2Height;
	uv[2] = (f32)((digit + 1) * font_width) / texture->m_pow2Width;
	uv[3] = 0.0f;

	const int color = 0xFFFFFF | (Util::Interp(0xA, 0xF, 1, mixfactor, 7) << (24 + 4));
	g_lib3DGL->paint2DModule(x, y, font_width, font_height, texture->m_glTextureName, uv, 0, color );
	
#else /* USE_OGL */

	pixel_type*	dest = &buffer(x, y + texture->SizeY() - 1);
	pixel_type*	src  = texture->Data() + digit * font_width;

	// fix bug #1701454
	unsigned	clipped_width = font_width;
	if (x + clipped_width > buffer.width())
		clipped_width = buffer.width() - x;

	unsigned	src_stride  = texture->SizeX() - clipped_width;
	unsigned	dest_stride = buffer.width()  + clipped_width;
	unsigned len;	

	unsigned j = texture->SizeY();
	if(j)
	{
		switch(mixfactor)
		{
			case 1:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = CLib2D::FastColorMix75_25(*dest, *src);
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;

			case 2:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = CLib2D::FastColorMix63_37(*dest, *src);
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;

			case 3:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = CLib2D::FastColorMix50_50(*src, *dest);
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;

			case 4:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = CLib2D::FastColorMix63_37(*src, *dest);
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;

			case 5:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = CLib2D::FastColorMix75_25(*src, *dest);
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;

			case 6:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = CLib2D::FastColorMix88_12(*src, *dest);
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;

			case 7:
				do
				{
					len = clipped_width;
					do
					{
						if (*src)
							*dest = *src;
						*dest++;
						*src++;
					} while (--len);
					src  += src_stride;
					dest -= dest_stride;
				} while (--j);
				break;
		}
	}
#endif /* USE_OGL */
}


void DigitFont::draw_number(Image& buffer, unsigned nb,	unsigned x, unsigned y, bool bRightAligned, int cAppendDigit, unsigned iZeros  )
{
	{
		const unsigned nb_ = nb;
		unsigned digit_count = 1;
		unsigned nDigits = 0;
		int iLeadingZeros = 0;

		if (nb == 0)
		{		
			digit_count = 10;
			nDigits = 1;
		}
		else
		{
			while (nb > 0)
			{
				digit_count *= 10;
				nb /= 10;
				nDigits++;
			}
		}


		
		if(iZeros > nDigits)
		{
			iLeadingZeros= iZeros-nDigits;
			nDigits = iZeros;
		}

		if(bRightAligned)
			x -= nDigits * (width()+m_iSpacing);

		if(cAppendDigit!=-1)
			draw_digit(buffer, cAppendDigit, x-width(), y);

		while(iLeadingZeros--)
		{
			draw_digit(buffer, 0, x, y);
			x += font_width+m_iSpacing;
		}

		nb = nb_;
		while (digit_count > 1)
		{
			digit_count /= 10;
			draw_digit(buffer, (nb / digit_count) % 10, x, y);
			x += font_width+m_iSpacing;
		}
	}
}

void DigitFont::draw_time(Image& buffer, int time, unsigned x, unsigned y, int nMix)
{
	time *= 5;
	if (time < 0)
	{
		time = -time;
		draw_digit_mix(buffer, 11, x, y, nMix); // ':' //FIXME: check if we have a '-' in the font
		x += 3;
	}
	draw_digit_mix(buffer, (time / 60000) % 10, x, y, nMix);
	x += font_width;
	draw_digit_mix(buffer, (time / 6000) % 10, x, y, nMix);
	x += font_width;
	draw_digit_mix(buffer, 11, x, y, nMix); // ':'
	x += 4;
	draw_digit_mix(buffer, (time / 1000) % 6, x, y, nMix);
	x += font_width;
	draw_digit_mix(buffer, (time / 100)% 10, x, y, nMix);
	x += font_width;
	draw_digit_mix(buffer, 11, x, y, nMix); // ':'
	x += 4;
	draw_digit_mix(buffer, (time / 10) % 10, x, y, nMix);
	x += font_width;
	draw_digit_mix(buffer, time % 10, x, y, nMix);
}


void DigitFont::draw_string(Image& buffer, const char* txt, unsigned x, unsigned y, DS_FX nFx, int nFxParam , DS_ALIGN nAlign)
{
	A_ASSERT(m_pCharMap);//LoadCharMap must be called before calling draw_string

	int l = strlen(txt);

	if( nAlign == ALIGN_CENTERED) //centered
		x -= l*(font_width+m_iSpacing)/2;
	else if( nAlign == ALIGN_RIGHT) //right-aligned
		x -= l*(font_width+m_iSpacing); 

	for(int i = 0; i < l; i++)
	{
		int ox = i*(font_width+m_iSpacing);
		if( nFx == FX_NONE)
			draw_digit( buffer, m_pCharMap[(unsigned char)txt[i]], x+ox, y);
		else if( nFx == FX_MIX)
			draw_digit_mix( buffer, m_pCharMap[(unsigned char)txt[i]], x+ox, y, nFxParam);
		else if( nFx == FX_SHINE)
			draw_digit_shine( buffer, m_pCharMap[(unsigned char)txt[i]], x+ox, y, nFxParam);
		else if( nFx == FX_ADD)
			draw_digit_additive( buffer, m_pCharMap[(unsigned char)txt[i]], x+ox, y);
	}
}

int DigitFont::GetStringWidth(const char* txt)
{	
	int l = strlen(txt);
	return l*(font_width+m_iSpacing);
}

