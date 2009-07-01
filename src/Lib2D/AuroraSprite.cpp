#include "HighGear.h"
#include "Lib2D/AuroraSprite.h"
#include "Lib2D/Lib2D.h"
#include "str_utils.h"

#define ENABLE_FONT_KERNING

#ifdef USE_FANCY_MENUS
#else // USE_FANCY_MENUS
	#include "Menu/DemoMenu.h"
#endif // USE_FANCY_MENUS

#ifdef USE_OGL
	#if defined(DEBUG_SPRITE_GLTEXTURE_ALLOC) && defined(_DEBUG)
		u32 CSprite::s_mapTexUniqueIdToGLTexName[k_MAX_TEXTURES_LOADED] = {0};
	#endif //DEBUG_SPRITE_GLTEXTURE_ALLOC
#endif //USE_OGL

CModule::CModule(int _width, int _height, int _type, int _x, int _y)
: width(_width),
height(_height)
{
	useAlpha = ((_type & 0x02) == 0x02);
	useIndexedPixels = ((_type & 0x01) == 0x01);
	useLowColor = ((_type & 0x04) == 0x04);

#ifdef USE_SINGLE_IMAGE
	
	x = _x;
	y = _y;

#else // USE_SINGLE_IMAGE

	pixels = 0;
	indexedPixels = 0;
	alpha = 0;

	if (useIndexedPixels)
	{
		int size = width * height;
		if (useLowColor)
		{
			size = (width * height + 1) >> 1;
		}

		indexedPixels = NEW unsigned char[size];
	}
	else
	{
		pixels = NEW unsigned short[width * height];
	}

	if (useAlpha)
	{
		int size = (width * height + 1) >> 1;

		alpha  = NEW unsigned char [size];

		memset (alpha, 0xFF, size);
	}

#endif // USE_SINGLE_IMAGE
}

CModule::~CModule() {

#ifndef USE_SINGLE_IMAGE

	if(useIndexedPixels) {
		SAFE_DELETE_ARRAY(indexedPixels);
	} else {
		SAFE_DELETE_ARRAY(pixels);
	}

	if(useAlpha) {
		SAFE_DELETE_ARRAY(alpha);
	}

#endif // USE_SINGLE_IMAGE

}

CAuroraPalette::CAuroraPalette(short _numColors)
: numColors(_numColors),
data(0)
{
	data = NEW unsigned char[numColors * 3];

	alphaIndex = -1;
}

CAuroraPalette::~CAuroraPalette()
{
	SAFE_DELETE_ARRAY(data);
}


//////////////////////////////////////////////////////////////////////////////////////

CFrame::CFrame(short _numFModules)
: numFModules(_numFModules),
fModules(0)
{
	fModules = NEW CFModule*[numFModules];
}

CFrame::~CFrame() {
	for(int i=0; i<numFModules; i++)
		SAFE_DELETE(fModules[i]);

	SAFE_DELETE_ARRAY(fModules);
}

//////////////////////////////////////////////////////////////////////////////////////

CAnim::CAnim(short _numAFrames)
: numAFrames(_numAFrames),
aFrames(0)
{
	aFrames = NEW CAFrame*[numAFrames];
}

CAnim::~CAnim()
{
	for(int i=0; i<numAFrames; i++)
		SAFE_DELETE(aFrames[i]);

	SAFE_DELETE_ARRAY(aFrames);
}


////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
	K_VERTICAL,
	K_SLOPE_UP,
	K_SLOPE_DN,
	K_ROUNDISH, // cause K_ROUND was shorter than the others :)
	K_POINT,
	K_EDGE_TYPE_NO,
} TEdgeType;
/*
const int k_letters_normal[] = 
{
	K_SLOPE_UP, K_SLOPE_DN, // A
	K_VERTICAL, K_VERTICAL, // B
	K_SLOPE_UP, K_SLOPE_UP, // C
	K_ROUNDISH, K_ROUNDISH, // D
	K_VERTICAL, K_VERTICAL, // E
	K_VERTICAL, K_SLOPE_UP, // F
	K_ROUNDISH, K_VERTICAL, // G
	K_VERTICAL, K_VERTICAL, // H
	K_VERTICAL, K_VERTICAL, // I
	K_SLOPE_UP, K_VERTICAL, // J
	K_VERTICAL, K_VERTICAL, // K
	K_VERTICAL, K_SLOPE_DN, // L
	K_VERTICAL, K_VERTICAL, // M
	K_VERTICAL, K_VERTICAL, // N
	K_ROUNDISH, K_ROUNDISH, // O
	K_VERTICAL, K_SLOPE_UP, // P
	K_ROUNDISH, K_ROUNDISH, // Q
	K_VERTICAL, K_VERTICAL, // R
	K_ROUNDISH, K_ROUNDISH, // S
	K_SLOPE_DN, K_SLOPE_UP, // T
	K_VERTICAL, K_VERTICAL, // U
	K_SLOPE_DN, K_SLOPE_UP, // V
	K_SLOPE_DN, K_SLOPE_UP, // W
	K_VERTICAL, K_VERTICAL, // X
	K_SLOPE_DN, K_SLOPE_UP, // Y
	K_VERTICAL, K_VERTICAL  // Z
};
*/

// rax: Do not modify ! If you want to change the kerning for font normal or font ingame, use other array
const int k_letters_italic[] = 
{
	K_VERTICAL, K_VERTICAL, // space	//0

	K_SLOPE_UP, K_SLOPE_UP, // !		//1
	K_SLOPE_UP, K_SLOPE_UP, // "
	K_SLOPE_UP, K_SLOPE_UP, // #
	K_VERTICAL, K_VERTICAL, // $
	K_VERTICAL, K_VERTICAL, // %
	K_SLOPE_UP, K_VERTICAL, // &
	K_VERTICAL, K_SLOPE_UP, // '
	K_VERTICAL, K_SLOPE_UP, // (
	K_SLOPE_UP, K_VERTICAL, // )
	K_VERTICAL, K_SLOPE_UP, // *
	K_VERTICAL, K_SLOPE_UP, // +
	K_SLOPE_UP, K_ROUNDISH,	// ,
	K_ROUNDISH, K_ROUNDISH, // -
	K_POINT,    K_VERTICAL, // .
	K_SLOPE_UP, K_SLOPE_UP, // /

	K_SLOPE_UP, K_ROUNDISH, // 0		//16
	K_SLOPE_UP, K_VERTICAL, // 1
	K_SLOPE_UP, K_SLOPE_UP, // 2
	K_SLOPE_UP, K_VERTICAL, // 3
	K_ROUNDISH, K_VERTICAL, // 4
	K_SLOPE_UP, K_VERTICAL, // 5
	K_VERTICAL, K_VERTICAL, // 6
	K_VERTICAL, K_SLOPE_UP, // 7
	K_SLOPE_UP, K_VERTICAL, // 8
	K_VERTICAL, K_VERTICAL, // 9

	K_SLOPE_UP, K_SLOPE_UP, // :		//26
	K_SLOPE_UP, K_SLOPE_UP, // ;
	K_ROUNDISH, K_SLOPE_UP, // <
	K_SLOPE_UP, K_SLOPE_UP, // =
	K_SLOPE_UP, K_ROUNDISH, // >
	K_VERTICAL, K_SLOPE_UP, // ?
	K_SLOPE_UP, K_SLOPE_UP, // @
	
	K_SLOPE_UP, K_SLOPE_UP, // A		//33 //caps
	K_SLOPE_UP, K_SLOPE_UP, // B
	K_SLOPE_UP, K_SLOPE_UP, // C
	K_SLOPE_UP, K_SLOPE_UP, // D
	K_SLOPE_UP, K_SLOPE_UP, // E
	K_SLOPE_UP, K_SLOPE_UP, // F
	K_SLOPE_UP, K_SLOPE_UP, // G
	K_SLOPE_UP, K_SLOPE_UP, // H
	K_SLOPE_UP, K_SLOPE_UP, // I
	K_SLOPE_UP, K_SLOPE_UP, // J
	K_SLOPE_UP, K_SLOPE_UP, // K
	K_SLOPE_UP, K_VERTICAL, // L
	K_SLOPE_UP, K_SLOPE_UP, // M
	K_SLOPE_UP, K_SLOPE_UP, // N
	K_SLOPE_UP, K_SLOPE_UP, // O
	K_SLOPE_UP, K_SLOPE_UP, // P
	K_SLOPE_UP, K_SLOPE_UP, // Q
	K_SLOPE_UP, K_SLOPE_UP, // R
	K_SLOPE_UP, K_SLOPE_UP, // S
	K_VERTICAL, K_SLOPE_UP, // T
	K_SLOPE_UP, K_SLOPE_UP, // U
	K_VERTICAL, K_SLOPE_UP, // V
	K_VERTICAL, K_SLOPE_UP, // W
	K_SLOPE_UP, K_SLOPE_UP, // X
	K_VERTICAL, K_SLOPE_UP, // Y
	K_SLOPE_UP, K_SLOPE_UP, // Z

	K_SLOPE_UP, K_SLOPE_UP, // [		//59
	K_SLOPE_DN, K_SLOPE_DN, // " \ "
	K_SLOPE_UP, K_SLOPE_UP, // ]
	K_VERTICAL, K_VERTICAL, // ^
	K_VERTICAL, K_VERTICAL, // _
	K_VERTICAL, K_VERTICAL, // `

	K_SLOPE_UP, K_VERTICAL, // A		//65
	K_SLOPE_UP, K_SLOPE_UP, // B
	K_SLOPE_UP, K_SLOPE_UP, // C
	K_SLOPE_UP, K_SLOPE_UP, // D
	K_SLOPE_UP, K_SLOPE_UP, // E
	K_SLOPE_UP, K_SLOPE_UP, // F
	K_SLOPE_UP, K_SLOPE_UP, // G
	K_SLOPE_UP, K_SLOPE_UP, // H
	K_SLOPE_UP, K_SLOPE_UP, // I
	K_SLOPE_UP, K_SLOPE_UP, // J
	K_SLOPE_UP, K_SLOPE_UP, // K
	K_SLOPE_UP, K_VERTICAL, // L
	K_SLOPE_UP, K_SLOPE_UP, // M
	K_SLOPE_UP, K_SLOPE_UP, // N
	K_SLOPE_UP, K_SLOPE_UP, // O
	K_SLOPE_UP, K_SLOPE_UP, // P
	K_SLOPE_UP, K_SLOPE_UP, // Q
	K_SLOPE_UP, K_SLOPE_UP, // R
	K_SLOPE_UP, K_SLOPE_UP, // S
	K_VERTICAL, K_SLOPE_UP, // T
	K_SLOPE_UP, K_SLOPE_UP, // U
	K_VERTICAL, K_SLOPE_UP, // V
	K_VERTICAL, K_SLOPE_UP, // W
	K_SLOPE_UP, K_SLOPE_UP, // X
	K_VERTICAL, K_SLOPE_UP, // Y
	K_SLOPE_UP, K_SLOPE_UP, // Z

	K_VERTICAL, K_SLOPE_UP, // {		//91
	K_SLOPE_UP, K_SLOPE_UP, // |
	K_SLOPE_UP, K_VERTICAL, // }
	K_SLOPE_DN, K_SLOPE_UP, // ~
	K_VERTICAL, K_VERTICAL, // garbage

	K_SLOPE_UP, K_VERTICAL, // À		//96	//caps
	K_SLOPE_UP, K_VERTICAL, // Á
	K_SLOPE_UP, K_VERTICAL, // Â
	K_SLOPE_UP, K_VERTICAL, // Ã
	K_SLOPE_UP, K_VERTICAL, // garbage
	K_SLOPE_UP, K_VERTICAL, // Ä
	K_SLOPE_UP, K_VERTICAL, // Å
	K_SLOPE_UP, K_VERTICAL, // Æ
	K_VERTICAL, K_SLOPE_UP, // Ç
	K_SLOPE_UP, K_SLOPE_UP, // È
	K_SLOPE_UP, K_SLOPE_UP, // É
	K_SLOPE_UP, K_SLOPE_UP, // Ê
	K_SLOPE_UP, K_SLOPE_UP, // Ë
	K_SLOPE_UP, K_SLOPE_UP, // Ì
	K_SLOPE_UP, K_SLOPE_UP, // Í
	K_SLOPE_UP, K_SLOPE_UP, // Î
	K_SLOPE_UP, K_SLOPE_UP, // Ï
	K_SLOPE_UP, K_ROUNDISH, // Ð
	K_SLOPE_UP, K_SLOPE_UP, // Ñ
	K_SLOPE_UP, K_SLOPE_UP, // Ò
	K_SLOPE_UP, K_SLOPE_UP, // Ó
	K_SLOPE_UP, K_SLOPE_UP, // Ô
	K_SLOPE_UP, K_SLOPE_UP, // Õ
	K_SLOPE_UP, K_SLOPE_UP, // Ö
	K_VERTICAL, K_VERTICAL, // garbage
	K_SLOPE_UP, K_SLOPE_UP, // Ø
	K_SLOPE_UP, K_SLOPE_UP, // Ù
	K_SLOPE_UP, K_SLOPE_UP, // Ú
	K_SLOPE_UP, K_SLOPE_UP, // Û
	K_SLOPE_UP, K_SLOPE_UP, // Ü
	K_VERTICAL, K_SLOPE_UP, // Ý
	K_SLOPE_UP, K_ROUNDISH, // Þ
	K_SLOPE_UP, K_VERTICAL, // ß

	K_ROUNDISH, K_VERTICAL, // CE		//129
	K_SLOPE_DN, K_SLOPE_DN, // '
	K_ROUNDISH, K_VERTICAL, // CE
	K_SLOPE_UP, K_VERTICAL, // ¡
	K_ROUNDISH, K_ROUNDISH, // ©
	K_SLOPE_UP, K_VERTICAL, // ¿
	K_VERTICAL, K_VERTICAL, // tm
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage
	K_VERTICAL, K_VERTICAL, // garbage

	K_SLOPE_UP, K_VERTICAL, // À		//144
	K_SLOPE_UP, K_VERTICAL, // Á
	K_SLOPE_UP, K_VERTICAL, // Â
	K_SLOPE_UP, K_VERTICAL, // Ã
	K_SLOPE_UP, K_VERTICAL, // Ä
	K_SLOPE_UP, K_VERTICAL, // Å
	K_SLOPE_UP, K_VERTICAL, // Æ
	K_VERTICAL, K_SLOPE_UP, // Ç
	K_SLOPE_UP, K_SLOPE_UP, // È
	K_SLOPE_UP, K_SLOPE_UP, // É
	K_SLOPE_UP, K_SLOPE_UP, // Ê
	K_SLOPE_UP, K_SLOPE_UP, // Ë
	K_SLOPE_UP, K_SLOPE_UP, // Ì
	K_SLOPE_UP, K_SLOPE_UP, // Í
	K_SLOPE_UP, K_SLOPE_UP, // Î
	K_SLOPE_UP, K_SLOPE_UP, // Ï
	K_SLOPE_UP, K_ROUNDISH, // Ð
	K_SLOPE_UP, K_SLOPE_UP, // Ñ
	K_SLOPE_UP, K_SLOPE_UP, // Ò
	K_SLOPE_UP, K_SLOPE_UP, // Ó
	K_SLOPE_UP, K_SLOPE_UP, // Ô
	K_SLOPE_UP, K_SLOPE_UP, // Õ
	K_SLOPE_UP, K_SLOPE_UP, // Ö
	K_VERTICAL, K_VERTICAL, // garbage
	K_SLOPE_UP, K_SLOPE_UP, // Ø
	K_SLOPE_UP, K_SLOPE_UP, // Ù
	K_SLOPE_UP, K_SLOPE_UP, // Ú
	K_SLOPE_UP, K_SLOPE_UP, // Û
	K_SLOPE_UP, K_SLOPE_UP, // Ü
	K_VERTICAL, K_SLOPE_UP, // Ý
	K_SLOPE_UP, K_ROUNDISH, // Þ
	K_SLOPE_UP, K_VERTICAL, // ß
};

const int k_kerning[] = 
{
	0,  0, -1, -1,  0, // vertical
	0, -2, -1, -2, -2, // up
	-1, -1, -2, -2, 0, // down
	-1, -2, -2, -1, -2, // roundish
	0,  0,  0,   0,  0, // point
};

void CSprite::initKerning( bool bItalic )
{
	m_italic = bItalic;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int  CSprite::GetKerningSpacing(unsigned short currentChar)
{
#ifdef ENABLE_FONT_KERNING

	if ( m_FontMap == m_AssianFontMap
		|| m_FontMap == m_ArabicFontMap )
		return 0;

	const int* letters = k_letters_italic; //m_italic ? k_letters_italic : k_letters_normal;

	currentChar = GetModuleByChar(currentChar);
	
	int kerning = 0;
	if( m_prevChar != -1 && currentChar != -1 )
		kerning = k_kerning[letters[m_prevChar * 2 + 1] * K_EDGE_TYPE_NO + letters[currentChar * 2 + 0]];

	m_prevChar = currentChar;

	return kerning;
#else
	return 0;
#endif
}

CSprite::CSprite(const char* fileName, int createFlags)
	: currentPalette(0),
	fontFrame(0),
	m_FontMap(m_NormalFontMap),
	lineSpacing(0),
	charSpacing(1),
	renderMode(RENDER_MODE_NORMAL),
	globalAlpha(GLOBAL_ALPHA_OPAQUE),
	palettes(0),
	numImages(0),
	numPalettes(0),
	kerningEnabled(true),
	kerningSpacing(1),
	m_italic(true),
	m_prevChar(-1),
	modules(0),
	frames(0),
	anims(0)
#ifdef USE_SINGLE_IMAGE
	,m_pixels(0)
	,m_indexedPixels(0)
	,m_alpha(0)
	#ifdef USE_OGL
	,m_glTexturesName(0)
	#endif // USE_OGL
#endif // USE_SINGLE_IMAGE
{
	createFlags |= SPRITE_FLAG_TEXTURE_SHORT;

	A_IFile* inputFile = A_IFile::Open(fileName, A_IFile::OPEN_BINARY | A_IFile::OPEN_READ, false);
	A_ASSERT(inputFile);
	if (inputFile!=0)
	{
		int type = 0;
		inputFile->Read(&type, 1);

		int bytesPerPixel;
		unsigned char moduleType;

		switch (type)
		{
		default:
		case 0x08:
			bytesPerPixel = 1;
			moduleType = 0x01;
			break;
		case 0x24:
			bytesPerPixel = 3;
			moduleType = 0x00;
			break;
		case 0x32:
			bytesPerPixel = 4;
			moduleType = 0x02;
			break;
		case 0x10:
			bytesPerPixel = 1;
			moduleType = 0x01 | 0x02;
			break;
		}

		inputFile->Read(&numModules,	sizeof(numModules));
		modules = NEW CModule*[numModules];

		for (int i=0; i<numModules; i++)
		{			
			unsigned char image;
			short width, height;			

			inputFile->Read(&image,		sizeof(image));

#ifdef USE_SINGLE_IMAGE
			unsigned short offsetX, offsetY;

			inputFile->Read(&offsetX,	sizeof(offsetX));
			inputFile->Read(&offsetY,	sizeof(offsetY));
#endif // USE_SINGLE_IMAGE

			inputFile->Read(&width,		sizeof(width));
			inputFile->Read(&height,	sizeof(height));

#ifdef USE_SINGLE_IMAGE
			modules[i] = NEW CModule(width, height, moduleType, offsetX, offsetY);
#else
			modules[i] = NEW CModule(width, height, moduleType);
#endif // USE_SINGLE_IMAGE

			modules[i]->image = image;
		}

		short numFModules;
		unsigned short	*fModuleIndex = NULL;
		short			*fModuleX = NULL;
		short			*fModuleY = NULL;
		unsigned char	*fModuleFlags = NULL;
		inputFile->Read(&numFModules,	sizeof(numFModules));

		if (numFModules > 0)
		{
			fModuleIndex = new unsigned short[numFModules];
			fModuleX = new short[numFModules];
			fModuleY = new short[numFModules];
			fModuleFlags = new unsigned char[numFModules];

			for (int i=0; i<numFModules; ++i)
			{
				inputFile->Read(&fModuleIndex[i],	sizeof(fModuleIndex[i]));
				inputFile->Read(&fModuleX[i],	sizeof(fModuleX[i]));
				inputFile->Read(&fModuleY[i],	sizeof(fModuleY[i]));
				inputFile->Read(&fModuleFlags[i],	sizeof(fModuleFlags[i]));
			}
		}

		inputFile->Read(&numFrames,		sizeof(numFrames));

		if (numFrames>0)
		{
			frames = NEW CFrame*[numFrames];

			for(int i=0; i<numFrames; i++)
			{
				short numFModules;
				short firstFModule;

				inputFile->Read(&numFModules,	sizeof(numFModules));
				inputFile->Read(&firstFModule,	sizeof(firstFModule));

				frames[i] = NEW CFrame(numFModules);

				for(int j=0; j<frames[i]->numFModules; j++)
				{
					int idx = firstFModule + j;
					frames[i]->fModules[j] = NEW CFModule;

					frames[i]->fModules[j]->module = modules[fModuleIndex[idx]];
					frames[i]->fModules[j]->xOffset= fModuleX[idx];
					frames[i]->fModules[j]->yOffset= fModuleY[idx];
					frames[i]->fModules[j]->flags  = fModuleFlags[idx];
				}
			}
		}

		SAFE_DELETE_ARRAY(fModuleIndex);
		SAFE_DELETE_ARRAY(fModuleX);
		SAFE_DELETE_ARRAY(fModuleY);
		SAFE_DELETE_ARRAY(fModuleFlags);

		short numAFrames;
		inputFile->Read(&numAFrames,	sizeof(numAFrames));		

		unsigned char	*aFrameIndex = NULL;
		unsigned char	*aFrameTime = NULL;
		short			*aFrameX = NULL;
		short			*aFrameY = NULL;
		unsigned char	*aFrameFlags = NULL;

		if (numAFrames > 0)
		{
			aFrameIndex = new unsigned char[numAFrames];
			aFrameTime = new unsigned char[numAFrames];
			aFrameX = new short[numAFrames];
			aFrameY = new short[numAFrames];
			aFrameFlags = new unsigned char[numAFrames];			

			for (int i=0; i<numAFrames; ++i)
			{
				inputFile->Read(&aFrameIndex[i],	sizeof(aFrameIndex[i]));
				inputFile->Read(&aFrameTime[i],		sizeof(aFrameTime[i]));
				inputFile->Read(&aFrameX[i],		sizeof(aFrameX[i]));
				inputFile->Read(&aFrameY[i],		sizeof(aFrameY[i]));
				inputFile->Read(&aFrameFlags[i],	sizeof(aFrameFlags[i]));
			}
		}

		inputFile->Read(&numAnims,		sizeof(numAnims));

		if (numAnims > 0)
		{
			anims = NEW CAnim*[numAnims];

			for(int i=0; i<numAnims; i++)
			{
				short numAFrames;
				short firstAFrame;

				inputFile->Read(&numAFrames,	sizeof(numAFrames));
				inputFile->Read(&firstAFrame,	sizeof(firstAFrame));

				anims[i] = NEW CAnim(numAFrames);

				for(int j=0; j<anims[i]->numAFrames; j++)
				{
					int idx = firstAFrame + j;
					anims[i]->aFrames[j] = NEW CAFrame;
					anims[i]->aFrames[j]->frame = frames[aFrameIndex[idx]];
					anims[i]->aFrames[j]->time = aFrameTime[idx];
					anims[i]->aFrames[j]->xOffset = aFrameX[idx];
					anims[i]->aFrames[j]->yOffset = aFrameY[idx];
					anims[i]->aFrames[j]->flags = aFrameFlags[idx];
				}
			}			
		}

		SAFE_DELETE_ARRAY(aFrameIndex);
		SAFE_DELETE_ARRAY(aFrameTime);
		SAFE_DELETE_ARRAY(aFrameX);
		SAFE_DELETE_ARRAY(aFrameY);
		SAFE_DELETE_ARRAY(aFrameFlags);

		short pixel_format;
		short data_format;
		unsigned char colors;

		if (bytesPerPixel == 1)
		{
			// Indexed image

#ifdef USE_SINGLE_IMAGE
			inputFile->Read(&width, sizeof(width));
			inputFile->Read(&height, sizeof(height));
#endif // USE_SINGLE_IMAGE

			inputFile->Read(&pixel_format, sizeof(pixel_format));
			inputFile->Read(&numImages,	sizeof(numImages));

			int l = 0;
			for (int k=0; k<numImages; k++)
			{
				inputFile->Read(&numPalettes,	sizeof(numPalettes));
				inputFile->Read(&colors,	sizeof(colors));

				int numColors = colors;
				if (numColors == 0)
					numColors = 256;

				if(numPalettes != 0)
				{
					if (palettes == NULL)
						palettes = NEW CAuroraPalette*[numPalettes * numImages];

					for(int i=0; i<numPalettes; i++)
					{
						palettes[l] = NEW CAuroraPalette(numColors);

						inputFile->Read(palettes[l]->data, sizeof(unsigned char) * (numColors*3));

						unsigned char*	data = palettes[l]->data;
						for (int c=0, t=0; c<numColors; c++, t+=3)
						{
							if (data[t] == 0xFF && data[t + 1] == 0 && data[t + 2] == 0xFF)
							{
								palettes[l]->alphaIndex = c;
								break;
							}
						}

						l++;
					}
				}
			}

			inputFile->Read(&data_format, sizeof(data_format));

#ifdef USE_SINGLE_IMAGE
			unsigned short sizeUShort; // = modules[i]->width * modules[i]->height;
			inputFile->Read(&sizeUShort, sizeof(sizeUShort));

			int size = width * height;

			m_indexedPixels = new unsigned char[size];
			inputFile->Read(m_indexedPixels, sizeof(unsigned char)	*size);

#ifdef USE_OGL

			m_glTexturesName = new GLuint[numPalettes];
	
			for(int i=0; i<numPalettes; i++)
			{
				m_bHasAlphaChannel = false;
				if (palettes[i]->alphaIndex >= 0)
				{
					m_bHasAlphaChannel = true;
				}

				//palettes[palettes[0]->alphaIndex] = 0;

				//prepare the texture buffer ... 
				//include this image in a texture with (width, height) pow2
				
				
				if(createFlags & SPRITE_FLAG_TEXTURE_GRAY)
				{
					g_lib3DGL->DecodeP888ToLUMINANCE(g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
								m_indexedPixels, width, height,
								palettes[i]->data);

					g_lib3DGL->CreateGLTexture(m_glTexturesName[i], 
								m_pow2Width, m_pow2Height, 
								g_lib3DGL->s_decodeBuffer, 
								GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE);
#ifdef DEBUG_TEX_MEM
					//to do ... here we have only one byte per pixel
					//m_glTexPixelType = GL_UNSIGNED_BYTE;
					m_videoMemSize = m_pow2Width * m_pow2Height;
#endif
				}
				else
				{
#ifdef USE_TEXTURES_SHORT_FORMATS				
					if (createFlags & SPRITE_FLAG_TEXTURE_SHORT)
					{
						if (m_bHasAlphaChannel)
						{
							g_lib3DGL->DecodeP888To5551(g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
								m_indexedPixels, width, height,
								palettes[i]->data, palettes[i]->alphaIndex);

							g_lib3DGL->CreateGLTexture(m_glTexturesName[i], 
								m_pow2Width, m_pow2Height, 
								g_lib3DGL->s_decodeBuffer, 
								GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
						}
						else
						{
							g_lib3DGL->DecodeP888To565(g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
								m_indexedPixels, width, height,
								palettes[i]->data);
							g_lib3DGL->CreateGLTexture(m_glTexturesName[i], 
								m_pow2Width, m_pow2Height, 
								g_lib3DGL->s_decodeBuffer,
								GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
						}
#ifdef DEBUG_TEX_MEM
						//m_glTexPixelType = GL_UNSIGNED_SHORT_5_6_5;
						m_videoMemSize = m_pow2Width * m_pow2Height * 2;
#endif
					}
					else
#endif // USE_TEXTURES_SHORT_FORMATS

					{
						g_lib3DGL->DecodeP888To8888(g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
							m_indexedPixels, width, height,
							palettes[i]->data, palettes[i]->alphaIndex);

						g_lib3DGL->CreateGLTexture(m_glTexturesName[i],
							m_pow2Width, m_pow2Height, 
							g_lib3DGL->s_decodeBuffer, 
							m_bHasAlphaChannel ? GL_RGBA : GL_RGB, m_bHasAlphaChannel ? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE
							);

#ifdef DEBUG_TEX_MEM
						//m_glTexPixelType = GL_UNSIGNED_BYTE;
						m_videoMemSize = m_pow2Width * m_pow2Height * (m_bHasAlphaChannel ? 4 : 3);
#endif
					}
				}

				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				
				#if defined(DEBUG_SPRITE_GLTEXTURE_ALLOC) && defined(_DEBUG)
					s16 key = 0;
					CreateKeyForGLTexName(key, m_glTexturesName[i]);
				#endif //DEBUG_SPRITE_GLTEXTURE_ALLOC
			}

			SAFE_DELETE_ARRAY(m_indexedPixels);

			

#endif /* USE_OGL*/

#else
			for (int i=0; i<numModules; i++)
			{
				unsigned short size; // = modules[i]->width * modules[i]->height;
				inputFile->Read(&size, sizeof(size));
				inputFile->Read(modules[i]->indexedPixels, sizeof(unsigned char)	*size);
			}

			// Load the mask
			if (type == 0x10)
			{
				char npMask;
				char npCols;
				inputFile->Read(&npMask,	sizeof(numPalettes)); // skip num images
				inputFile->Read(&npMask,	sizeof(numPalettes));
				inputFile->Read(&npCols,	sizeof(colors));

				int numColors = npCols & 0xFF;
				if (numColors == 0)
					numColors = 256;

				A_ASSERT(npMask == 1);

				unsigned char *dataPal = new unsigned char[npMask * numColors * 3];
				inputFile->Read(dataPal, sizeof(unsigned char) * (numColors*3));

				unsigned char *data;
				for (int i=0; i<numModules; i++)
				{
					unsigned short size;// = modules[i]->width * modules[i]->height;
					inputFile->Read(&size, sizeof(size));
					data = new unsigned char[size];
					inputFile->Read(data, sizeof(unsigned char)	*size);

					for (int k=0; k < size; ++k)
					{
						//int t = data[k] * 3;
						//int color = (dataPal[t] + dataPal[t + 1] + dataPal[t + 2]) / 3;
						int shift = ((~k & 1) << 2);
						int mask  = (0x0F << shift);
						modules[i]->alpha[k >> 1] ^= (modules[i]->alpha[k >> 1] & mask);
						//modules[i]->alpha[k >> 1] |= ((color >> 4) & 0x0F) << shift;
						modules[i]->alpha[k >> 1] |= ((data[k] >> 4) & 0x0F) << shift;
					}

					delete[] data;
				}

				delete[] dataPal;
			}
#endif // USE_SINGLE_IMAGE
		}
		else
		{
#ifdef USE_SINGLE_IMAGE
			inputFile->Read(&width, sizeof(width));
			inputFile->Read(&height, sizeof(height));

			unsigned int sizeUInt; // = modules[i]->width * modules[i]->height;
			inputFile->Read(&sizeUInt, sizeof(sizeUInt));

			int size = (width * height) * bytesPerPixel;

			unsigned char *data = new unsigned char[size];
			inputFile->Read(data, size);

#ifdef USE_OGL
	
			m_glTexturesName = new GLuint[1];			

			//prepare the texture buffer ... 
			//include this image in a texture with (width, height) pow2

			if (bytesPerPixel == 4)
			{
				m_bHasAlphaChannel = true;

				g_lib3DGL->Decode8888To8888( g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
											data, width, height);

				g_lib3DGL->CreateGLTexture(m_glTexturesName[0], 
											m_pow2Width, m_pow2Height, 
											g_lib3DGL->s_decodeBuffer, 
											GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE
											);
			}
			else
			{
				m_bHasAlphaChannel = false;

				g_lib3DGL->Decode888To8888( g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
											data, width, height);

				g_lib3DGL->CreateGLTexture(m_glTexturesName[0], 
											m_pow2Width, m_pow2Height, 
											g_lib3DGL->s_decodeBuffer, 
											GL_RGB, GL_RGB, GL_UNSIGNED_BYTE
											);
			}
			
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			#if defined(DEBUG_SPRITE_GLTEXTURE_ALLOC) && defined(_DEBUG)
				s16 key = 0;
				CreateKeyForGLTexName(key, m_glTexturesName[0]);
			#endif //DEBUG_SPRITE_GLTEXTURE_ALLOC

			#ifdef DEBUG_TEX_MEM
				//m_glTexPixelType = GL_UNSIGNED_BYTE;
				m_videoMemSize = m_pow2Width * m_pow2Height * (m_bHasAlphaChannel ? 4 : 3);
			#endif	

#else //USE_OGL
			size /= bytesPerPixel;

			m_pixels = new unsigned short[size];

			int offset = 0;
			if (bytesPerPixel == 4)
			{
				m_alpha = new unsigned char[size];
				offset = 1;
			}
			
			for (unsigned int k=0, t=0; k<size; k++, t += bytesPerPixel)
			{
				m_pixels[k] = ((data[t + offset + 0] >> 3) << 11);
				m_pixels[k] = m_pixels[k] | ((data[t + offset + 1] >> 2) << 5);
				m_pixels[k] = m_pixels[k] | ((data[t + offset + 2] >> 3));

				if (m_alpha)
				{
					int shift = ((~k & 1) << 2);
					int mask  = (0x0F << shift);
					m_alpha[k >> 1] ^= (m_alpha[k >> 1] & mask);
					m_alpha[k >> 1] |= ((data[t] >> 4) & 0x0F) << shift;
				}
			}

#endif /* USE_OGL*/

			delete[] data;
#else //USE_SINGLE_IMAGE

			inputFile->Read(&pixel_format, sizeof(pixel_format));
			inputFile->Read(&data_format, sizeof(data_format));

			for (int i=0; i<numModules; i++)
			{
				unsigned int size; // = modules[i]->width * modules[i]->height;
				inputFile->Read(&size, sizeof(size));

				size = modules[i]->width * modules[i]->height * bytesPerPixel;

				unsigned char *data = new unsigned char[size];
				inputFile->Read(data, size);

				size /= bytesPerPixel;

				int offset = 0;
				if (bytesPerPixel == 4)
				{
					offset = 1;
				}

				for (unsigned int k=0, t=0; k<size; k++, t += bytesPerPixel)
				{
					modules[i]->pixels[k] = ((data[t + offset + 0] >> 3) << 11);
					modules[i]->pixels[k] = modules[i]->pixels[k] | ((data[t + offset + 1] >> 2) << 5);
					modules[i]->pixels[k] = modules[i]->pixels[k] | ((data[t + offset + 2] >> 3));

					if (bytesPerPixel == 4)
					{
						int shift = ((~k & 1) << 2);
						int mask  = (0x0F << shift);
						modules[i]->alpha[k >> 1] ^= (modules[i]->alpha[k >> 1] & mask);
						modules[i]->alpha[k >> 1] |= ((data[t] >> 4) & 0x0F) << shift;
					}
				}

				delete[] data;
			}
#endif // USE_SINGLE_IMAGE
		}
	}
	A_IFile::Close(inputFile);
}

CSprite::~CSprite()
{
	for(int i=0; i<numFrames; i++)
		SAFE_DELETE(frames[i]);

	SAFE_DELETE_ARRAY(frames);

	for(int i=0; i<numModules; i++)
		SAFE_DELETE(modules[i]);

	SAFE_DELETE_ARRAY(modules);

	for(int i=0; i<numPalettes * numImages; i++)
		SAFE_DELETE(palettes[i]);

	for(int i=0; i<numAnims; i++)
		SAFE_DELETE(anims[i]);

	SAFE_DELETE_ARRAY(anims);
	
	SAFE_DELETE_ARRAY(palettes);

#ifdef USE_SINGLE_IMAGE
	SAFE_DELETE_ARRAY(m_pixels);
	SAFE_DELETE_ARRAY(m_indexedPixels);
	SAFE_DELETE_ARRAY(m_alpha);
#endif // USE_SINGLE_IMAGE

#ifdef USE_OGL
	//release the gl texture name only if this is the original texture( not a reference )
	if(numPalettes > 0)
	{
		for(int i=0; i<numPalettes; i++)
		{
			g_lib3DGL->ReleaseGLTexture(m_glTexturesName[i]);
			
			#ifdef DEBUG_TEX_MEM
				#ifdef DEBUG_SPRITE_GLTEXTURE_ALLOC
					FreeKeyForGLTexName(m_glTexturesName[i]);
				#endif //DEBUG_SPRITE_GLTEXTURE_ALLOC
				
				//int bytesPerPixel = ( m_bHasAlphaChannel ? 4 : 3);

				//if(m_glTexPixelType == GL_UNSIGNED_SHORT_5_6_5 || m_glTexPixelType == GL_UNSIGNED_SHORT_5_5_5_1)
				//{
				//	bytesPerPixel = 2;
				//}
				//
				//int texsize = m_pow2Width * m_pow2Height * bytesPerPixel;				

				g_TexTotalTexSize -= m_videoMemSize; // texsize;
				debug_out("ASprite RELEASE[%d] TOTAL_SIZE:[%d] [%d, %d] [alpha: %d]\n", m_glTexturesName[i], (g_TexTotalTexSize / 1024), m_pow2Width, m_pow2Height, m_bHasAlphaChannel);
			#endif //DEBUG_TEX_MEM
		}
	}
	else
	{
		g_lib3DGL->ReleaseGLTexture(m_glTexturesName[0]);	
		
		#ifdef DEBUG_TEX_MEM
			#ifdef DEBUG_SPRITE_GLTEXTURE_ALLOC		
				FreeKeyForGLTexName(m_glTexturesName[0]);
			#endif //DEBUG_SPRITE_GLTEXTURE_ALLOC

			//int bytesPerPixel = ( m_bHasAlphaChannel ? 4 : 3);

			//if(m_glTexPixelType == GL_UNSIGNED_SHORT_5_6_5 || m_glTexPixelType == GL_UNSIGNED_SHORT_5_5_5_1)
			//{
			//	bytesPerPixel = 2;
			//}

			//int texsize = m_pow2Width * m_pow2Height * bytesPerPixel;	

			g_TexTotalTexSize -= m_videoMemSize; // texsize;
			debug_out("ASprite RELEASE[%d] TOTAL_SIZE:[%d] [%d, %d] [alpha: %d]\n", m_glTexturesName[0], (g_TexTotalTexSize / 1024), m_pow2Width, m_pow2Height, m_bHasAlphaChannel);
		#endif //DEBUG_TEX_MEM
	}
	SAFE_DELETE_ARRAY(m_glTexturesName);
#endif /* USE_OGL*/
}

void CSprite::DrawAFrame (CLib2D& lib2d, int x, int y, int anim, int aframe) const
{
	CAFrame* afrm = anims[anim]->aFrames[aframe];
	CFrame* frame = afrm->frame;

	DrawFrame(lib2d, x + afrm->xOffset, y + afrm->yOffset, frame, afrm->flags);
}

int CSprite::GetNumAFrames (int anim) const
{
	return anims[anim]->numAFrames;
}

int CSprite::GetAFrameTime(int anim, int aframe) const
{
	return anims[anim]->aFrames[aframe]->time;
}

void CSprite::DrawFrame(CLib2D& lib2d, int x, int y, int frame, int flags, int rotation) const
{
	CFrame* f = frames[frame];

	for(int i = 0; i<f->numFModules; ++i)
	{
		CFModule* cfm = f->fModules[i];

		DrawModule(lib2d, x + cfm->xOffset, y + cfm->yOffset, cfm->module, cfm->flags ^ flags, rotation, x, y);
	}
}

void CSprite::DrawFrame(CLib2D& lib2d, int x, int y, CFrame *f, int flags, int rotation) const
{
	for(int i = 0; i<f->numFModules; ++i)
	{
		CFModule* cfm = f->fModules[i];

//parameters modified for rotationCenterX and rotationCenterY
		DrawModule(lib2d, x + cfm->xOffset, y + cfm->yOffset, cfm->module, cfm->flags ^ flags, rotation, x/*cfm->xOffset*/, y/*cfm->yOffset*/);
	}
}

short CSprite::GetFrameWidth(int frame) const
{
	CFrame* f = frames[frame];

	int x1 = 9999;
	int x2 = -9999;

	for(int i = 0; i<f->numFModules; ++i)
	{
		CFModule* cfm = f->fModules[i];

		if (cfm->xOffset < x1)
			x1 = cfm->xOffset;
		if (cfm->xOffset + cfm->module->width > x2)
			x2 = cfm->xOffset + cfm->module->width;
	}

	return x2 - x1;
}

short CSprite::GetFrameHeight(int frame) const
{
	CFrame* f = frames[frame];

	int y1 = 9999;
	int y2 = -9999;

	for(int i = 0; i<f->numFModules; ++i)
	{
		CFModule* cfm = f->fModules[i];

		if (cfm->yOffset < y1)
			y1 = cfm->yOffset;
		if (cfm->yOffset + cfm->module->height > y2)
			y2 = cfm->yOffset + cfm->module->height;
	}

	return y2 - y1;
}

void CSprite::GetAFrameRect(int anim, int aframe, int &x, int &y, int &w, int &h) const
{
	CAFrame* afrm = anims[anim]->aFrames[aframe];
	CFrame* f = afrm->frame;

	int x1 = 9999;
	int x2 = -9999;
	int y1 = 9999;
	int y2 = -9999;

	for(int i = 0; i<f->numFModules; ++i)
	{
		CFModule* cfm = f->fModules[i];

		if (cfm->xOffset < x1)
			x1 = cfm->xOffset;
		if (cfm->xOffset + cfm->module->width > x2)
			x2 = cfm->xOffset + cfm->module->width;

		if (cfm->yOffset < y1)
			y1 = cfm->yOffset;
		if (cfm->yOffset + cfm->module->height > y2)
			y2 = cfm->yOffset + cfm->module->height;
	}

	x = x1 + afrm->xOffset;
	y = y1 + afrm->yOffset;
	w = x2 - x1;
	h = y2 - y1;
}

void CSprite::DrawModule(CLib2D& lib2d, int x, int y, int module, int flags, int rotation, int rotCenterX, int rotCenterY) const
{
	DrawModule(lib2d, x, y, modules[module], flags, rotation, rotCenterX, rotCenterY);
}

#define SWAP_SHORT(a,b) {short t = a; a = b; b = t;}
#ifdef USE_OGL
	#define SWAP_FLOAT(a,b) {f32 t = a; a = b; b = t;}
#endif /* USE_OGL */

void CSprite::DrawModule(CLib2D& lib2d, int x, int y, CModule* module, int flags, int rotation, int rotCenterX, int rotCenterY) const
{
	int srcStartX = 0,	srcEndX = module->width;
	int srcStartY = 0,	srcEndY = module->height;

#ifdef USE_SINGLE_IMAGE

	srcStartX = module->x;
	srcStartY = module->y;

	srcEndX += srcStartX;
	srcEndY += srcStartY;

#endif // USE_SINGLE_IMAGE

	int srcWidth	= module->width;
	int srcHeight	= module->height;

	unsigned short * pScreen = (unsigned short *) lib2d.GetDestPtr();
	int destWidth	= lib2d.GetDestWidth();
	int destHeight	= lib2d.GetDestHeight();

	if (lib2d.GetClipX() == -1)
	{
		lib2d.SetClip(0, 0, destWidth, destHeight);
	}

	int clipStartX = lib2d.GetClipX(), clipEndX = lib2d.GetClipX() + lib2d.GetClipWidth();
	int clipStartY = lib2d.GetClipY(), clipEndY = lib2d.GetClipY() + lib2d.GetClipHeight();

	A_ASSERT(clipStartX >= 0);
	A_ASSERT(clipStartY >= 0);
	A_ASSERT(clipEndX <= destWidth);
	A_ASSERT(clipEndY <= destHeight);

	if (rotation == 0 && (flags & FLAGS_ROT_90)==0)
	{
		if(x < clipStartX) {
			int clipped = (clipStartX - x);
			srcWidth -= clipped;
			srcStartX += clipped;
			x = clipStartX;
		}

		if(y < clipStartY) {
			int clipped = (clipStartY - y);
			srcHeight -= clipped;
			srcStartY += clipped;
			y = clipStartY;
		}

		if(x + srcWidth > clipEndX) {
			int clipped = ((x + srcWidth) - clipEndX);
			srcWidth -= clipped;
			srcEndX -= clipped;
		}

		if(y + srcHeight > clipEndY) {
			int clipped = ((y + srcHeight) - clipEndY);
			srcHeight -= clipped;
			srcEndY -= clipped;
		}
	}

	// Completely outside of the screen
	if((srcWidth <= 0) || (srcHeight <= 0))
		return;

#ifdef USE_OGL
	
	f32 uv[4];
	uv[0] = (f32)srcStartX / m_pow2Width;
	uv[1] = (f32)srcStartY / m_pow2Height;
	uv[2] = (f32)srcEndX / m_pow2Width;
	uv[3] = (f32)srcEndY / m_pow2Height;

	if (flags & FLAGS_FLIP_X)
	{
		SWAP_FLOAT(uv[0], uv[2]);
	}

	if (flags & FLAGS_FLIP_Y)
	{
		SWAP_FLOAT(uv[1], uv[3]);
	}
	
	if (flags & FLAGS_ROT_90)
	{
		int moduleHeight = srcHeight;

		if (rotation == 0)
		{
			// TODO: start clip
			if (y + srcWidth > clipEndY) {
				int clipped = ((y + srcWidth) - clipEndY);
				srcWidth -= clipped;
				srcStartX += clipped;

				if (flags & FLAGS_FLIP_X)
					uv[2] = (f32)srcStartX / m_pow2Width;
				else
					uv[0] = (f32)srcStartX / m_pow2Width;
			}

			if (x + srcHeight > clipEndX) {
				int clipped = ((x + srcHeight) - clipEndX);
				srcHeight -= clipped;
				srcStartY += clipped;

				if (flags & FLAGS_FLIP_Y)
					uv[3] = (f32)srcStartY / m_pow2Height;
				else
					uv[1] = (f32)srcStartY / m_pow2Height;
			}
		}

		rotation += ANGLEPI / 2;
		rotCenterX = x + moduleHeight;
		rotCenterY = y;
		x += moduleHeight;
	}

	int appearanceFlag = 0;

	if(flags & FLAGS_ADDITIVE_BLENDING)
		appearanceFlag = FLAG_USE_ADDITIVE_BLEND;

	g_lib3DGL->paint2DModule(x, y, srcEndX - srcStartX, srcEndY - srcStartY, m_glTexturesName[currentPalette], uv, appearanceFlag, rotation, rotCenterX, rotCenterY);

	return ;

#endif /* USE_OGL */

	int destDx = 1;
	int destDy = destWidth - srcWidth;

	if (flags & FLAGS_FLIP_X)
	{
		srcEndX = module->width - srcStartX;
		srcStartX = srcEndX - srcWidth;

		SWAP_SHORT(srcStartX, srcEndX);

		srcStartX--;
		srcEndX--;
	}

	if (flags & FLAGS_FLIP_Y)
	{
		srcEndY = module->height - srcStartY;
		srcStartY = srcEndY - srcHeight;

		SWAP_SHORT(srcStartY, srcEndY);

		srcStartY--;
		srcEndY--;
	}

	if (flags & FLAGS_ROT_90)
	{
		srcEndY = module->height - srcStartY;
		srcStartY = srcEndY - srcHeight;

		SWAP_SHORT(srcStartY, srcEndY);
		srcStartY--;
		srcEndY--;

		destDx = destWidth;
		destDy = 1 - (srcWidth * destWidth);
	}

	int srcDx = srcStartX < srcEndX ? 1 : -1;
	int srcDy = srcStartY < srcEndY ? 1 : -1;
#ifdef USE_SINGLE_IMAGE
	int srcOffset = srcStartY * width;
#else
	int srcOffset = srcStartY * module->width;
#endif // USE_SINGLE_IMAGE

	pScreen += (y * destWidth + x);

	for(int yy = srcStartY; yy != srcEndY; yy += srcDy)
	{
		for(int xx = srcStartX; xx != srcEndX; xx += srcDx)
		{
			int offset = srcOffset + xx;

#define COLOR_R(color)	(((color) & 0xF800) >> 11)
#define COLOR_G(color)	(((color) & 0x07E0) >> 6)
#define COLOR_B(color)	(((color) & 0x001F) >> 0)

#define MAKE_COLOR(r,g,b)	((((r) << 11) & 0xF800) | \
							 (((g) <<  6) & 0x07E0) | \
							 (((b) <<  0) & 0x001F) )

			short pixel;
			short alpha = 0x0F;

			if (module->useIndexedPixels)
			{
				int index;

#ifdef USE_SINGLE_IMAGE
				
				index = m_indexedPixels[offset];

#else
				if (module->useLowColor)
					index = (module->indexedPixels[offset >> 1] >> ((~offset & 0x01) << 2)) & 0x0F;
				else
					index = module->indexedPixels[offset];

#endif // USE_SINGLE_IMAGE

				int pal = numPalettes * module->image + currentPalette;

				if (index == palettes[pal]->alphaIndex)
					alpha = 0;
				
				index *= 3;

				//pixel = MAKE_COLOR(
				//	palettes[currentPalette]->data[index+0],
				//	palettes[currentPalette]->data[index+1],
				//	palettes[currentPalette]->data[index+2]);

				pixel = ((palettes[pal]->data[index+2] >> 3) << 11);
				pixel = pixel | ((palettes[pal]->data[index+1] >> 2) << 5);
				pixel = pixel | ((palettes[pal]->data[index+0] >> 3));
			}
			else
			{
#ifdef USE_SINGLE_IMAGE

				pixel = m_pixels[offset];
				
#else
				pixel = module->pixels[offset];

#endif // USE_SINGLE_IMAGE

			}

#ifdef USE_SINGLE_IMAGE
			if (m_alpha)
				alpha = (m_alpha[offset >> 1] >> ((~offset & 0x01) << 2)) & 0x0F;

#else
			if (module->useAlpha)
			{
				alpha = (module->alpha[offset >> 1] >> ((~offset & 0x01) << 2)) & 0x0F;

				//alpha = module->alpha[offset];
			}
#endif // USE_SINGLE_IMAGE


			if(globalAlpha != GLOBAL_ALPHA_OPAQUE)
			{
				alpha = (alpha * globalAlpha) >> 4;
			}

			switch(renderMode)
			{
				case RENDER_MODE_GRAYSCALE:
				{
					int r = (COLOR_R(pixel) * 38) >> 7;
					int g = (COLOR_G(pixel) * 76) >> 7;
					int b = (COLOR_B(pixel) * 14) >> 7;

					int t = r+g+b;

					pixel = MAKE_COLOR(t,t,t);
				}
				break;
				case RENDER_MODE_SEPIA:
				{
					int r = COLOR_R(pixel);
					int g = COLOR_G(pixel);
					int b = COLOR_B(pixel);

					int rs = (r * 393 + g * 769 + b * 189) / 1351;
					int gs = (r * 349 + g * 686 + b * 168) / 1203;
					int bs = (r * 272 + g * 534 + b * 131) / 2140;

					pixel = MAKE_COLOR(rs,gs,bs);
				}
				break;
				case RENDER_MODE_NEGATIVE:
				{
					int r = ~COLOR_R(pixel);
					int g = ~COLOR_G(pixel);
					int b = ~COLOR_B(pixel);

					pixel = MAKE_COLOR(r,g,b);
				}
				break;
				case RENDER_MODE_SHADOW:
				{
					pixel = kRenderModeShadowColor;
				}
				break;

			}

//#define SKIP_ALPHA_PROCESSING

#ifndef SKIP_ALPHA_PROCESSING
 			if (alpha == 0)
			{
				// empty
			}
			else if (alpha == 0xF)
			{
#endif
 				*pScreen = pixel;
#ifndef SKIP_ALPHA_PROCESSING
			}
			else
			{
				//*pScreen = CLib2D::MixColor(pixel, *pScreen, alpha>>4);
				*pScreen = CLib2D::MixColor(pixel, *pScreen, alpha);
			}
#endif

			pScreen += destDx;
		}

		pScreen += destDy;

#ifdef USE_SINGLE_IMAGE
		srcOffset += width * srcDy;
#else
		srcOffset += module->width * srcDy;
#endif // USE_SINGLE_IMAGE
	}
}

// Returns the number of lines
int CSprite::FormatStringWithLimitedWidth( const unsigned short* s, unsigned short* buf, int maxWidth ,  bool ignoreNewLines, bool __ignoreSpaces)
{
	int nLines = 1;

	int charLength = strlen(s);
	A_ASSERT(charLength<MAX_STRING_LENGTH);
	strcpy(buf, s);

	const int wordSpacing = frames[fontFrame]->fModules[0]->module->width + charSpacing;

	int lineWidth = -wordSpacing;
	const char spaceChar=' ';
	for (int i = 0, j = -1; i <= charLength; i++)	
	{
		if ( (buf[i] == spaceChar || buf[i] == 0 || buf[i] == '\n' || __ignoreSpaces || buf[i] == '|') && 
			 (i == charLength || (buf[i+1] != '!')) )
		{
			if (j + 1 == i)
			{
				while (buf[i] == '\n' || (buf[i] == spaceChar && __ignoreSpaces )||  buf[i] == '|')
				{
					for (int idxMove = i; idxMove < charLength; idxMove++)
					{
						buf[idxMove] = buf[idxMove+1];
					}
					charLength--;
				}
				if(!__ignoreSpaces)
				continue;
			}

			// French
			if (buf[i] == spaceChar)
			{
				while (buf[i+1] != 0 && (buf[i+1] == '!' || buf[i+1] == '?' || buf[i+1] == '.'))
					i++;
			}

			MeasureString(buf + j + 1, buf + i + (__ignoreSpaces?1:0));
			lineWidth += lastStringWidth + charSpacing + wordSpacing;
			if (lineWidth > maxWidth)
			{
				if (j >= 0)
					buf[j] = '\n';
				lineWidth = lastStringWidth + charSpacing;
				++nLines;
			}

			if (buf[i] == '\n')
			{
				if (ignoreNewLines)
				{
					buf[i] = spaceChar;
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

int	CSprite::GetStringWidthLimited(const unsigned short* s, int maxWidth ,  bool ignoreNewLines)
{
	unsigned short buf[MAX_STRING_LENGTH];
	FormatStringWithLimitedWidth(s,buf,maxWidth,ignoreNewLines);
	return GetStringWidth(buf);
}
int	CSprite::GetStringHeightLimited(const unsigned short* s, int maxWidth ,  bool ignoreNewLines)
{
	unsigned short buf[MAX_STRING_LENGTH];
	FormatStringWithLimitedWidth(s,buf,maxWidth,ignoreNewLines);
	return GetStringHeight(buf);
}


int CSprite::DrawStringWithLimitedWidth(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth, bool ignoreNewLines, Align align, bool __ignoreSpaces)
{
	unsigned short buf[MAX_STRING_LENGTH];

    int nLines = FormatStringWithLimitedWidth(s, buf, maxWidth, ignoreNewLines, __ignoreSpaces);

	DrawString(lib2d, x, y, buf, align);

	return nLines;
}

void CSprite::DrawStringWithLimitedWidthByAdjustingCharSpacing(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth ,  Align align )
{
	int initialCharSpacing = GetCharSpacing();
	//Fits the text into the screen
	while (GetCharSpacing()>-5 && //ensures it stops at -5
		GetStringWidth(s) > (maxWidth))
	{
		SetCharSpacing(GetCharSpacing()-1);
	}
	DrawString(lib2d,x,y,s,align);
	SetCharSpacing(initialCharSpacing);
}
void CSprite::DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, Align align) 
{
	const unsigned short * stringStart=s;
	const unsigned short * stringEnd=s;
	int yy=y;

	int lineCount = 1;
	while (*stringStart != 0)
	{
		if (*stringStart == '\n')
			lineCount++;

		stringStart++;
	}

	if(align & VALIGN_CENTER)
		yy -= (lineCount - 1) * (GetFontHeight() + lineSpacing) / 2;
	else if(align & VALIGN_BOTTOM)
		yy -= (lineCount - 1) * (GetFontHeight() + lineSpacing);

	stringStart = s;
	while (*stringStart != 0)
	{
		while(*stringEnd && *stringEnd != '\n')
			stringEnd++;

		DrawString(lib2d, x, yy, stringStart, stringEnd, align);
		yy += GetFontHeight() + lineSpacing;
		if (*stringEnd==0) break;
		stringEnd++;
		stringStart=stringEnd;
	}
}

void CSprite::DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, const unsigned short* lastCharacter, Align align) {

	MeasureString(s,lastCharacter);

	int xx = x, yy = y;

	if(align & VALIGN_BOTTOM)
		yy -= lastStringHeight;
	else if(align & VALIGN_CENTER)
		yy -= (lastStringHeight >> 1);

	if(align & HALIGN_RIGHT)
		xx -= lastStringWidth;
	else if(align & HALIGN_CENTER)
		xx -= (lastStringWidth >> 1);

	int bx = xx;
	m_prevChar = -1;

	while((*s !=0) && 
		   (s !=lastCharacter)) 
	{
		if(*s == '\n') 
		{
			xx = bx;
			yy += GetFontHeight() + lineSpacing;
			m_prevChar = -1;
		} 
		else if(*s == ' ')
		{
			xx += frames[fontFrame]->fModules[0]->module->width + charSpacing;
		} 
		else 
		{
			CFModule* cfm = frames[fontFrame]->fModules[GetModuleByChar(*s)];
			//int kerningSpacing = GetKerningSpacing(*s);
			xx += GetKerningSpacing(*s);
			if(!((m_FontMap == m_AssianFontMap) && (*s == '|')))
				DrawModule(lib2d, xx + cfm->xOffset+kerningSpacing, yy + cfm->yOffset, cfm->module, cfm->flags);

			xx += cfm->module->width + charSpacing /*+kerningSpacing*/;
		}

		s++;
	}
}

void CSprite::DrawNumber(CLib2D& lib2d, int x, int y, int number, Align align)
{
	MeasureNumber(number);

	const unsigned nb_ = number;
	unsigned digit_count = 1;
	unsigned nDigits = 0;

	if (number == 0)
	{		
		digit_count = 10;
		nDigits = 1;
	}
	else
	{
		while (number > 0)
		{
			digit_count *= 10;
			number /= 10;
			nDigits++;
		}
	}
	

	int xx = x, yy = y;

	if(align & VALIGN_BOTTOM)
		yy -= lastNumberHeight;
	else if(align & VALIGN_CENTER)
		yy -= (lastNumberHeight >> 1);

	if(align & HALIGN_RIGHT)
		xx -= lastNumberWidth;
	else if(align & HALIGN_CENTER)
		xx -= (lastNumberWidth >> 1);

	number = nb_;
	while (digit_count > 1)
	{
		digit_count /= 10;
		int digit = (number / digit_count) % 10;

		CFModule* cfm = frames[fontFrame]->fModules[1 + digit];
		DrawModule(lib2d, xx + cfm->xOffset, yy + cfm->yOffset, cfm->module, cfm->flags);
		xx += cfm->module->width + charSpacing;
	}
}

void CSprite::DrawNumberString(CLib2D& lib2d, int x, int y, const unsigned short* s, Align align) 
{
	const unsigned short * stringStart=s;
	const unsigned short * stringEnd=s;
	int yy=y;
	while (*stringStart != 0)
	{
		while(*stringEnd!='\n' && //New line character?
			  *stringEnd!=0 //Or is it end of string
			  ) stringEnd++;
		DrawNumberString(lib2d, x, yy, stringStart, stringEnd,align);
		yy += GetFontHeight() + lineSpacing;
		if (*stringEnd==0) break;
		stringEnd++;
		stringStart=stringEnd;
	}
}

void CSprite::DrawNumberString(CLib2D& lib2d, int x, int y, const unsigned short* s, const unsigned short* lastCharacter, Align align) {

	MeasureNumberString(s,lastCharacter);

	int xx = x, yy = y;

	if(align & VALIGN_BOTTOM)
		yy -= lastStringHeight;
	else if(align & VALIGN_CENTER)
		yy -= (lastStringHeight >> 1);

	if(align & HALIGN_RIGHT)
		xx -= lastStringWidth;
	else if(align & HALIGN_CENTER)
		xx -= (lastStringWidth >> 1);

	int bx = xx;
	m_prevChar = -1;

	while((*s !=0) && 
		   (s !=lastCharacter)) 
	{
		if(*s == '\n') 
		{
			xx = bx;
			yy += GetFontHeight() + lineSpacing;
			m_prevChar = -1;
		} 
		else if(*s == ' ') 
		{
			xx += frames[fontFrame]->fModules[0]->module->width + charSpacing;
		} 
		else 
		{
			CFModule* cfm = frames[fontFrame]->fModules[GetNumberModuleByChar(*s)];
			
			//int kerningSpacing = GetKerningSpacing(*s);
			xx += GetKerningSpacing(*s);
			DrawModule(lib2d, xx + cfm->xOffset+kerningSpacing, yy + cfm->yOffset, cfm->module, cfm->flags);

			xx += cfm->module->width + charSpacing /*+kerningSpacing*/;
		}

		s++;
	}
}

short CSprite::GetStringWidth(const unsigned short* s) 
{
	MeasureString(s);

	return lastStringWidth;
}

short CSprite::GetStringHeight(const unsigned short* s) 
{
	MeasureString(s);

	return lastStringHeight;
}

int CSprite::GetModuleByChar(unsigned short c) const
{
	if (m_FontMap == m_AssianFontMap)
	{
		// perform a binary search
		const unsigned short* p = m_FontMap;
		const unsigned short* q = m_FontMap + m_AssianFontMapSize;
		const unsigned short* r;

		while (p < q - 1)
		{
			r = p + (q - p) / 2;
			
			if (*r <= c)
				p = r;
			else
				q = r;
		}

		A_ASSERT(c == *p);
		
		return p - m_FontMap;
	}
	else if (m_FontMap == m_ArabicFontMap)
	{
		// perform a binary search
		const unsigned short* p = m_FontMap;
		const unsigned short* q = m_FontMap + m_ArabicFontMapSize;
		const unsigned short* r;

		while (p < q - 1)
		{
			r = p + (q - p) / 2;

			if (*r <= c)
				p = r;
			else
				q = r;
		}

		A_ASSERT(c == *p);

		return p - m_FontMap;
	}
	else
	{
		// rax: quick fix for TM character :|
		if (c == 0x2122)
			c = 153;
		return m_FontMap[c];
	}
}

void CSprite::MeasureString(const unsigned short* stringToMeasure,const unsigned short* lastCharacter)
{
	int w = 0, lines = 1;

	lastStringWidth = 0;
	m_prevChar = -1;

	while((*stringToMeasure!=0) && 
		   (stringToMeasure!=lastCharacter)) 
	{
		if(*stringToMeasure == '\n') 
		{
			lastStringWidth = MATH_MAX(lastStringWidth, w);
			w = 0;
			lines++;
		}
		else if(*stringToMeasure == ' ') 
		{
			w += frames[fontFrame]->fModules[0]->module->width + charSpacing;
		}
		else 
		{
			w += frames[fontFrame]->fModules[GetModuleByChar(*stringToMeasure)]->module->width + charSpacing + GetKerningSpacing(*stringToMeasure);
		}

		stringToMeasure++;
	}

	lastStringWidth = MATH_MAX(lastStringWidth, w) - charSpacing; //LAngelov: Heavy patch for centering
	lastStringHeight = lines * GetFontHeight() + (lines - 1) * lineSpacing;
}

short CSprite::GetNumberWidth(int number) {
	MeasureNumber(number);

	return lastNumberWidth;
}

short CSprite::GetNumberHeight(int number) {
	MeasureNumber(number);

	return lastNumberHeight;
}

void CSprite::MeasureNumber(int number)
{
	lastNumberWidth = 0;
//#ifdef WIN_DEBUG
//	// rax - remove
//	if (number < 0)
//		return;
//#endif
	
	do
	{
		int digit = number % 10;

		CFModule* cfm = frames[fontFrame]->fModules[1 + digit];
		lastNumberWidth += cfm->module->width + charSpacing;

		number /= 10;
	} while (number);

	lastNumberHeight = GetFontHeight();
}

short CSprite::GetNumberStringWidth(const unsigned short* s) 
{
	MeasureNumberString(s);

	return lastStringWidth;
}

short CSprite::GetNumberStringHeight(const unsigned short* s) 
{
	MeasureNumberString(s);

	return lastStringHeight;
}


void CSprite::ReplaceWidthChar(int charSpacing)
{
	if(m_FontMap == m_AssianFontMap)
	{
		frames[fontFrame]->fModules[GetModuleByChar('|')]->module->width = - charSpacing;
		frames[fontFrame]->fModules[GetModuleByChar(' ')]->module->width = 9;
	}
}
void CSprite::MeasureNumberString(const unsigned short* stringToMeasure,const unsigned short* lastCharacter) {
	int w = 0, lines = 1;

	lastStringWidth = 0;
	m_prevChar = -1;

	while((*stringToMeasure!=0) && 
		   (stringToMeasure!=lastCharacter)) 
	{
		if(*stringToMeasure == '\n') {
			lastStringWidth = MATH_MAX(lastStringWidth, w);
			w = 0;
			lines++;
		} else if(*stringToMeasure == ' ') {
			w += frames[fontFrame]->fModules[0]->module->width + charSpacing;
		} else {
			w += frames[fontFrame]->fModules[GetNumberModuleByChar(*stringToMeasure)]->module->width + charSpacing + GetKerningSpacing(*stringToMeasure);
		}

		stringToMeasure++;
	}

	lastStringWidth = MATH_MAX(lastStringWidth, w) - charSpacing; //LAngelov: Heavy patch for centering
	lastStringHeight = lines * GetFontHeight() + (lines - 1) * lineSpacing;
}

const unsigned short CSprite::m_NormalFontMap[] = {
	  0, // 0   00
	  0, // 1   01
	  0, // 2   02
	  0, // 3   03
	  0, // 4   04
	  0, // 5   05
	  0, // 6   06
	  0, // 7   07
	  0, // 8   08
	  0, // 9   09
	  0, // 10  0A
	  0, // 11  0B
	  0, // 12  0C
	  0, // 13  0D
	  0, // 14  0E
	  0, // 15  0F
	  0, // 16  10
	  0, // 17  11
	  0, // 18  12
	  0, // 19  13
	  0, // 20  14
	  0, // 21  15
	  0, // 22  16
	  0, // 23  17
	  0, // 24  18
	  0, // 25  19
	  0, // 26  1A
	  0, // 27  1B
	  0, // 28  1C
	  0, // 29  1D
	  0, // 30  1E
	  0, // 31  1F

	  0, // 32  20 ' '
	  1, // 33  21 '!'
	  2, // 34  22 '"'
	  3, // 35  23 '#'
	  4, // 36  24 '$'
	  5, // 37  25 '%'
	  6, // 38  26 '&'
	  7, // 39  27 '''
	  8, // 40  28 '('
	  9, // 41  29 ')'
	 10, // 42  2A '*'
	 11, // 43  2B '+'
	 12, // 44  2C ','
	 13, // 45  2D '-'
	 14, // 46  2E '.'
	 15, // 47  2F '/'
	 16, // 48  30 '0'
	 17, // 49  31 '1'
	 18, // 50  32 '2'
	 19, // 51  33 '3'
	 20, // 52  34 '4'
	 21, // 53  35 '5'
	 22, // 54  36 '6'
	 23, // 55  37 '7'
	 24, // 56  38 '8'
	 25, // 57  39 '9'
	 26, // 58  3A ':'
	 27, // 59  3B ';'
	 28, // 60  3C '<'
	 29, // 61  3D '='
	 30, // 62  3E '>'
	 31, // 63  3F '?'

	 32, // 64  40 '@'
	 33, // 65  41 'A'
	 34, // 66  42 'B'
	 35, // 67  43 'C'
	 36, // 68  44 'D'
	 37, // 69  45 'E'
	 38, // 70  46 'F'
	 39, // 71  47 'G'
	 40, // 72  48 'H'
	 41, // 73  49 'I'
	 42, // 74  4A 'J'
	 43, // 75  4B 'K'
	 44, // 76  4C 'L'
	 45, // 77  4D 'M'
	 46, // 78  4E 'N'
	 47, // 79  4F 'O'
	 48, // 80  50 'P'
	 49, // 81  51 'Q'
	 50, // 82  52 'R'
	 51, // 83  53 'S'
	 52, // 84  54 'T'
	 53, // 85  55 'U'
	 54, // 86  56 'V'
	 55, // 87  57 'W'
	 56, // 88  58 'X'
	 57, // 89  59 'Y'
	 58, // 90  5A 'Z'
	 59, // 91  5B '['
	 60, // 92  5C '\'
	 61, // 93  5D ']'
	 62, // 94  5E '^'
	 63, // 95  5F '_'

	 64, // 96  60 '`'
	 65, // 97  61 'a'
	 66, // 98  62 'b'
	 67, // 99  63 'c'
	 68, // 100 64 'd'
	 69, // 101 65 'e'
	 70, // 102 66 'f'
	 71, // 103 67 'g'
	 72, // 104 68 'h'
	 73, // 105 69 'i'
	 74, // 106 6A 'j'
	 75, // 107 6B 'k'
	 76, // 108 6C 'l'
	 77, // 109 6D 'm'
	 78, // 110 6E 'n'
	 79, // 111 6F 'o'
	 80, // 112 70 'p'
	 81, // 113 71 'q'
	 82, // 114 72 'r'
	 83, // 115 73 's'
	 84, // 116 74 't'
	 85, // 117 75 'u'
	 86, // 118 76 'v'
	 87, // 119 77 'w'
	 88, // 120 78 'x'
	 89, // 121 79 'y'
	 90, // 122 7A 'z'
	 91, // 123 7B '{'
	 92, // 124 7C '|'
	 93, // 125 7D '}'
	 94, // 126 7E '~'
	 95, // 127 7F ''

	  0, // 128 80 '€'
	  0, // 129 81 '?'
	  0, // 130 82 '‚'
	  0, // 131 83 'ƒ'
	  0, // 132 84 '„'
	  0, // 133 85 '…'
	  0, // 134 86 '†'
	  0, // 135 87 '‡'
	  0, // 136 88 'ˆ'
	  0, // 137 89 '‰'
	  0, // 138 8A 'Š'
	  0, // 139 8B '‹'
	129, // 140 8C 'Œ'
	  0, // 141 8D '?'
	  0, // 142 8E 'Ž'
	  0, // 143 8F '?'
	  0, // 144 90 '?'
	  0, // 145 91 '‘'
	130, // 146 92 '’'
	  0, // 147 93 '“'
	  0, // 148 94 '”'
	  0, // 149 95 '•'
	  0, // 150 96 '–'
	  0, // 151 97 '—'
	  0, // 152 98 '˜'
	135, // 153 99 '™' //TODO, move it on separate module // 174 
	  0, // 154 9A 'š'
	  0, // 155 9B '›'
	131, // 156 9C 'œ'
	  0, // 157 9D '?'
	  0, // 158 9E 'ž'
	  0, // 159 9F 'Ÿ'

	  0, // 160 A0 ' ' No-Break Space
	132, // 161 A1 '¡'
	  0, // 162 A2 '¢'
	  0, // 163 A3 '£'
	  0, // 164 A4 '¤'
	  0, // 165 A5 '¥'
	  0, // 166 A6 '¦'
	  0, // 167 A7 '§'
	  0, // 168 A8 '¨'
	133, // 169 A9 '©'
	  0, // 170 AA 'ª'
	  0, // 171 AB '«'
	  0, // 172 AC '¬'
	  0, // 173 AD '­'
	  0, // 174 AE '®'
	  0, // 175 AF '¯'
	  0, // 176 B0 '°' // TODO // 174
	  0, // 177 B1 '±'
	  0, // 178 B2 '²'
	136, // 179 B3 '³'
	  0, // 180 B4 '´'
	  0, // 181 B5 'µ'
	  0, // 182 B6 '¶'
	  0, // 183 B7 '·'
	  0, // 184 B8 '¸'
	  0, // 185 B9 '¹'
	137, // 186 BA 'º'// TODO // 177
	  0, // 187 BB '»'
	  0, // 188 BC '¼'
	  0, // 189 BD '½'
	  0, // 190 BE '¾'
	134, // 191 BF '¿'

	 96, // 192 C0 'À'
	 97, // 193 C1 'Á'
	 98, // 194 C2 'Â'
	 99, // 195 C3 'Ã'
	101, // 196 C4 'Ä'
	102, // 197 C5 'Å'
	103, // 198 C6 'Æ'
	104, // 199 C7 'Ç'
	105, // 200 C8 'È'
	106, // 201 C9 'É'
	107, // 202 CA 'Ê'
	108, // 203 CB 'Ë'
	109, // 204 CC 'Ì'
	110, // 205 CD 'Í'
	111, // 206 CE 'Î'
	112, // 207 CF 'Ï'
	113, // 208 D0 'Ð'
	114, // 209 D1 'Ñ'
	115, // 210 D2 'Ò'
	116, // 211 D3 'Ó'
	117, // 212 D4 'Ô'
	118, // 213 D5 'Õ'
	119, // 214 D6 'Ö'
	120, // 215 D7 '×'
	121, // 216 D8 'Ø'
	122, // 217 D9 'Ù'
	123, // 218 DA 'Ú'
	124, // 219 DB 'Û'
	125, // 220 DC 'Ü'
	126, // 221 DD 'Ý'
	127, // 222 DE 'Þ'
	128, // 223 DF 'ß'

	144, // 224 E0 'à'
	145, // 225 E1 'á'
	146, // 226 E2 'â'
	147, // 227 E3 'ã'
	148, // 228 E4 'ä'
	149, // 229 E5 'å'
	150, // 230 E6 'æ'
	151, // 231 E7 'ç'
	152, // 232 E8 'è'
	153, // 233 E9 'é'
	154, // 234 EA 'ê'
	155, // 235 EB 'ë'
	156, // 236 EC 'ì'
	157, // 237 ED 'í'
	158, // 238 EE 'î'
	159, // 239 EF 'ï'
	160, // 240 F0 'ð'
	161, // 241 F1 'ñ'
	162, // 242 F2 'ò'
	163, // 243 F3 'ó'
	164, // 244 F4 'ô'
	165, // 245 F5 'õ'
	166, // 246 F6 'ö'
	167, // 247 F7 '÷'
	168, // 248 F8 'ø'
	169, // 249 F9 'ù'
	170, // 250 FA 'ú'
	171, // 251 FB 'û'
	172, // 252 FC 'ü'
	173, // 253 FD 'ý'
	174, // 254 FE 'þ'
	175, // 255 FF 'ÿ'
}; // m_NormalFontMap

const unsigned short CSprite::m_NumberFontMap[] = {	  
	  0, // 0   00
	  0, // 1   01
	  0, // 2   02
	  0, // 3   03
	  0, // 4   04
	  0, // 5   05
	  0, // 6   06
	  0, // 7   07
	  0, // 8   08
	  0, // 9   09
	  0, // 10  0A
	  0, // 11  0B
	  0, // 12  0C
	  0, // 13  0D
	  0, // 14  0E
	  0, // 15  0F
	  0, // 16  10
	  0, // 17  11
	  0, // 18  12
	  0, // 19  13
	  0, // 20  14
	  0, // 21  15
	  0, // 22  16
	  0, // 23  17
	  0, // 24  18
	  0, // 25  19
	  0, // 26  1A
	  0, // 27  1B
	  0, // 28  1C
	  0, // 29  1D
	  0, // 30  1E
	  0, // 31  1F

	  0, // 32  20 ' '
	  0, // 33  21 '!'
	  0, // 34  22 '"'
	  0, // 35  23 '#'
	 13, // 36  24 '$'
	  0, // 37  25 '%'
	  0, // 38  26 '&'
	  0, // 39  27 '''
	  0, // 40  28 '('
	  0, // 41  29 ')'
	  0, // 42  2A '*'
	  0, // 43  2B '+'
	 12, // 44  2C ','
	  0, // 45  2D '-'
	 11, // 46  2E '.'
	 14, // 47  2F '/'
	  1, // 48  30 '0'
	  2, // 49  31 '1'
	  3, // 50  32 '2'
	  4, // 51  33 '3'
	  5, // 52  34 '4'
	  6, // 53  35 '5'
	  7, // 54  36 '6'
	  8, // 55  37 '7'
	  9, // 56  38 '8'
	 10, // 57  39 '9'
	  0, // 58  3A ':'
	  0, // 59  3B ';'
	  0, // 60  3C '<'
	  0, // 61  3D '='
	  0, // 62  3E '>'
	  0, // 63  3F '?'

	  0, // 64  40 '@'
	  0, // 65  41 'A'
	  0, // 66  42 'B'
	  0, // 67  43 'C'
	  0, // 68  44 'D'
	  0, // 69  45 'E'
	  0, // 70  46 'F'
	  0, // 71  47 'G'
	  0, // 72  48 'H'
	  0, // 73  49 'I'
	  0, // 74  4A 'J'
	  0, // 75  4B 'K'
	  0, // 76  4C 'L'
	  0, // 77  4D 'M'
	  0, // 78  4E 'N'
	  0, // 79  4F 'O'
	  0, // 80  50 'P'
	  0, // 81  51 'Q'
	  0, // 82  52 'R'
	  0, // 83  53 'S'
	  0, // 84  54 'T'
	  0, // 85  55 'U'
	  0, // 86  56 'V'
	  0, // 87  57 'W'
	  0, // 88  58 'X'
	  0, // 89  59 'Y'
	  0, // 90  5A 'Z'
	  0, // 91  5B '['
	  0, // 92  5C '\'
	  0, // 93  5D ']'
	  0, // 94  5E '^'
	  0, // 95  5F '_'

	  0, // 96  60 '`'
	  0, // 97  61 'a'
	  0, // 98  62 'b'
	  0, // 99  63 'c'
	  0, // 100 64 'd'
	  0, // 101 65 'e'
	  0, // 102 66 'f'
	  0, // 103 67 'g'
	  0, // 104 68 'h'
	  0, // 105 69 'i'
	  0, // 106 6A 'j'
	  0, // 107 6B 'k'
	  0, // 108 6C 'l'
	  0, // 109 6D 'm'
	  0, // 110 6E 'n'
	  0, // 111 6F 'o'
	  0, // 112 70 'p'
	  0, // 113 71 'q'
	  0, // 114 72 'r'
	  0, // 115 73 's'
	  0, // 116 74 't'
	  0, // 117 75 'u'
	  0, // 118 76 'v'
	  0, // 119 77 'w'
	  0, // 120 78 'x'
	  0, // 121 79 'y'
	  0, // 122 7A 'z'
	  0, // 123 7B '{'
	  0, // 124 7C '|'
	  0, // 125 7D '}'
	  0, // 126 7E '~'
	  0, // 127 7F ''

	  0, // 128 80 '€'
	  0, // 129 81 '?'
	  0, // 130 82 '‚'
	  0, // 131 83 'ƒ'
	  0, // 132 84 '„'
	  0, // 133 85 '…'
	  0, // 134 86 '†'
	  0, // 135 87 '‡'
	  0, // 136 88 'ˆ'
	  0, // 137 89 '‰'
	  0, // 138 8A 'Š'
	  0, // 139 8B '‹'
	  0, // 140 8C 'Œ'
	  0, // 141 8D '?'
	  0, // 142 8E 'Ž'
	  0, // 143 8F '?'
	  0, // 144 90 '?'
	  0, // 145 91 '‘'
	  0, // 146 92 '’'
	  0, // 147 93 '“'
	  0, // 148 94 '”'
	  0, // 149 95 '•'
	  0, // 150 96 '–'
	  0, // 151 97 '—'
	  0, // 152 98 '˜'
	135, // 153 99 '™' //TODO, move it on separate module
	  0, // 154 9A 'š'
	  0, // 155 9B '›'
	  0, // 156 9C 'œ'
	  0, // 157 9D '?'
	  0, // 158 9E 'ž'
	  0, // 159 9F 'Ÿ'

	  0, // 160 A0 ' ' No-Break Space
	  0, // 161 A1 '¡'
	  0, // 162 A2 '¢'
	  0, // 163 A3 '£'
	  0, // 164 A4 '¤'
	  0, // 165 A5 '¥'
	  0, // 166 A6 '¦'
	  0, // 167 A7 '§'
	  0, // 168 A8 '¨'
	  0, // 169 A9 '©'
	  0, // 170 AA 'ª'
	  0, // 171 AB '«'
	  0, // 172 AC '¬'
	  0, // 173 AD '­'
	  0, // 174 AE '®'
	  0, // 175 AF '¯'
	  0, // 176 B0 '°'
	  0, // 177 B1 '±'
	  0, // 178 B2 '²'
	136, // 179 B3 '³'
	  0, // 180 B4 '´'
	  0, // 181 B5 'µ'
	  0, // 182 B6 '¶'
	  0, // 183 B7 '·'
	  0, // 184 B8 '¸'
	  0, // 185 B9 '¹'
	  0, // 186 BA 'º'
	  0, // 187 BB '»'
	  0, // 188 BC '¼'
	  0, // 189 BD '½'
	  0, // 190 BE '¾'
	  0, // 191 BF '¿'

	  0, // 192 C0 'À'
	  0, // 193 C1 'Á'
	  0, // 194 C2 'Â'
	  0, // 195 C3 'Ã'
	  0, // 196 C4 'Ä'
	  0, // 197 C5 'Å'
	  0, // 198 C6 'Æ'
	  0, // 199 C7 'Ç'
	  0, // 200 C8 'È'
	  0, // 201 C9 'É'
	  0, // 202 CA 'Ê'
	  0, // 203 CB 'Ë'
	  0, // 204 CC 'Ì'
	  0, // 205 CD 'Í'
	  0, // 206 CE 'Î'
	  0, // 207 CF 'Ï'
	  0, // 208 D0 'Ð'
	  0, // 209 D1 'Ñ'
	  0, // 210 D2 'Ò'
	  0, // 211 D3 'Ó'
	  0, // 212 D4 'Ô'
	  0, // 213 D5 'Õ'
	  0, // 214 D6 'Ö'
	  0, // 215 D7 '×'
	  0, // 216 D8 'Ø'
	  0, // 217 D9 'Ù'
	  0, // 218 DA 'Ú'
	  0, // 219 DB 'Û'
	  0, // 220 DC 'Ü'
	  0, // 221 DD 'Ý'
	  0, // 222 DE 'Þ'
	  0, // 223 DF 'ß'

	  0, // 224 E0 'à'
	  0, // 225 E1 'á'
	  0, // 226 E2 'â'
	  0, // 227 E3 'ã'
	  0, // 228 E4 'ä'
	  0, // 229 E5 'å'
	  0, // 230 E6 'æ'
	  0, // 231 E7 'ç'
	  0, // 232 E8 'è'
	  0, // 233 E9 'é'
	  0, // 234 EA 'ê'
	  0, // 235 EB 'ë'
	  0, // 236 EC 'ì'
	  0, // 237 ED 'í'
	  0, // 238 EE 'î'
	  0, // 239 EF 'ï'
	  0, // 240 F0 'ð'
	  0, // 241 F1 'ñ'
	  0, // 242 F2 'ò'
	  0, // 243 F3 'ó'
	  0, // 244 F4 'ô'
	  0, // 245 F5 'õ'
	  0, // 246 F6 'ö'
	  0, // 247 F7 '÷'
	  0, // 248 F8 'ø'
	  0, // 249 F9 'ù'
	  0, // 250 FA 'ú'
	  0, // 251 FB 'û'
	  0, // 252 FC 'ü'
	  0, // 253 FD 'ý'
	  0, // 254 FE 'þ'
	  0, // 255 FF 'ÿ'
}; // m_NumberFontMap

const unsigned short CSprite::m_AssianFontMap[] = {
	0x0020, //  
	0x0021, // !
	0x0022, // "
	0x0023, // #
	0x0024, // $
	0x0025, // %
	0x0026, // &
	0x0027, // '
	0x0028, // (
	0x0029, // )
	0x002A, // *
	0x002B, // +
	0x002C, // ,
	0x002D, // -
	0x002E, // .
	0x002F, // /
	0x0030, // 0
	0x0031, // 1
	0x0032, // 2
	0x0033, // 3
	0x0034, // 4
	0x0035, // 5
	0x0036, // 6
	0x0037, // 7
	0x0038, // 8
	0x0039, // 9
	0x003A, // :
	0x003B, // ;
	0x003C, // <
	0x003E, // >
	0x003F, // ?
	0x0040, // @
	0x0041, // A
	0x0042, // B
	0x0043, // C
	0x0044, // D
	0x0045, // E
	0x0046, // F
	0x0047, // G
	0x0048, // H
	0x0049, // I
	0x004A, // J
	0x004B, // K
	0x004C, // L
	0x004D, // M
	0x004E, // N
	0x004F, // O
	0x0050, // P
	0x0051, // Q
	0x0052, // R
	0x0053, // S
	0x0054, // T
	0x0055, // U
	0x0056, // V
	0x0057, // W
	0x0058, // X
	0x0059, // Y
	0x005A, // Z
	0x005B, // [
	0x005C, // backslash
	0x005D, // ]
	0x005F, // _
	0x0061, // a
	0x0062, // b
	0x0063, // c
	0x0064, // d
	0x0065, // e
	0x0066, // f
	0x0067, // g
	0x0068, // h
	0x0069, // i
	0x006A, // j
	0x006B, // k
	0x006C, // l
	0x006D, // m
	0x006E, // n
	0x006F, // o
	0x0070, // p
	0x0071, // q
	0x0072, // r
	0x0073, // s
	0x0074, // t
	0x0075, // u
	0x0076, // v
	0x0077, // w
	0x0078, // x
	0x0079, // y
	0x007A, // z
	0x007C, // |
	0x00A1, // ¡
	0x00A9, // ©
	0x00BF, // ¿
	0x00C0, // À
	0x00C1, // Á
	0x00C2, // Â
	0x00C4, // Ä
	0x00C6, // Æ
	0x00C7, // Ç
	0x00C8, // È
	0x00C9, // É
	0x00CA, // Ê
	0x00CB, // Ë
	0x00CC, // Ì
	0x00CD, // Í
	0x00CE, // Î
	0x00CF, // Ï
	0x00D1, // Ñ
	0x00D2, // Ò
	0x00D3, // Ó
	0x00D4, // Ô
	0x00D6, // Ö
	0x00D9, // Ù
	0x00DA, // Ú
	0x00DB, // Û
	0x00DC, // Ü
	0x00DF, // ß
	0x00E0, // à
	0x00E1, // á
	0x00E2, // â
	0x00E4, // ä
	0x00E6, // æ
	0x00E7, // ç
	0x00E8, // è
	0x00E9, // é
	0x00EA, // ê
	0x00EB, // ë
	0x00EC, // ì
	0x00ED, // í
	0x00EE, // î
	0x00EF, // ï
	0x00F1, // ñ
	0x00F2, // ò
	0x00F3, // ó
	0x00F4, // ô
	0x00F6, // ö
	0x00F9, // ù
	0x00FA, // ú
	0x00FB, // û
	0x00FC, // ü
	0x2019, // ’
	0x201C, // “
	0x201D, // ”
	0x2122, // ™
	0x3001, // 、
	0x3002, // 。
	0x300c, // 「
	0x300d, // 」
	0x3041, // ぁ
	0x3042, // あ
	0x3044, // い
	0x3046, // う
	0x3048, // え
	0x304A, // お
	0x304B, // か
	0x304C, // が
	0x304D, // き
	0x304E, // ぎ
	0x304F, // く
	0x3050, // ぐ
	0x3051, // け
	0x3053, // こ
	0x3054, // ご
	0x3055, // さ
	0x3056, // ざ
	0x3057, // し
	0x3058, // じ
	0x3059, // す
	0x305A, // ず
	0x305B, // せ
	0x305C, // ぜ
	0x305D, // そ
	0x305E, // ぞ
	0x305F, // た
	0x3060, // だ
	0x3061, // ち
	0x3063, // っ
	0x3064, // つ
	0x3065, // づ
	0x3066, // て
	0x3067, // で
	0x3068, // と
	0x3069, // ど
	0x306A, // な
	0x306B, // に
	0x306D, // ね
	0x306E, // の
	0x306F, // は
	0x3070, // ば
	0x3071, // ぱ
	0x3073, // び
	0x3076, // ぶ
	0x3078, // へ
	0x307B, // ほ
	0x307E, // ま
	0x307F, // み
	0x3080, // む
	0x3081, // め
	0x3082, // も
	0x3083, // ゃ
	0x3084, // や
	0x3086, // ゆ
	0x3087, // ょ
	0x3088, // よ
	0x3089, // ら
	0x308A, // り
	0x308B, // る
	0x308C, // れ
	0x308D, // ろ
	0x308F, // わ
	0x3092, // を
	0x3093, // ん
	0x30A1, // ァ
	0x30A2, // ア
	0x30A3, // ィ
	0x30A4, // イ
	0x30A6, // ウ
	0x30A7, // ェ
	0x30A8, // エ
	0x30A9, // ォ
	0x30AA, // オ
	0x30AB, // カ
	0x30AC, // ガ
	0x30AD, // キ
	0x30AE, // ギ
	0x30AF, // ク
	0x30B0, // グ
	0x30B1, // ケ
	0x30B2, // ゲ
	0x30B3, // コ
	0x30B4, // ゴ
	0x30B5, // サ
	0x30B6, // ザ
	0x30B7, // シ
	0x30B8, // ジ
	0x30B9, // ス
	0x30BA, // ズ
	0x30BB, // セ
	0x30BC, // ゼ
	0x30BD, // ソ
	0x30BF, // タ
	0x30C0, // ダ
	0x30C1, // チ
	0x30C3, // ッ
	0x30C4, // ツ
	0x30C6, // テ
	0x30C7, // デ
	0x30C8, // ト
	0x30C9, // ド
	0x30CA, // ナ
	0x30CB, // ニ
	0x30CD, // ネ
	0x30CE, // ノ
	0x30CF, // ハ
	0x30D0, // バ
	0x30D1, // パ
	0x30D2, // ヒ
	0x30D3, // ビ
	0x30D4, // ピ
	0x30D5, // フ
	0x30D6, // ブ
	0x30D7, // プ
	0x30D8, // ヘ
	0x30D9, // ベ
	0x30DA, // ペ
	0x30DB, // ホ
	0x30DC, // ボ
	0x30DD, // ポ
	0x30DE, // マ
	0x30DF, // ミ
	0x30E0, // ム
	0x30E1, // メ
	0x30E2, // モ
	0x30E3, // ャ
	0x30E4, // ヤ
	0x30E5, // ュ
	0x30E6, // ユ
	0x30E7, // ョ
	0x30E8, // ヨ
	0x30E9, // ラ
	0x30EA, // リ
	0x30EB, // ル
	0x30EC, // レ
	0x30ED, // ロ
	0x30EF, // ワ
	0x30F3, // ン
	0x30FB, // ・
	0x30FC, // ー
	0x4E00, // 一
	0x4E07, // 万
	0x4E0a, // 上
	0x4E0b, // 下
	0x4E16, // 世
	0x4E2D, // 中
	0x4E3B, // 主
	0x4E57, // 乗
	0x4E86, // 了
	0x4E8B, // 事
	0x4ED5, // 仕
	0x4ED6, // 他
	0x4EE3, // 代
	0x4EE5, // 以
	0x4EF6, // 件
	0x4F4D, // 位
	0x4F53, // 体
	0x4F55, // 何
	0x4F59, // 余
	0x4F5C, // 作
	0x4F7F, // 使
	0x4F9B, // 供
	0x4FDD, // 保
	0x4FE1, // 信
	0x5099, // 備
	0x50AC, // 催
	0x50BE, // 傾
	0x5104, // 億
	0x512A, // 優
	0x5148, // 先
	0x5165, // 入
	0x5168, // 全
	0x5171, // 共
	0x5185, // 内
	0x518D, // 再
	0x51FA, // 出
	0x5206, // 分
	0x5207, // 切
	0x521D, // 初
	0x5229, // 利
	0x5230, // 到
	0x524D, // 前
	0x529B, // 力
	0x52A0, // 加
	0x52B9, // 効
	0x52D5, // 動
	0x52DD, // 勝
	0x5317, // 北
	0x5374, // 却
	0x53BB, // 去
	0x53C2, // 参
	0x53CD, // 反
	0x53D6, // 取
	0x53D7, // 受
	0x53EF, // 可
	0x53F0, // 台
	0x53F3, // 右
	0x540c, // 同
	0x540d, // 名
	0x5411, // 向
	0x5426, // 否
	0x5473, // 味
	0x54C1, // 品
	0x5546, // 商
	0x56DE, // 回
	0x56FD, // 国
	0x5728, // 在
	0x574A, // 坊
	0x5831, // 報
	0x5834, // 場
	0x5897, // 増
	0x58C1, // 壁
	0x58CA, // 壊
	0x58F0, // 声
	0x58F2, // 売
	0x5909, // 変
	0x5916, // 外
	0x591A, // 多
	0x5931, // 失
	0x59CB, // 始
	0x5B50, // 子
	0x5B57, // 字
	0x5B8C, // 完
	0x5B9A, // 定
	0x5B9F, // 実
	0x5BB9, // 容
	0x5BDF, // 察
	0x5BFA, // 寺
	0x5BFE, // 対
	0x5C11, // 少
	0x5DDD, // 川
	0x5DE6, // 左
	0x5DF1, // 己
	0x5DFB, // 巻
	0x5E02, // 市
	0x5EA6, // 度
	0x5F15, // 引
	0x5F53, // 当
	0x5F71, // 影
	0x5F85, // 待
	0x5F97, // 得
	0x5FC5, // 必
	0x5FD7, // 志
	0x5FDC, // 応
	0x6012, // 怒
	0x6016, // 怖
	0x6027, // 性
	0x60C5, // 情
	0x614B, // 態
	0x6226, // 戦
	0x623B, // 戻
	0x6240, // 所
	0x624B, // 手
	0x627F, // 承
	0x629C, // 抜
	0x629E, // 択
	0x62BC, // 押
	0x62D2, // 拒
	0x6301, // 持
	0x6307, // 指
	0x632F, // 振
	0x6355, // 捕
	0x6392, // 排
	0x63A2, // 探
	0x63A5, // 接
	0x63DB, // 換
	0x63EE, // 揮
	0x640d, // 損
	0x64CD, // 操
	0x653E, // 放
	0x6557, // 敗
	0x656C, // 敬
	0x6570, // 数
	0x6575, // 敵
	0x6587, // 文
	0x65AD, // 断
	0x65B0, // 新
	0x65B9, // 方
	0x65E5, // 日
	0x65E7, // 旧
	0x6613, // 易
	0x6642, // 時
	0x66B4, // 暴
	0x66F2, // 曲
	0x66F4, // 更
	0x66FF, // 替
	0x6700, // 最
	0x6709, // 有
	0x671B, // 望
	0x671F, // 期
	0x672C, // 本
	0x6761, // 条
	0x6765, // 来
	0x6790, // 析
	0x679C, // 果
	0x683C, // 格
	0x691C, // 検
	0x697D, // 楽
	0x6A19, // 標
	0x6A21, // 模
	0x6A5F, // 機
	0x6B21, // 次
	0x6B62, // 止
	0x6C17, // 気
	0x6C7A, // 決
	0x6CBF, // 沿
	0x6CC1, // 況
	0x6CD5, // 法
	0x6D77, // 海
	0x6D88, // 消
	0x6D9B, // 涛
	0x6E08, // 済
	0x6E1B, // 減
	0x6E26, // 渦
	0x6E96, // 準
	0x6F14, // 演
	0x7058, // 灘
	0x70BA, // 為
	0x7248, // 版
	0x72AF, // 犯
	0x72B6, // 状
	0x7372, // 獲
	0x738B, // 王
	0x73FE, // 現
	0x751F, // 生
	0x7528, // 用
	0x753B, // 画
	0x754C, // 界
	0x767A, // 発
	0x767B, // 登
	0x76EE, // 目
	0x76F8, // 相
	0x7834, // 破
	0x78BA, // 確
	0x793A, // 示
	0x793C, // 礼
	0x79C1, // 私
	0x79D2, // 秒
	0x7A2E, // 種
	0x7A3C, // 稼
	0x7A4D, // 積
	0x7B52, // 筒
	0x7D22, // 索
	0x7D42, // 終
	0x7D99, // 継
	0x7D9A, // 続
	0x7DD2, // 緒
	0x7F6A, // 罪
	0x7F6E, // 置
	0x7F70, // 罰
	0x8005, // 者
	0x8074, // 聴
	0x80FD, // 能
	0x8155, // 腕
	0x81EA, // 自
	0x826F, // 良
	0x878D, // 融
	0x884C, // 行
	0x8857, // 街
	0x8868, // 表
	0x88D5, // 裕
	0x88FD, // 製
	0x8981, // 要
	0x8987, // 覇
	0x898B, // 見
	0x8A00, // 言
	0x8A18, // 記
	0x8A2D, // 設
	0x8A3C, // 証
	0x8A66, // 試
	0x8A8D, // 認
	0x8A9E, // 語
	0x8AA0, // 誠
	0x8AB0, // 誰
	0x8AFE, // 諾
	0x8B66, // 警
	0x8CA0, // 負
	0x8CDE, // 賞
	0x8CE2, // 賢
	0x8CEA, // 質
	0x8CED, // 賭
	0x8CFC, // 購
	0x8D70, // 走
	0x8D8A, // 越
	0x8DB3, // 足
	0x8DDD, // 距
	0x8ECA, // 車
	0x8EE2, // 転
	0x8FBC, // 込
	0x8FFD, // 追
	0x9001, // 送
	0x9006, // 逆
	0x901F, // 速
	0x9023, // 連
	0x902E, // 逮
	0x9032, // 進
	0x904B, // 運
	0x9054, // 達
	0x9055, // 違
	0x9078, // 選
	0x907F, // 避
	0x914D, // 配
	0x91CD, // 重
	0x91CF, // 量
	0x91D1, // 金
	0x9332, // 録
	0x9577, // 長
	0x958B, // 開
	0x9593, // 間
	0x9632, // 防
	0x9662, // 院
	0x969C, // 障
	0x96C6, // 集
	0x96E2, // 離
	0x96E3, // 難
	0x9762, // 面
	0x97F3, // 音
	0x97FF, // 響
	0x9806, // 順
	0x982D, // 頭
	0x984D, // 額
	0x99C6, // 駆
	0x9AD8, // 高
	0xFF01, // ！
	0xFF06, // ＆
	0xFF0d, // －
	0xFF11, // １
	0xFF12, // ２
	0xFF13, // ３
	0xFF14, // ４
	0xFF15, // ５
	0xFF1A, // ：
	0xFF1F, // ？
	0xFF65, // ･
	0xFF6C, // ｬ
	0xFF6F, // ｯ
	0xFF70, // ｰ
	0xFF71, // ｱ
	0xFF72, // ｲ
	0xFF73, // ｳ
	0xFF74, // ｴ
	0xFF76, // ｶ
	0xFF78, // ｸ
	0xFF7C, // ｼ
	0xFF7D, // ｽ
	0xFF7E, // ｾ
	0xFF83, // ﾃ
	0xFF84, // ﾄ
	0xFF8B, // ﾋ
	0xFF8E, // ﾎ
	0xFF90, // ﾐ
	0xFF91, // ﾑ
	0xFF97, // ﾗ
	0xFF98, // ﾘ
	0xFF9A, // ﾚ
	0xFF9D, // ﾝ
	0xFF9E, // ﾞ
	0xFF9F, // ﾟ
}; // m_AssianFontMap


const unsigned short CSprite::m_ArabicFontMap[] = {
	0x0020, //    
	0x0021, // !  
	0x0022, // "  
	0x0023, // #  
	0x0024, // $  
	0x0025, // %  
	0x0026, // &  
	0x0027, // '  
	0x0028, // (  
	0x0029, // )  
	0x002A, // *  
	0x002B, // +  
	0x002C, // ,  
	0x002D, // -  
	0x002E, // .  
	0x002F, // /  
	0x0030, // 0  
	0x0031, // 1  
	0x0032, // 2  
	0x0033, // 3  
	0x0034, // 4  
	0x0035, // 5  
	0x0036, // 6  
	0x0037, // 7  
	0x0038, // 8  
	0x0039, // 9  
	0x003A, // :  
	0x003B, // ;  
	0x003C, // <  
	0x003E, // >  
	0x003F, // ?  
	0x0040, // @  
	0x0041, // A  
	0x0042, // B  
	0x0043, // C  
	0x0044, // D  
	0x0045, // E  
	0x0046, // F  
	0x0047, // G  
	0x0048, // H  
	0x0049, // I  
	0x004A, // J  
	0x004B, // K  
	0x004C, // L  
	0x004D, // M  
	0x004E, // N  
	0x004F, // O  
	0x0050, // P  
	0x0051, // Q  
	0x0052, // R  
	0x0053, // S  
	0x0054, // T  
	0x0055, // U  
	0x0056, // V  
	0x0057, // W  
	0x0058, // X  
	0x0059, // Y  
	0x005A, // Z  
	0x005C, // backslash 
	0x005F, // _  
	0x0061, // a  
	0x0062, // b  
	0x0063, // c  
	0x0064, // d  
	0x0065, // e  
	0x0066, // f  
	0x0067, // g  
	0x0068, // h  
	0x0069, // i  
	0x006A, // j  
	0x006B, // k  
	0x006C, // l  
	0x006D, // m  
	0x006E, // n  
	0x006F, // o  
	0x0070, // p  
	0x0071, // q  
	0x0072, // r  
	0x0073, // s  
	0x0074, // t  
	0x0075, // u  
	0x0076, // v  
	0x0077, // w  
	0x0078, // x  
	0x0079, // y  
	0x007A, // z  
	0x00A1, // ¡  
	0x00A9, // ©  
	0x00BF, // ¿  
	0x00C7, // Ç  
	0x00D1, // Ñ  
	0x060C, // ،  
	0x061F, // ؟  
	0x0621, // ء  
	0x0622, // آ  
	0x0623, // أ  
	0x0624, // ؤ  
	0x0625, // إ  
	0x0626, // ئ  
	0x0627, // ا  
	0x0628, // ب  
	0x0629, // ة  
	0x062A, // ت  
	0x062B, // ث  
	0x062C, // ج  
	0x062D, // ح  
	0x062E, // خ  
	0x062F, // د  
	0x0630, // ذ  
	0x0631, // ر  
	0x0632, // ز  
	0x0633, // س  
	0x0634, // ش  
	0x0635, // ص  
	0x0636, // ض  
	0x0637, // ط  
	0x0638, // ظ  
	0x0639, // ع  
	0x063A, // غ  
	0x0640, // ـ  
	0x0641, // ف  
	0x0642, // ق  
	0x0643, // ك  
	0x0644, // ل  
	0x0645, // م  
	0x0646, // ن  
	0x0647, // ه  
	0x0648, // و  
	0x0649, // ى  
	0x064A, // ي  
	0x064B, // ً  
	0x064C, // ٌ  
	0x064D, // ٍ  
	0x064E, // َ  
	0x064F, // ُ  
	0x0650, // ِ  
	0x0651, // ّ  
	0x0660, // ٠  
	0x0661, // ١  
	0x0662, // ٢  
	0x0663, // ٣  
	0x0664, // ٤  
	0x0665, // ٥  
	0x0666, // ٦  
	0x0667, // ٧  
	0x0669, // ٩  
	0x06F2, // ۲  
	0x06F3, // ۳  
	0x201C, // “  
	0x201D, // ”  
	0x2122, // ™  
};
