#ifndef __AURORASPRITE__
#define __AURORASPRITE__

class CLib2D;

#include "config.h"

#define GLOBAL_ALPHA_OPAQUE		0x0F

#define MAX_STRING_LENGTH		512

#ifdef USE_OGL
	#include "Lib3DGL/Lib3DGL.h"
#endif /* USE_OGL */

#define SPRITE_FLAG_TEXTURE_SHORT		(1 << 0)
#define SPRITE_FLAG_TEXTURE_GRAY		(1 << 1)	//create qray textures

class CModule {
public:
    short			width;
    short			height;
	bool			useAlpha;
	bool			useIndexedPixels;
	bool			useLowColor;
	
	unsigned char	image;

#ifdef USE_SINGLE_IMAGE

	short			x;
	short			y;

#else 

	unsigned short*	pixels;
	unsigned char*  indexedPixels;
	unsigned char*	alpha;

#endif // USE_SINGLE_IMAGE

			CModule(int _width, int _height, int _type, int _x = 0, int _y = 0);
	virtual ~CModule();
};

class CAuroraPalette {
public:
	short			numColors;
	unsigned char*	data;
	short			alphaIndex;

			 CAuroraPalette(short _numColors);
	virtual ~CAuroraPalette();
};

class CFModule {
public:
	short			xOffset;
	short			yOffset;
	short			flags;
	CModule*		module;
};

class CFrame
{
public:
	short			numFModules;
	CFModule**		fModules;

			 CFrame(short _numFModules);
			~CFrame();
};

class CAFrame
{
public:
	short xOffset;
	short yOffset;
	short time;
	short flags;

	CFrame*		frame;
};

class CAnim {
public:
	short			numAFrames;
	CAFrame**		aFrames;

			 CAnim(short _numAFrames);
			~CAnim();
};

#define ALIGN_CENTERED_TEXT (CSprite::ALIGN_HCENTER_TOP)
#define ALIGN_RIGHT_TEXT (CSprite::ALIGN_RIGHT_TOP)
#define ALIGN_LEFT_TEXT (CSprite::ALIGN_LEFT_TOP)

class CSprite
{
public:

	int	FormatStringWithLimitedWidth( const unsigned short* s, unsigned short* buf, int maxWidth ,  bool ignoreNewLines, bool __ignoreSpaces = false);

	enum Align
	{
		HALIGN_LEFT		= 0x00,
		HALIGN_CENTER	= 0x01,
		HALIGN_RIGHT	= 0x02,

		VALIGN_TOP		= 0x00,
		VALIGN_CENTER	= 0x10,
		VALIGN_BOTTOM	= 0x20,

		ALIGN_LEFT_TOP	= HALIGN_LEFT | VALIGN_TOP,
		ALIGN_RIGHT_TOP	= HALIGN_RIGHT | VALIGN_TOP,
		ALIGN_HCENTER_TOP	= HALIGN_CENTER | VALIGN_TOP,

		ALIGN_LEFT_VCENTER	= HALIGN_LEFT | VALIGN_CENTER,
		ALIGN_RIGHT_VCENTER	= HALIGN_RIGHT | VALIGN_CENTER,
		ALIGN_HCENTER_VCENTER	= HALIGN_CENTER | VALIGN_CENTER,

		ALIGN_LEFT_BOTTOM	= HALIGN_LEFT | VALIGN_BOTTOM,
		ALIGN_RIGHT_BOTTOM	= HALIGN_RIGHT | VALIGN_BOTTOM,
		ALIGN_HCENTER_BOTTOM	= HALIGN_CENTER | VALIGN_BOTTOM,
	};

	enum Flags {
		FLAGS_FLIP_X	= 0x01,
		FLAGS_FLIP_Y	= 0x02,
		FLAGS_ROT_90	= 0x04,

		//blending flags
		FLAGS_ADDITIVE_BLENDING= 0x08
	};

	enum RenderMode {
		RENDER_MODE_NORMAL		= 0,
		RENDER_MODE_GRAYSCALE	= 1,
		RENDER_MODE_SEPIA		= 2,
		RENDER_MODE_NEGATIVE	= 3,
		RENDER_MODE_SHADOW		= 4
	};
	
	static const unsigned short m_NormalFontMap[];
	static const unsigned short m_NumberFontMap[];
	static const int m_AssianFontMapSize = 605;
	static const unsigned short m_AssianFontMap[];
	static const int m_ArabicFontMapSize = 151;
	static const unsigned short m_ArabicFontMap[];

	static const unsigned short kRenderModeShadowColor = 0x39C7;

	CSprite()
	{
	};

	CSprite(const char* fileName, int createFlags = 0);
	virtual ~CSprite();

			void	DrawFrame (CLib2D& lib2d, int x, int y, int frame, int flags = 0, int rotation = 0) const;
			void    DrawFrame (CLib2D& lib2d, int x, int y, CFrame *f, int flags = 0, int rotation = 0) const;
			void	DrawModule(CLib2D& lib2d, int x, int y, int module, int flags = 0, int rotation = 0, int rotCenterX = 0, int rotCenterY = 0) const;
			void	DrawModule(CLib2D& lib2d, int x, int y, CModule* module, int flags = 0, int rotation = 0, int rotCenterX = 0, int rotCenterY = 0) const;

			void	DrawAFrame (CLib2D& lib2d, int x, int y, int anim, int aframe) const;			
			int		GetNumAFrames (int anim) const;
			int		GetAFrameTime(int anim, int aframe) const;

			int		GetStringWidthLimited(const unsigned short* s, int maxWidth ,  bool ignoreNewLines);
			int		GetStringHeightLimited(const unsigned short* s, int maxWidth ,  bool ignoreNewLines);
			int		DrawStringWithLimitedWidth(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth ,  bool ignoreNewLines, Align align = Align(HALIGN_LEFT|VALIGN_TOP), bool __ignoreSpaces = false);

			void    DrawStringWithLimitedWidthByAdjustingCharSpacing(CLib2D& lib2d, int x, int y, const unsigned short* s, int maxWidth ,  Align align = Align(HALIGN_LEFT|VALIGN_TOP));

			void	DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, Align align = Align(HALIGN_LEFT|VALIGN_TOP));
			void	DrawString(CLib2D& lib2d, int x, int y, const unsigned short* s, const unsigned short* lastCharacter,Align align = Align(HALIGN_LEFT|VALIGN_TOP));

			void	DrawNumber(CLib2D& lib2d, int x, int y, int number,    Align align = Align(HALIGN_LEFT|VALIGN_TOP));

			void	DrawNumberString(CLib2D& lib2d, int x, int y, const unsigned short* number,    Align align = Align(HALIGN_LEFT|VALIGN_TOP));
			void	DrawNumberString(CLib2D& lib2d, int x, int y, const unsigned short* s, const unsigned short* lastCharacter,Align align = Align(HALIGN_LEFT|VALIGN_TOP));

			short	GetStringWidth (const unsigned short* s);
			short	GetStringHeight(const unsigned short* s);

			short	GetNumberWidth (int number);
			short	GetNumberHeight(int number);

			short	GetNumberStringWidth (const unsigned short* number);
			short	GetNumberStringHeight(const unsigned short* number);
			void	ReplaceWidthChar(int charSpacing);
	inline	void	SetPalette  (short _palette)   { if (_palette < numPalettes ) currentPalette = _palette; }
	inline	short	GetPalette  ()				   { return currentPalette; }

	inline	short	GetFontHeight () const { return frames[fontFrame]->fModules[0]->module->height; }
	inline	short	GetFontWidth () const { return frames[fontFrame]->fModules[0]->module->width; }

	inline	short	GetFontFrame()	 const { return fontFrame; }
	inline	short	GetLineSpacing() const { return lineSpacing; }
	inline	short	GetCharSpacing() const { return charSpacing; }

	inline	void	SetFontFrame  (short _fontFrame)
	{
		fontFrame = _fontFrame;
		charSpacing = frames[fontFrame]->fModules[0]->xOffset;
	}
	inline	void	SetLineSpacing(short _lineSpacing) { lineSpacing = _lineSpacing; }
	inline	void	SetCharSpacing(short _charSpacing) { charSpacing = _charSpacing; }

	inline	short	GetModuleWidth (short module) const { return modules[module]->width; }
	inline	short	GetModuleHeight(short module) const { return modules[module]->height; }

	inline	short	GetModuleX (short module) const { return modules[module]->x; }
	inline	short	GetModuleY(short module) const { return modules[module]->y; }

	inline	short	GetFModuleX	(short frame, short fmodule) const { return frames[frame]->fModules[fmodule]->xOffset; }
	inline	short	GetFModuleY	(short frame, short fmodule) const { return frames[frame]->fModules[fmodule]->yOffset; }

	short GetFrameWidth(int frame) const;
	short GetFrameHeight(int frame) const;

	void GetAFrameRect(int anim, int aframe, int &x, int &y, int &w, int &h) const;

	inline	RenderMode	GetRenderMode()							{ return renderMode; }
	inline	void		SetRenderMode(RenderMode _RenderMode)	{ renderMode = _RenderMode; }

	inline	short		GetGlobalAlpha()						{ return globalAlpha; }
	inline	void		SetGlobalAlpha(short _globalAlpha)		{ globalAlpha = _globalAlpha; }

#ifndef USE_SINGLE_IMAGE
	inline const unsigned short* GetModuleData(short module) { return modules[module]->pixels; }
#else
	inline const unsigned short* GetModuleData(short module) { return m_pixels; }
#endif // USE_SINGLE_IMAGE

	inline void		SetMapping(const unsigned short* mapping) { m_FontMap = mapping; }

protected:
// 	//Auto-kerning
 	bool kerningEnabled;
	int   kerningSpacing;
 	int  GetKerningSpacing(unsigned short currentChar);
	void initKerning( bool bItalic );
	bool m_italic;
	short m_prevChar;

	const unsigned short* m_FontMap;

	int		GetModuleByChar(unsigned short c) const;
	inline int		GetNumberModuleByChar(unsigned short c) const { return m_NumberFontMap[c]; }
	void	MeasureString(const unsigned short* stringToMeasure, const unsigned short* lastCharacter=0);
	void	MeasureNumber(int number);
	void	MeasureNumberString(const unsigned short* stringToMeasure, const unsigned short* lastCharacter=0);

	char		numPalettes;
	char		numImages;
	short		numModules;
	short		numFrames;
	short		numAnims;

	CAuroraPalette**	palettes;
	CModule**	modules;
	CFrame**	frames;
	CAnim**		anims;

	short		currentPalette;
	short		fontFrame;
	short		lineSpacing;
	short		charSpacing;

	short		lastStringWidth;
	short		lastStringHeight;

	short		lastNumberWidth;
	short		lastNumberHeight;


	RenderMode			renderMode;
	short				globalAlpha;

#ifdef USE_SINGLE_IMAGE

public:
	unsigned short	width;
	unsigned short	height;
#ifdef USE_OGL
	int				m_pow2Width;
	int				m_pow2Height;

#ifdef DEBUG_TEX_MEM
	//u32				m_glTexPixelType;
	u32 m_videoMemSize;
#endif

#if defined(DEBUG_SPRITE_GLTEXTURE_ALLOC) && defined(_DEBUG)
	static const int k_MAX_TEXTURES_LOADED = 400;
	static u32 s_mapTexUniqueIdToGLTexName[k_MAX_TEXTURES_LOADED]; //array with glTexName

	static void CreateKeyForGLTexName(s16& key, u32 glTextureName) 
	{
	#ifdef _DEBUG
		A_ASSERT(glTextureName != 0);
		
		
		//verify if a key was created for this texturename ... this should not be happen
		for( int i = k_MAX_TEXTURES_LOADED-1; i >=0; i-- )
		{
			A_ASSERT(s_mapTexUniqueIdToGLTexName[i] != glTextureName);		
		}
	#endif

		for( int i = k_MAX_TEXTURES_LOADED-1; i >=0; i-- )
		{
			if( s_mapTexUniqueIdToGLTexName[i] == 0 )
			{
				key = i;
				s_mapTexUniqueIdToGLTexName[key] = glTextureName;
				return;
			}
		}
		//could not find a slot ... please increase k_MAX_TEXTURES_LOADED
		A_ASSERT(false);
	}

	//
	static void FreeKeyForGLTexName(const u32& glTextureName) 
	{
		int count = 0;
		for( int i = k_MAX_TEXTURES_LOADED-1; i >=0; i-- )
		{
			if( s_mapTexUniqueIdToGLTexName[i] == glTextureName )
			{
				count++;
				s_mapTexUniqueIdToGLTexName[i] = 0;
			}
		}

		A_ASSERT(count == 1);
	}
#endif //DEBUG_SPRITE_GLTEXTURE_ALLOC

	bool			m_bHasAlphaChannel;
	GLuint*			m_glTexturesName;
#endif // USE_OGL

	// temp
	unsigned short*	m_pixels;
	unsigned char*  m_indexedPixels;
	unsigned char*	m_alpha;

#endif // USE_SINGLE_IMAGE

};


#endif
