
#ifndef HG_LIB2D_IMAGE_H
# define HG_LIB2D_IMAGE_H

#include "config.h"

#ifdef USE_OGL
	#ifdef IPHONE
		#import <OpenGLES/ES1/gl.h>
		#import <OpenGLES/ES1/glext.h>	
	#else
		#include <windows.h>
		//#include "GLES/egl.h"
		#include "GL/gl.h"
		#include "GL/glext.h"
		#include "GL/GLES_TO_WGL.h"
	#endif // IPHONE
#endif /* USE_OGL */

///////////////////////////////////////////////////////////
// helper

struct ImgSize
{
	ImgSize(unsigned width, unsigned height)
		: width(width), height(height)
	{}

	unsigned width;
	unsigned height;
};

class A_IFile;

///////////////////////////////////////////////////////////
// no template to speed up compilation time and prevent code bloat

typedef unsigned short pixel_type;
typedef unsigned int size_t2;

struct Image
{
	Image();
	Image(unsigned width, unsigned height);
	Image(unsigned width, unsigned height, pixel_type* data, int _textureName = 0);

	Image(const Image& rhs);
	Image(const char* filename);
	Image(A_IFile&);

	~Image()
	{
		if (is_owning_data_ && data_)
			DELETE_ARRAY data_;
	}

	Image& operator=(const Image& rhs);

	pixel_type& operator[](unsigned offset)			{ return data_[offset]; }
	pixel_type  operator[](unsigned offset) const	{ return data_[offset]; }

	pixel_type& operator()(unsigned i, unsigned j)		 { return data_[j * width_ + i]; }
	pixel_type  operator()(unsigned i, unsigned j) const { return data_[j * width_ + i]; }

inline	pixel_type*		  data()		 { return data_; }
inline	const pixel_type* data() const   { return data_; }

inline	unsigned		  width()  const { return width_; }
inline	unsigned		  height() const { return height_; }

inline void rotate() { int tmp = width_; width_ = height_; height_ = tmp; }
// Attention: width_ * height_ must be equal to w * h
inline void changeSize(int w, int h) { width_ = w; height_ = h; }

protected:
	bool		is_owning_data_;
	unsigned	width_;
	unsigned	height_;

	pixel_type* data_;

public:
#ifdef USE_OGL
	int			m_pow2Width;
	int			m_pow2Height;
	GLuint		m_glTextureName;
#endif /* USE_OGL */
};

void getPow2Size(int &dstPow2Width, int &dstPow2Height, int srcWidth, int srcHeight);

void blit		(Image& dest, const Image& src, unsigned x = 0, unsigned y = 0);
void blit_vflip (Image& dest, const Image& src, unsigned x = 0, unsigned y = 0, int iWidth = -1, int iHeight = -1);
void blit_hflip (Image& dest, const Image& src, unsigned x = 0, unsigned y = 0);
void blit_hvflip(Image& dest, const Image& src, unsigned x = 0, unsigned y = 0);

void blit_additive		 (Image& dest, const Image& src, unsigned x = 0, unsigned y = 0);
void blit_additive_clipped(Image& dest, const Image& src, int x = 0, int y = 0);
void blit_vflip_additive (Image& dest, const Image& src, int x = 0, int y = 0, int iWidth = -1, int iHeight = -1);
void blit_vflip_mix		 (Image& dest, const Image& src, int x = 0, int y = 0, int iWidth = -1, int iHeight = -1, int iMix = 0xf);
//void blit_hflip_additive (Image& dest, const Image& src, unsigned x = 0, unsigned y = 0);
//void blit_hvflip_additive(Image& dest, const Image& src, unsigned x = 0, unsigned y = 0);

void draw_mou(Image& dest, Image& src, int x, int y, int mou);

void memset_				(pixel_type* dest, const pixel_type value, size_t2 len);
void memcpy_				(pixel_type* dest, const pixel_type* src,  size_t2 len);
void memcpy_faded			(pixel_type* dest, const pixel_type* src,  size_t2 len);
void memcpy_colorkey		(pixel_type* dest, const pixel_type* src,  size_t2 len);
void memcpy_colorkeymix		(pixel_type* dest, const pixel_type* src,  size_t2 len, int iMix);
void memcpy_colorkey_reverse(pixel_type* dest, const pixel_type* src,  size_t2 len);
void memcpy_additive		(pixel_type* dest, const pixel_type* src,  size_t2 len);
void memcpy_additive_reverse(pixel_type* dest, const pixel_type* src,  size_t2 len);


void blit(Image& dest, const Image& src,
		  unsigned x, unsigned y, unsigned width, unsigned height);

void memcpy_vertical_additive(pixel_type* dest, const pixel_type* src,
							  size_t2 ylen, size_t2 dest_size_x, size_t2 src_size_x);
void memcpy_vertical_colorkey(pixel_type* dest, const pixel_type* src,
							  size_t2 ylen, size_t2 dest_size_x, size_t2 src_size_x);


void blit_vflip_aliased(Image& dest, const Image& src, unsigned x, unsigned y, pixel_type color);
void blit_vflip_black_colorkey(Image& dest, const Image& src, unsigned x, unsigned y);

pixel_type fade_color(pixel_type color, int percentage);

// FIXME: bugged version
void fade_buffer(Image& buffer, int percentage, unsigned strip = 0);
void fade_buffer_preserve_colorkey(Image& buffer, int percentage, unsigned strip = 0);

// Note: good version
void fade_image(Image& buffer, int percentage, unsigned strip = 0);
void fade_image_preserve_colorkey(Image& buffer, int percentage, unsigned strip = 0);

void convert_to_grayscale(Image& img);


// compatibility
#include "Texture.h"

#ifdef USE_OGL
inline Image image_from_texture(Lib3D::TTexture& tex) { return Image(tex.SizeX(), tex.SizeY(), tex.Data(), tex.m_glTextureName); }
#else
inline Image image_from_texture(Lib3D::TTexture& tex) { return Image(tex.SizeX(), tex.SizeY(), tex.Data()); }
#endif /* USE_OGL */


#endif //HG_LIB2D_IMAGE_H
