
#ifndef _LIB3DGL_H_
#define _LIB3DGL_H_ 


//!!!ATENTION : MAX_2D_QUADS * VTX_COUNT_PER_QUAD should be  < 2 pow 16 - 1
#define MAX_2D_QUADS			( 2500 )

#define VTX_COUNT_PER_LINE			2

#define VTX_COUNT_PER_QUAD			4
#define VTX_COUNT_PER_TRI			3

#define TRI_COUNT_PER_QUAD			2

#define VTX_SIZE					2
#define COLOR_SIZE					4
#define TEXCOOR_SIZE				2


//specific to decode buffer
#define MAX_BYTES_PER_COLOR			(4)
#define MAX_TEX_WIDTH				(1024) // (512)
#define MAX_TEX_HEIGHT				(1024) // (512)

#define R8FROM565(col565)		( ( (col565) & 0xF800 ) >> 8 )
#define G8FROM565(col565)		( ( (col565) & 0x07E0 ) >> 3 )
#define B8FROM565(col565)		( ( (col565) & 0x001F ) << 3 )

#include "DevUtil.h"
#include "GenDef.h"

#ifdef IPHONE
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>	
#else
	#include <windows.h>
	//#include "GLES/egl.h"
	#include "GL/gl.h"
	#include "GL/glext.h"
	#include "GL/GLES_TO_WGL.h"

	#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG          0x8C00
	#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG          0x8C01
	#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG         0x8C02
	#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG         0x8C03
#endif

#include <math.h>


// Flags for rendering 2D quads
#define FLAG_HAS_COLOR			(1 << 0)
#define FLAG_IS_LINE			(1 << 1)
#define FLAG_USE_ADDITIVE_BLEND	(1 << 2)	//lensflare specific
#define FLAG_SET_CLIP			(1 << 3)
#define FLAG_CLEAR_CLIP			(1 << 4)

#ifdef DEBUG_TEX_MEM
extern int g_TexTotalTexSize;
#endif

class Lib3DGL
{
private:
	int		m_nNum2DQuads;
	s16*	m_pVtxBuffer2D;
	
	u8*		m_pClrBuffer2D;	
	f32*	m_pTexBuffer2D;	
	u32*	m_pTexID;
	u8*		m_pFlags;
	
public:	
	u16		*	m_pIndices; //specific to draw elements
	u16		*	m_pIndicesStrip; //specific to draw elements
	u16		*	m_pLineIndices;
	
	u32 color;

public:
	Lib3DGL();
	~Lib3DGL();

	void		setColor(u32 clr = 0xFFFFFFFF) { color = clr; }

	//FUNCTIONS TO RENDER COLOURED QUADS	
	void		fillRect(int x, int y, int w, int h, u32 color);
	void		fillRectImmediateMode( int x, int y, int w, int h, u32 color);
	void		drawLine( int x_start, int y_start, int x_end, int y_end, u32 color );
	
	void 		fillArc(int x, int y, int radius, int a1, int a2, u32 color);
	void 		drawArc(int x, int y, int radius, int a1, int a2, u32 color);
	
	void 		setClip(int x, int y, int w, int h);
	void 		clearClip();
	
	//FUNCTIONS TO RENDER TEXTURED QUADS
	//add the current module to a list of quads
	//this list will be processed and rendered with OpenGL in Flush2D
	void		paint2DModule( int x, int y, int w, int h, u32 texId, const f32 uv[], int flags, int angle = 0, int rotCenterX = 0, int rotCenterY = 0 );
	//render a quad using OpenGL
	void		paint2DModuleImmediateMode( int x, int y, int w, int h, u32 texId, const f32 uv[], int flags );

	//RENDERING STATE CONTROL
	void		Begin2DRendering();
	void		End2DRendering();
	void		Flush2D();

	//ALLOCATOR/DEALLOCATOR
	void		Init3D();
	void		Clean3D();

	//OpenGL WRAPPERS 
	void		GLEnableFog(u32 mode, f32 density, f32 start, f32 end, f32 red, f32 green, f32 blue, f32 alpha);
	void		GLDisableFog();

	void		GLScissor(int x, int y, int w, int h);
	
	//TEXTURE SPECIFIC
	static void CreateGLTexture(GLuint &glTextureName, 
								u32 width, u32 height, 
								const u8* data, 
								u32 glInternalFormat, u32 glDataFormat, u32 glDataPixelType,
								bool generateMipmap = false
								);

	static void Decode565To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u16* src, int srcWidth, int srcHeight,
							  bool bHasFullyTransp, const u8* pAlphaMask );
	
	static void DecodeP888To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight,
							  const u8* srcPal, int transp_index);

	static void Decode8888To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight);

	static void Decode888To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight, bool rgb = true);

	static void DecodeP888To5551( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight,
							  const u8* srcPal, int transp_index);
	
	static void DecodeP888ToLUMINANCE( u8* dst, int &dstPow2Width, int &dstPow2Height, 
									const u8* src, int srcWidth, int srcHeight,
									const u8* srcPal);

	static void DecodeP888To565( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight,
							  const u8* srcPal);

	static void GetPow2Size(int &dstPow2Width, int &dstPow2Height,
						  int srcWidth, int srcHeight);

	static void ReleaseGLTexture(GLuint glTextureName);

	static u8* s_decodeBuffer;

	static f32 s_fogColor[];

	int GetNumQuads() { return m_nNum2DQuads; }

};

extern Lib3DGL* g_lib3DGL;

#endif /* _LIB3DGL_H_ */
