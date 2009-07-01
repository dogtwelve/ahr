#ifndef _TEXTURE_H_
#define _TEXTURE_H_


#include <stdio.h>
#include <limits.h>

#include "devutil.h"
#include "config.h"
#include "Constants.h"
#include "Color.h"
#include "File.h"
#include "File_iPhone.h"

#ifdef USE_OGL
#include "Lib3DGL/Lib3DGL.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum PixelType_TAG
{
	MGLPT_ARGB_4444 = 0x00,
	MGLPT_ARGB_1555,
	MGLPT_RGB_565,
	MGLPT_RGB_555,
	MGLPT_RGB_888,
	MGLPT_ARGB_8888,
	MGLPT_ARGB_8332,
	MGLPT_I_8,
	MGLPT_AI_88,
	MGLPT_1_BPP,
	MGLPT_VY1UY0,
	MGLPT_Y1VY0U,
	MGLPT_PVRTC2,
	MGLPT_PVRTC4,
	MGLPT_PVRTC2_2,
	MGLPT_PVRTC2_4,

	OGL_RGBA_4444= 0x10,
	OGL_RGBA_5551,
	OGL_RGBA_8888,
	OGL_RGB_565,
	OGL_RGB_555,
	OGL_RGB_888,
	OGL_I_8,
	OGL_AI_88,
	OGL_PVRTC2,
	OGL_PVRTC4,

	// OGL_BGRA_8888 extension
	OGL_BGRA_8888,

	D3D_DXT1 = 0x20,
	D3D_DXT2,
	D3D_DXT3,
	D3D_DXT4,
	D3D_DXT5,

	D3D_RGB_332,
	D3D_AI_44,
	D3D_LVU_655,
	D3D_XLVU_8888,
	D3D_QWVU_8888,

	//10 bits per channel
	D3D_ABGR_2101010,
	D3D_ARGB_2101010,
	D3D_AWVU_2101010,

	//16 bits per channel
	D3D_GR_1616,
	D3D_VU_1616,
	D3D_ABGR_16161616,

	//HDR formats
	D3D_R16F,
	D3D_GR_1616F,
	D3D_ABGR_16161616F,

	//32 bits per channel
	D3D_R32F,
	D3D_GR_3232F,
	D3D_ABGR_32323232F,

	// Ericsson
	ETC_RGB_4BPP,
	ETC_RGBA_EXPLICIT,
	ETC_RGBA_INTERPOLATED,

	MGLPT_NOTYPE = 0xff

} PixelType;

typedef struct PVR_Header_Texture_TAG
{
	unsigned int dwHeaderSize;			/*!< size of the structure */
	unsigned int dwHeight;				/*!< height of surface to be created */
	unsigned int dwWidth;				/*!< width of input surface */
	unsigned int dwMipMapCount;			/*!< number of mip-map levels requested */
	unsigned int dwpfFlags;				/*!< pixel format flags */
	unsigned int dwTextureDataSize;		/*!< Total size in bytes */
	unsigned int dwBitCount;			/*!< number of bits per pixel  */
	unsigned int dwRBitMask;			/*!< mask for red bit */
	unsigned int dwGBitMask;			/*!< mask for green bits */
	unsigned int dwBBitMask;			/*!< mask for blue bits */
	unsigned int dwAlphaBitMask;		/*!< mask for alpha channel */
	unsigned int dwPVR;					/*!< magic number identifying pvr file */
	unsigned int dwNumSurfs;			/*!< the number of surfaces present in the pvr */
} PVR_Texture_Header;


#ifndef IPHONE

extern unsigned int PVRTLoadTextureFromPointer(const void* pointer, GLuint *const texName, const void *psTextureHeader=NULL);

#endif // WIN32


#endif /* USE_OGL */

#define TEXTURE_COUNT

#ifndef NDEBUG
	#include <string>
#endif

#define FLAG_TEXTURE_CREATE						( 1 << 0 )
#define FLAG_TEXTURE_ALPHA_ADDITIVE				( 1 << 1 )
#define FLAG_TEXTURE_NEAREST					( 1 << 2 )
#define FLAG_TEXTURE_LINEAR						( 1 << 3 )
#define FLAG_TEXTURE_DONT_WRITE_Z				( 1 << 4 )
#define FLAG_TEXTURE_ALPHA_TEST					( 1 << 5 )
#define FLAG_TEXTURE_POLYGON_OFFSET				( 1 << 6 )
#define FLAG_TEXTURE_DISABLE_DEPTH_TEST			( 1 << 7 )

#define FLAG_TEXTURE_FREE_BUFFER				( 1 << 8 )
#define FLAG_TEXTURE_DISABLE_CULLING			( 1 << 9 )

#define MASK_TEXTURE_NEAREST_FREE_BUFFER		( FLAG_TEXTURE_CREATE | FLAG_TEXTURE_NEAREST | FLAG_TEXTURE_FREE_BUFFER)

namespace Lib3D
{

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class TTexture
{
public:

    //Put explicit values to make sure we have a 1:1 correspondance with the codes
    //used in the targa file creator
	enum e_RasterMode
	{
		RAST_OPAQUE   = 1,      // no transparency
		RAST_BINALPHA = 2,      // binary tranparency
		RAST_ALPHA    = 3,      // alpha transparency (50% / 50%)
		RAST_ALPHAMIX = 4,      // mix, binary tranparency + alpha transparency
	};

	enum 
	{
//		TEX_NAMELEN		=	32,                         
		TEX_UV_SHIFT					=	4,	// fixed float bits convertion accuracy for textures
		TEX_UV_SHIFTCorrected	=	4,	// fixed float bits convertion accuracy for textures
		TEX_V_SHIFT		=	4   // to avoid overflow because v coordinates and scaled by Texture->SizeX
	};

private:
	
	void LoadTexture(const char* fileName, bool mirror, const char* palettename, bool doublex, bool doubley, int createGLTextureFlags ); // mirror if needed

public:
	TTexture();
	TTexture(int width, int height);
	TTexture(const char* fileName, int createGLTextureFlags = FLAG_TEXTURE_CREATE | FLAG_TEXTURE_NEAREST);
    TTexture(const char* fileName, bool mirror = false, const char* palette = NULL, bool doublex = false, bool doubley = false, int createGLTextureFlags = FLAG_TEXTURE_CREATE | FLAG_TEXTURE_NEAREST );

	TTexture(A_IFile& file, bool createGLTexture = false);

	~TTexture();

	void CleanBuffers();

	inline void SetGlobalAlpha(unsigned char alpha) { m_globalAlpha = alpha; }
	inline int	GetGlobalAlpha() { return m_globalAlpha; }

	inline void RedirectTo(TTexture* pTexture)
	{		
	#ifdef USE_OGL	
		m_glTextureName = pTexture->m_glTextureName;
		m_nTexUniqueIdForGLTexName = pTexture->m_nTexUniqueIdForGLTexName;
		m_pow2Width = pTexture->m_pow2Width;
		m_pow2Height = pTexture->m_pow2Height;
		m_flags = pTexture->m_flags;
		m_bHasAlphaChannel = pTexture->m_bHasAlphaChannel;
	#else
		m_shtupData = pTexture->m_shtupData;
		m_mask = pTexture->m_mask;
	#endif/* USE_OGL */

	}

	void HackPointers(unsigned short* NewData,
		unsigned short** SaveData = NULL)
	{
		// TODO: change for OGL
		if (SaveData)
			*SaveData = m_shtupData;

		m_shtupData = NewData; 
	}

    inline bool                 IsCreated() const {return(m_blnCreationSuccess);}

	inline int					SizeX() const{return m_sizeX;}
	inline int					SizeY() const{return m_sizeY;}

	inline unsigned short GetTextureColor(int u,int v)const
	{
		TEXTURE_COUNT
		int coord_u = (u >> DIV_SHIFT); 
		int coord_v = (v >> DIV_SHIFT); 

		int idx		= ((coord_u & m_drawMaskX) | ((coord_v & m_drawMaskY) << m_vShift));

		return m_shtupData[idx];   // base color

	}

	inline unsigned short	GetTextureColorCorrected(int u,int v,unsigned short correction)const
	{
		u = u >> (TEX_UV_SHIFT + 6);
		v = v >> (TEX_UV_SHIFT + 6);

		// overflow allowed due to result masking (?)
		return GetTextureColor(u * correction ,v * correction);
	}

	// Transform  [0..1] normalized UV coordinate into useable coordinate
	short		GetU(float f) const 
    {return		Clamp16(int(f*m_sizeX) <<TEX_UV_SHIFT);}

	short		GetV(float f) const 
    {return		Clamp16(int(f*m_sizeY) <<TEX_UV_SHIFT);}

	inline int	VScale() const {return SizeX() >> TEX_V_SHIFT;}

	#ifdef WIN_DEBUG
		void	SaveTarga(const char* name) const;
	#endif

inline	unsigned short* Data() const			{return m_shtupData;}

//inline	unsigned long	DrawMask() const		{A_ASSERT(m_drawMaskX==m_drawMaskY); return m_drawMaskX;}
inline	unsigned long	DrawMaskX() const		{return m_drawMaskX;}
inline	unsigned long	DrawMaskY() const		{return m_drawMaskY;}
inline	int				VShift() const			{return m_vShift;}
inline	int				XShift() const			{return m_xShift;}
inline	int				YShift() const			{return m_yShift;}

	void			FlipHorizontal();

inline	int				MemoryUsed() const	{return m_sizeX * m_sizeY * sizeof(short);}

private:

	inline unsigned short	Get(int u,int v) const	{return m_shtupData[ (u & m_drawMaskX) | ((v & m_drawMaskY) << m_vShift)  ];}
	inline static short		Clamp16(int uv)			{if (uv < SHRT_MIN) uv = SHRT_MIN;else if (uv > SHRT_MAX) uv = SHRT_MAX; return uv;}

    void Init(unsigned long w,unsigned long h, e_RasterMode mode);

private:

	unsigned short		            m_sizeX;                                   
    unsigned short		            m_sizeY;

	unsigned int			        m_drawMaskX;               // rasterisation drawing mask
    unsigned int			        m_drawMaskY;

	unsigned short*					m_originalData;
	unsigned short*                 m_shtupData;               // pixel color array - all variants of the texture

	int								m_xShift; // log2 width
	int								m_yShift; // log2 height
	int								m_vShift; // shift of v tcoord 	
  
    bool                            m_blnCreationSuccess;
	bool							m_bReference;

	int								m_flags;
public:

	inline int GetFlags() { return m_flags;}
#ifdef USE_OGL
	
#ifdef DEBUG_TEX_MEM
	//u32 m_glTexPixelType;
	u32 m_videoMemSize;
#endif

	int m_pow2Width;
	int m_pow2Height;

	bool m_bHasAlphaChannel;

	s16 m_nTexUniqueIdForGLTexName; //this will be in the range in [0 ... k_MAX_TEXTURES_LOADED-1]
	GLuint m_glTextureName;

	typedef struct {
		unsigned char id;
		unsigned short x;
		unsigned short y;
		unsigned short w;
		unsigned short h;
		unsigned char globalAlpha;
	} sAtlasTextureTile;

#define ATLAS_TEXTURE_TILES_MAX		128
#define ATLAS_TEXTURE_WIDTH			1024
#define ATLAS_TEXTURE_HEIGHT		1024

	static void CreateAtlasTexture(TTexture *textOpaque, TTexture *textAlpha,  
								  sAtlasTextureTile *textOpaqueTiles, sAtlasTextureTile *textAlphaTiles,
								  int colSize, int nCols,
								  TTexture **textures, int textures_nb);
	
	static const int k_MAX_TEXTURES_LOADED = 400;
	static u32 s_mapTexUniqueIdToGLTexName[k_MAX_TEXTURES_LOADED]; //array with glTexName

	static void CreateKeyForGLTexName(s16& key, u32 glTextureName) 
	{
	#ifdef _DEBUG
		A_ASSERT(glTextureName != 0);
		//if( glTextureName == 0 )
		//{
		//	//texture could not be created
		//	_asm int 3;
		//}
		
		//verify if a key was created for this texturename ... this should not be happen
		for( int i = k_MAX_TEXTURES_LOADED-1; i >=0; i-- )
		{
			A_ASSERT(s_mapTexUniqueIdToGLTexName[i] != glTextureName);
			//if( s_mapTexUniqueIdToGLTexName[i] == glTextureName )		
			//	_asm int 3;
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

		A_ASSERT(false);
//	#ifdef _DEBUG
//		//could not find a slot ... please increase k_MAX_TEXTURES_LOADED
//		_asm int 3;
//	#endif
	}

	//
	static void FreeKeyForGLTexName(s16& key) 
	{
		if( key >=0 && key < k_MAX_TEXTURES_LOADED )
			s_mapTexUniqueIdToGLTexName[key] = 0;		
		else
		{
			A_ASSERT(false);
//		#ifdef _DEBUG
//			//this should not be happen
//			_asm int 3;
//		#endif 
		}
	}


#endif /* USE_OGL*/

	unsigned char					m_globalAlpha;
	unsigned char*					m_mask;

public:
	char *m_name;

};




}//namespace
#endif // _TEXTURE_H_
