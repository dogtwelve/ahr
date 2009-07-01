
#ifndef HG_LIB2D_FONT_H
# define HG_LIB2D_FONT_H

#include "Lib2D/Image.h"
#include "Lib3D2/Texture.h"

using namespace Lib3D;

enum TextAlignment
{
	HLeft   = 1,
	HMiddle = 2,
	HRight  = 4,
	VTop    = 8,
	VMiddle = 16,
	VBottom = 32
};

class DigitFont
{
public:
	DigitFont(const char* font_bitmap, unsigned nb_digits = 10);
	~DigitFont();
	void SetSpacing(int iSpacing) { m_iSpacing = iSpacing; }
	
	void draw_digit(Image& buffer, unsigned char digit, unsigned x, unsigned y);
	void draw_digit_shine(	Image& buffer, 
							unsigned char digit, 
							unsigned x, 
							unsigned y,
							int shine_pos,
							int shine_width = 5,
							int shine_shift = 1);

	void draw_number(Image& buffer, unsigned nb, unsigned x, unsigned y, bool bRightAligned = false, int cAppendDigit = -1, unsigned iZeros = 0);

	void draw_time(Image& buffer, int time, unsigned x, unsigned y, int nMix=7);
	void draw_digit_additive(Image& buffer, unsigned char digit, unsigned x, unsigned y);
	void draw_digit_mix(Image& buffer, unsigned char digit, unsigned x, unsigned y,int mixfactor=7);//mixfactor belong to [0,7]
	unsigned width() { return font_width; }
	unsigned height() { return font_height; }

	enum DS_FX {FX_NONE, FX_MIX, FX_SHINE, FX_ADD	};
	enum DS_ALIGN {ALIGN_LEFT, ALIGN_CENTERED, ALIGN_RIGHT	};
	void draw_string(Image& buffer, const char* txt, unsigned x, unsigned y, DS_FX nFx = FX_NONE, int nFxParam =0, DS_ALIGN nAlign = ALIGN_LEFT); //nAlign: 1 centered, 2 right;  
	int GetStringWidth(const char* txt);

	void LoadCharMap( const unsigned char * sCharOrder, unsigned char iDefaultIndex = 0 );
protected:
	unsigned font_width;
	unsigned font_height;
	int m_iSpacing;
//	Image*	 bitmap;
	Lib3D::TTexture *texture;

	unsigned char* m_pCharMap;

	int& m_dispX;
	int& m_dispY;
};



#endif //!HG_LIB2D_FONT_H