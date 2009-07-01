#include "devutil.h"
#include "Image.h"
#include "lib2d.h"

#ifdef __565_RENDERING__
	#define __BREW__
#endif

///////////////////////////////////////////////////////////
// Image 

Image::Image(A_IFile& file)
{
	Lib3D::TTexture* tex = NEW Lib3D::TTexture(file);
	is_owning_data_ = true;
	width_  = tex->SizeX();
	height_ = tex->SizeY();
	data_	= NEW pixel_type[width_ * height_];
	memcpy_(data_, tex->Data(), width_ * height_);

#ifdef USE_OGL
	getPow2Size(m_pow2Width, m_pow2Height, width_, height_);
#endif /* USE_OGL */

	MM_DELETE tex;
}


Image::Image(const char* filename)
{
//SEFU 8
	Lib3D::TTexture* tex = NEW Lib3D::TTexture(filename, false, 0);
	is_owning_data_ = true;
	width_  = tex->SizeX();
	height_ = tex->SizeY();
	data_	= NEW pixel_type[width_ * height_];
	memcpy_(data_, tex->Data(), width_ * height_);

#ifdef USE_OGL
	getPow2Size(m_pow2Width, m_pow2Height, width_, height_);
#endif /* USE_OGL */

	MM_DELETE tex;
}

Image::Image(const Image& rhs)
{
	is_owning_data_ = true;
	width_  = rhs.width_;
	height_ = rhs.height_;
	data_	= NEW pixel_type[width_ * height_];
	memcpy_(data_, rhs.data_, width_ * height_);

#ifdef USE_OGL
	getPow2Size(m_pow2Width, m_pow2Height, width_, height_);
#endif /* USE_OGL */

}

Image& Image::operator=(const Image& rhs)
{
	if (is_owning_data_ && data_)
		DELETE_ARRAY data_;

	is_owning_data_ = true;
	width_  = rhs.width_;
	height_ = rhs.height_;
	
#ifdef USE_OGL
	getPow2Size(m_pow2Width, m_pow2Height, width_, height_);
	m_glTextureName = rhs.m_glTextureName;
	
	//could be a PVRTC texture
	if (rhs.data_ != 0)
	{
		data_	= NEW pixel_type[width_ * height_];		
		memcpy_(data_, rhs.data_, width_ * height_);
	}

#else /* USE_OGL */

	data_	= NEW pixel_type[width_ * height_];		
	memcpy_(data_, rhs.data_, width_ * height_);

#endif /* USE_OGL */


	return *this;
}

Image::Image()
	: is_owning_data_(false),
	  width_(0),
	  height_(0),
	  data_(0)
	#ifdef USE_OGL
	  ,m_pow2Width(0)
	  ,m_pow2Height(0)
	#endif /* USE_OGL */
{
}

Image::Image(unsigned width, unsigned height)
	: is_owning_data_(true),
	  width_(width),
	  height_(height)
{
	data_ = NEW pixel_type[width * height];
#ifdef USE_OGL
	getPow2Size(m_pow2Width, m_pow2Height, width_, height_);
#endif /* USE_OGL */
}

Image::Image(unsigned width, unsigned height, pixel_type* data, int _textureName)
	: is_owning_data_(false),
	  width_(width),
	  height_(height),
	  data_(data)
{
#ifdef USE_OGL
	getPow2Size(m_pow2Width, m_pow2Height, width_, height_);
	m_glTextureName = _textureName;
#endif /* USE_OGL */
}

void getPow2Size(int &dstPow2Width, int &dstPow2Height, int srcWidth, int srcHeight)
{
	dstPow2Width = 1;
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;
}

///////////////////////////////////////////////////////////
// Regular Blit, with colorkey

void blit(Image& dest, const Image& src, unsigned x, unsigned y)
{
	const unsigned size_x = src.width();
	const unsigned size_y = src.height();
	const unsigned max_y = dest.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y);

	unsigned len = size_y;
	if ((len + y) > max_y)
		len = max_y - y;
	while (len--)
	{
		memcpy_colorkey(destp, datas, size_x);

		datas += size_x;
		destp += dest.width();		
	}
}

void blit_vflip(Image& dest, const Image& src, unsigned x, unsigned y, int iWidth , int iHeight )
{
	//A_ASSERT(iWidth<=src.width() );
	//A_ASSERT(iHeight<=src.height() );

	const unsigned size_x = iWidth!=-1? iWidth : src.width();
	const unsigned size_y = iHeight!=-1? iHeight : src.height();
	const unsigned max_y = dest.height();

	const pixel_type* datas = src.data();

	datas += size_x*(src.height()-size_y); //clip the beginning

	unsigned len = size_y;
	if ((len + y) > max_y)
		len = max_y - y;
	pixel_type* destp = &dest(x, y + len);
	while (len--)
	{
		destp -= dest.width();
		memcpy_colorkey(destp, datas, size_x);
		datas += src.width();
	}
}

void blit_vflip_mix(Image& dest, const Image& src, int x, int y, int iWidth , int iHeight, int iMix )
{
#ifdef USE_OGL
	f32 uv[4];

	uv[0] = 0.0f;
	uv[1] = (f32)iHeight / src.m_pow2Height;
	uv[2] = (f32)iWidth / src.m_pow2Width;
	uv[3] = 0.0f;

	iMix = (iMix << 28) + (iMix << 24);
	iMix |= 0x00FFFFFF;
	g_lib3DGL->paint2DModule(x, y, iWidth, iHeight, src.m_glTextureName, uv, 0, iMix);

#else

	const unsigned size_x = iWidth!=-1? iWidth : src.width();
	const unsigned size_y = iHeight!=-1? iHeight : src.height();

	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y + size_y);

	datas += size_x*(src.height()-size_y); //clip the beginning

	unsigned len = size_y;
	while (len--)
	{
		destp -= dest.width();
		if(iMix!=0xf)
			memcpy_colorkeymix(destp, datas, size_x, iMix);
		else
			memcpy_colorkey(destp, datas, size_x);
		datas += src.width();
	}

#endif /*USE_OGL*/

}

void blit_hflip(Image& dest, const Image& src, unsigned x, unsigned y)
{
	const unsigned size_x = src.width();
	const unsigned size_y = src.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y);

	unsigned len = size_y;
	while (len--)
	{
		memcpy_colorkey_reverse(destp, datas, size_x);
		datas += size_x;
		destp += dest.width();//m_dispX;
	}
}

void blit_hvflip(Image& dest, const Image& src, unsigned x, unsigned y)
{
	const unsigned size_x = src.width();
	const unsigned size_y = src.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y + size_y);

	unsigned len = size_y;
	while (len--)
	{
		destp -= dest.width();//m_dispX;
		memcpy_colorkey_reverse(destp, datas, size_x);
		datas += size_x;
	}
}

void draw_line_clip(Image& dest, Image& src, int x, int y, int line)
{
	int leftclipx = 0;
	if (x < 0) leftclipx = -x;
	if ((unsigned int)leftclipx >= src.width()) return;

	int rightclipx = dest.width()-(x+src.width());
	if (rightclipx > 0) rightclipx = 0;
	if ((unsigned int)(-rightclipx) >= src.width()) return;

	int startDstX = x + leftclipx;
	int startSrcX = leftclipx;

	memcpy_colorkey(&dest(startDstX, y), &src(startSrcX,line), src.width() + rightclipx - leftclipx);
}

void draw_mou(Image& dest, Image& src, int x, int y, int mou)
{
	const unsigned size_y = src.height();
	for(unsigned int i = 0; i < size_y; ++i)
	{
		int local_mou = mou*i/size_y;
		draw_line_clip(dest, src, x+local_mou, y+i, i);
	}
}

///////////////////////////////////////////////////////////
// Additive Blit

void blit_additive(Image& dest, const Image& src, unsigned x, unsigned y)
{
#ifdef USE_OGL
	f32 uv[4];

	uv[0] = 0.0f;
	uv[1] = 1.0f;
	uv[2] = 1.0f;
	uv[3] = 0.0f;

	g_lib3DGL->paint2DModule(x, y, src.width(), src.height(), src.m_glTextureName, uv, FLAG_USE_ADDITIVE_BLEND);

#else

	const unsigned size_x = src.width();
	const unsigned size_y = src.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y);

	unsigned len = size_y;
	while (len--)
	{
		memcpy_additive(destp, datas, size_x);
		datas += size_x;
		destp += dest.width();
	}

#endif /* USE_OGL */
}

void blit_additive_clipped(Image& dest, const Image& src, int x, int y)
{
#ifdef USE_OGL
	f32 uv[4];

	uv[0] = 0.0f;
	uv[1] = 1.0f * src.height() / src.m_pow2Height;
	uv[2] = 1.0f * src.width() / src.m_pow2Width;
	uv[3] = 0.0f;	

	g_lib3DGL->paint2DModule(x, y, src.width(), src.height(), src.m_glTextureName, uv, FLAG_USE_ADDITIVE_BLEND);

#else
	int size_x = src.width();
	int size_y = src.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y);

	if (x >= (int)dest.width())
		return;
	if (y >= (int)dest.height())
		return;


	int		max = (x + size_x);
	if (max >= (int)dest.width())
	{
		size_x -= max - dest.width() + 1;
	}

	max = (y + size_y);
	if (max >= (int)dest.height())
	{
		size_y -= max - dest.height() + 1;
	}

	if (y < 0)
	{
		destp += (-y) * dest.width();
		datas += (-y) * size_x;
		size_y += y;
	}

	if (x < 0)
	{
		destp += (-x);
		datas += (-x);
		size_x += x;
	}

	if (size_x <= 0)
		return;
	if (size_y <= 0)
		return;

	unsigned len = size_y;
	while (len--)
	{
		memcpy_additive(destp, datas, size_x);
		datas += src.width();
		destp += dest.width();
	}

#endif /* USE_OGL */
}

//void blit_vflip_additive(Image& dest, const Image& src, int x, int y, int iWidth , int iHeight)
//{
//	//A_ASSERT(iWidth<=src.width() );
//	//A_ASSERT(iHeight<=src.height() );
//
//	const unsigned size_x = iWidth!=-1? iWidth : src.width();
//	const unsigned size_y = iHeight!=-1? iHeight : src.height();
//
//	const pixel_type* datas = src.data();
//	pixel_type* destp = &dest(x, y + size_y);
//
//	datas += size_x*(src.height()-size_y); //clip the beginning
//
//	//quick fix 
//	int iClipX = (x<0? -x : 0);
//
//
//	unsigned len = size_y;
//	while (len--)
//	{
//		destp -= dest.width();
//		memcpy_additive(destp+iClipX, datas+iClipX, size_x-iClipX);
//		datas += src.width();
//	}
//}
//
//void blit_hflip_additive(Image& dest, const Image& src, unsigned x, unsigned y)
//{
//	const unsigned size_x = src.width();
//	const unsigned size_y = src.height();
//	const pixel_type* datas = src.data();
//	pixel_type* destp = &dest(x, y);
//
//	unsigned len = size_y;
//	while (len--)
//	{
//		memcpy_additive_reverse(destp, datas, size_x);
//		datas += size_x;
//		destp += dest.width();//m_dispX;
//	}
//}

void blit_hvflip_additive(Image& dest, const Image& src, unsigned x, unsigned y)
{
	const unsigned size_x = src.width();
	const unsigned size_y = src.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y + size_y);

	unsigned len = size_y;
	while (len--)
	{
		destp -= dest.width();//m_dispX;
		memcpy_additive_reverse(destp, datas, size_x);
		datas += size_x;
	}
}


///////////////////////////////////////////////////////////
// Memory fill


void memset_(pixel_type* dest, const pixel_type value, size_t2 len)
{
	while (len--)
		*dest++ = value;
}

void memcpy_(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	while (len--)
		*dest++ = *src++;
}

void memcpy_faded(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	while (len--)
	{
#ifdef __BREW__
		if (*src)
			*dest =    ((*src & 0x1f  ) >> 1)
					| (((*src & 0x7e0 ) >> 1) & 0x7e0)
					| (((*src & 0xf800) >> 1) & 0xf800);
#else
		if (*src)
			*dest =    ((*src & 0xf  ) >> 1)
					| (((*src & 0xf0 ) >> 1) & 0xf0)
					| (((*src & 0xf00) >> 1) & 0xf00);
#endif
		++dest;
		++src;
	}
}

void memcpy_colorkey(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	while (len--)
	{
		if (*src)
			*dest = *src;
		++dest;
		++src;
	}
}

void memcpy_colorkeymix(pixel_type* dest, const pixel_type* src, size_t2 len, int iMix)
{
	while (len--)
	{
		if (*src)
			*dest = CLib2D::MixColor(*src, *dest, iMix)  ;
		++dest;
		++src;
	}
}

void memcpy_colorkey_reverse(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	dest += len;
	while (len--)
	{
		--dest;
		if (*src)
			*dest = *src;
		++src;
	}
}

void memcpy_additive(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	while (len--)
	{
		// thanks to board3.cpp :)
		unsigned int r, g, b;

#ifdef __BREW__
		r = (*src & 0xF800) + (unsigned int)(*dest & 0xF800);
		if (r > 0xF800) r = 0xF800;
		g = (*src & 0x7e0 ) + (*dest & 0x7e0 );
		if (g > 0x7e0 ) g = 0x7e0;
		b = (*src & 0x1f  ) + (*dest & 0x1f  );
		if (b > 0x1F  ) b = 0x1F;
#else
		r = (*src & 0xF00) + (*dest & 0xF00);
		if (r > 0xF00) r = 0xF00;
		g = (*src & 0xF0 ) + (*dest & 0xF0 );
		if (g > 0xF0 ) g = 0xF0;
		b = (*src & 0xF  ) + (*dest & 0xF  );
		if (b > 0xF  ) b = 0xF;
#endif
					
		*dest++ = r | g | b;
		++src;
	}
}

void memcpy_additive_reverse(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	dest += len;
	while (len--)
	{
		--dest;

		// thanks to board3.cpp :)
		unsigned int r, g, b;

#ifdef __BREW__
		r = (*src & 0xF800) + (unsigned int)(*dest & 0xF800);
		if (r > 0xF800) r = 0xF800;
		g = (*src & 0x7e0 ) + (*dest & 0x7e0 );
		if (g > 0x7e0 ) g = 0x7e0;
		b = (*src & 0x1F  ) + (*dest & 0x1F  );
		if (b > 0x1F  ) b = 0x1F;
#else
		r = (*src & 0xF00) + (*dest & 0xF00);
		if (r > 0xF00) r = 0xF00;
		g = (*src & 0xF0 ) + (*dest & 0xF0 );
		if (g > 0xF0 ) g = 0xF0;
		b = (*src & 0xF  ) + (*dest & 0xF  );
		if (b > 0xF  ) b = 0xF;
#endif

		*dest = r | g | b;
		++src;
	}
}


///////////////////////////////////////////////////////////
// partial blit

void blit(	Image& dest, const Image& src,
			unsigned x, unsigned y,
			unsigned width, unsigned height)
{
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y);

	while (height--)
	{
		memcpy_colorkey(destp, datas, width);
		datas += src.width();
		destp += dest.width();
	}
}

///////////////////////////////////////////////////////////
// misc

void memcpy_vertical_additive(pixel_type* dest, const pixel_type* src,
							  size_t2 ylen, size_t2 dest_size_x, size_t2 src_size_x)
{
	while (ylen--)
	{
		*dest = CLib2D::FastColorAdd(*dest, *src);
		dest += dest_size_x;
		src += src_size_x;
	}
}

void memcpy_vertical_colorkey(pixel_type* dest, const pixel_type* src,
							  size_t2 ylen, size_t2 dest_size_x, size_t2 src_size_x)
{
	while (ylen--)
	{
		if (*src)
			*dest = *src;
		dest += dest_size_x;
		src += src_size_x;
	}
}


///////////////////////////////////////////////////////////
// fx

void memcpy_black_colorkey(pixel_type* dest, const pixel_type* src, size_t2 len)
{
	while (len--)
	{
		if (*src)
			*dest = *src;
		++dest;
		++src;
	}
}

void blit_vflip_black_colorkey(Image& dest, const Image& src, unsigned x, unsigned y)
{
	const unsigned size_x = src.width();
	const unsigned size_y = src.height();
	const pixel_type* datas = src.data();
	pixel_type* destp = &dest(x, y + size_y);

	unsigned len = size_y;
	while (len--)
	{
		destp -= dest.width();
		memcpy_black_colorkey(destp, datas, size_x);
		datas += size_x;
	}
}

pixel_type fade_color(pixel_type color, int percentage)
{
	unsigned r = (((color & 0xf00) >> 8) * percentage / 100);
	if (r > 0xf) r = 0xf;
	unsigned g = (((color & 0x0f0) >> 4) * percentage / 100);
	if (g > 0xf) g = 0xf;
	unsigned b = ( (color & 0x00f)       * percentage / 100);
	if (b > 0xf) b = 0xf;
	color =	(r << 8) | (g << 4) | b;
	return color;
}

//FIXME: incorrect, bugged, to trash
// use fade_image instead
void fade_buffer(Image& buffer, int percentage, unsigned strip)
{
	unsigned ylen = buffer.height();
	pixel_type* p = buffer.data();

	while (--ylen)
	{
		unsigned len = buffer.width();
		while (--len)
		{
			pixel_type color = *p;
			unsigned r = (((color & 0xf00) >> 8) * percentage / 100);
			if (r > 0xf) r = 0xf;
			unsigned g = (((color & 0x0f0) >> 4) * percentage / 100);
			if (g > 0xf) g = 0xf;
			unsigned b = ( (color & 0x00f)       * percentage / 100);
			if (b > 0xf) b = 0xf;
			color =	(r << 8) | (g << 4) | b;
			*p++ = color;
		}
		p += strip;
	}
}

//FIXME: incorrect, bugged, to trash
// use fade_image instead
void fade_buffer_preserve_colorkey(Image& buffer, int percentage, unsigned strip)
{
	unsigned ylen = buffer.height();
	pixel_type* p = buffer.data();

	while (--ylen)
	{
		unsigned len = buffer.width();
		while (--len)
		{
			pixel_type color = *p;
			if (color != 0xff0f)
			{
				unsigned r = (((color & 0xf00) >> 8) * percentage / 100);
				if (r > 0xf) r = 0xf;
				unsigned g = (((color & 0x0f0) >> 4) * percentage / 100);
				if (g > 0xf) g = 0xf;
				unsigned b = ( (color & 0x00f)       * percentage / 100);
				if (b > 0xf) b = 0xf;
				color =	(r << 8) | (g << 4) | b;
				*p++ = color;
			}
			else
				++p;
		}
		p += strip;
	}
}

void fade_image(Image& buffer, int percentage, unsigned strip)
{
	unsigned ylen = buffer.height();
	pixel_type* p = buffer.data();

	while (ylen--)
	{
		// [OPTIMIZEME] use the CLib2D color mixing functions
		unsigned len = buffer.width();
		while (len--)
		{
			pixel_type color = *p;
			unsigned r = (((color & 0xf00) >> 8) * percentage / 100);
			if (r > 0xf) r = 0xf;
			unsigned g = (((color & 0x0f0) >> 4) * percentage / 100);
			if (g > 0xf) g = 0xf;
			unsigned b = ( (color & 0x00f)       * percentage / 100);
			if (b > 0xf) b = 0xf;
			color =	(r << 8) | (g << 4) | b;
			*p++ = color;
		}
		p += strip;
	}
}

void fade_image_preserve_colorkey(Image& buffer, int percentage, unsigned strip)
{
	unsigned ylen = buffer.height();
	pixel_type* p = buffer.data();

	while (ylen--)
	{
		// [OPTIMIZEME] use the CLib2D color mixing functions
		unsigned len = buffer.width();
		while (len--)
		{
			pixel_type color = *p;
			if (color != 0xff0f)
			{
				unsigned r = (((color & 0xf00) >> 8) * percentage / 100);
				if (r > 0xf) r = 0xf;
				unsigned g = (((color & 0x0f0) >> 4) * percentage / 100);
				if (g > 0xf) g = 0xf;
				unsigned b = ( (color & 0x00f)       * percentage / 100);
				if (b > 0xf) b = 0xf;
				color =	(r << 8) | (g << 4) | b;
				*p++ = color;
			}
			else
				++p;
		}
		p += strip;
	}
}

void convert_to_grayscale(Image& img)
{
	pixel_type* p = img.data();
	unsigned len = img.width() * img.height();
	while (len--)
	{
		if (*p != 0xff0f)
		{
			unsigned r = (*p & 0xf00) >> 8;
			unsigned g = (*p & 0x0f0) >> 4;
			unsigned b = (*p & 0x00f);
			unsigned c = (r + g + b) / 3;
			*p = (c << 8) | (c << 4) | c;
		}
		++p;
	}
}
