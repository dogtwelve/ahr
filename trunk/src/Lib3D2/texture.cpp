#include "DevUtil.h"
#include "Texture.h"
#include "File.h"

#include <stdlib.h>
#include <string.h>

using namespace Lib3D;

#ifdef USE_OGL

//definition
u32 TTexture::s_mapTexUniqueIdToGLTexName[TTexture::k_MAX_TEXTURES_LOADED] = {0};

#endif /* USE_OGL */

void GetMaskShift(int size, int &mask, int &shift)
{
	shift = 1;

	while (size > 0 && ((size >> 1) & 0x01) == 0)
	{
		shift++;
		size >>= 1;
	}


	mask = (1 << shift) - 1;
}

typedef struct
{
#define COMPRESS_PAL4			0
#define COMPRESS_RLE4			1
#define COMPRESS_RLE6			2
#define COMPRESS_PAL8			3
#define COMPRESS_PAL4_ONLY		4
#define COMPRESS_PAL6_ONLY		5
#define COMPRESS_PAL8_ONLY		6	
#define COMPRESS_PVRTC			7       //// PowerVR MBX specific

#define COMPRESS_HALF_HEIGHT_F	0x0080	// flag for half height (should be mirrored)
#define TEXTURE_MASK_FLAG		0x0040	// flag for mask (which follows the image data)
#define UNCOMPRESSED			0x0010
#define ALPHA_CHANNEL			0x0020

	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width=heigh <=256 
	unsigned short	height;
	//Jogy
	//We must support files larger than 64kb  
	unsigned long	datasize;		// file size	
	//unsigned short	datasize;		// file size
//	unsigned short	palette[];		// palette  (16 or 64 or 256 elements)

} RLE_HEADER;



unsigned short	COLOR_AVERAGE( unsigned short a, unsigned short b )
{
	if ((a == 0) || (b == 0))
		return 0;

	return (	((((a & 0xF800) + (b & 0xF800)) >> 1) & 0xF800) |
				((((a & 0x7E0) + (b & 0x7E0)) >> 1) & 0x7E0) |
				((((a & 0x1F) + (b & 0x1F)) >> 1) & 0x1F) );
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

TTexture::TTexture(int width, int height)
	:m_blnCreationSuccess(true),
    m_shtupData(0),
	m_originalData(0),
	m_bReference(true),
	m_globalAlpha(0xFF),
	m_mask(0),
	m_flags(0),
	m_name(0)
{

#ifdef USE_OGL
	m_glTextureName = 0;
	m_nTexUniqueIdForGLTexName = -1;
#endif /* USE_OGL */

	m_sizeX = static_cast<unsigned short>(width);
	m_sizeY = static_cast<unsigned short>(height);

	GetMaskShift(m_sizeX, (int&)m_drawMaskX, (int&)m_xShift);
	GetMaskShift(m_sizeY, (int&)m_drawMaskY, (int&)m_yShift);
	m_vShift = m_xShift;
}

void TTexture::LoadTexture(const char* fileName, bool mirror, const char* palettename, bool doublex, bool doubley, int createGLTextureFlags ) // mirror if needed
{
#ifdef USE_OGL
	m_glTextureName = 0;
	m_nTexUniqueIdForGLTexName = -1;
#endif/* USE_OGL */


	bool bHasFullyTransp = false;

	A_ASSERT(fileName);

	m_name = strdup(fileName);

	A_IFile* filpTexture;

    //It could be interesting to have this another way, but right now, if you want to know if a
    //texture file opened, you can poll this boolean via its get function.
	filpTexture = A_IFile::Open(fileName, A_IFile::OPEN_BINARY | A_IFile::OPEN_READ, false);
    if(filpTexture == 0)
    {
        m_blnCreationSuccess = false;

		debug_out("Cant open %s\n", fileName);
		A_ASSERT( 0 );		
        return;
    }

	//Get the header
	RLE_HEADER	rleh;
	int n = sizeof(RLE_HEADER);
	filpTexture->Read(&rleh, sizeof(RLE_HEADER));


#ifdef USE_OGL
	if(rleh.rle == COMPRESS_PVRTC)
	{	
		filpTexture->Read( g_lib3DGL->s_decodeBuffer, rleh.datasize );
		A_IFile::Close(filpTexture);

		PVR_Texture_Header texHeader = *((PVR_Texture_Header*)g_lib3DGL->s_decodeBuffer);		

		m_pow2Width = texHeader.dwWidth;
		m_pow2Height = texHeader.dwHeight;

		m_bHasAlphaChannel = (texHeader.dwAlphaBitMask!=0);

#ifndef IPHONE
		int ret = PVRTLoadTextureFromPointer(g_lib3DGL->s_decodeBuffer, &m_glTextureName);		
		::glBindTexture( GL_TEXTURE_2D, m_glTextureName );
#else
		int textureFormat = 0;

		switch( texHeader.dwpfFlags & 0xFF )
		{
			case OGL_PVRTC2:
				textureFormat = texHeader.dwAlphaBitMask==0 ? GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG : GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG ;	// PVRTC2
				break;
			case OGL_PVRTC4:
				textureFormat = texHeader.dwAlphaBitMask==0 ? GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG ;	// PVRTC4
				break;		
		}

		::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	
	
		::glEnable (GL_TEXTURE_2D);
		::glGenTextures( 1, &m_glTextureName );	
		::glBindTexture( GL_TEXTURE_2D, m_glTextureName );

		::glCompressedTexImage2D(GL_TEXTURE_2D, 0, textureFormat, m_pow2Width, m_pow2Height, 0, texHeader.dwTextureDataSize, g_lib3DGL->s_decodeBuffer + texHeader.dwHeaderSize);

#endif // WIN32		

		int filter = (createGLTextureFlags & FLAG_TEXTURE_LINEAR) ? GL_LINEAR : GL_NEAREST;
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//cdp create the key( TexUniqueId) for this name
		TTexture::CreateKeyForGLTexName( m_nTexUniqueIdForGLTexName, m_glTextureName);
		
		//Init(rleh.width, rleh.height, RAST_OPAQUE);
		m_sizeX = rleh.width;
		m_sizeY = rleh.height;

		// init rasterization parameters
		GetMaskShift(m_sizeX, (int&)m_drawMaskX, (int&)m_xShift);
		GetMaskShift(m_sizeY, (int&)m_drawMaskY, (int&)m_yShift);
		m_vShift = m_xShift;
	
		m_originalData	= NULL;
		m_shtupData		= m_originalData;

#ifdef DEBUG_TEX_MEM
		m_videoMemSize = texHeader.dwTextureDataSize;

		debug_out("CREATED PVR texture name: [%d] [%s] [%d , %d] [alpha: %d]\n", m_glTextureName, m_name, m_pow2Width, m_pow2Height, m_bHasAlphaChannel ? 1 : 0);
		g_TexTotalTexSize += m_videoMemSize;
		debug_out("TOTAL_SIZE:[%d]\n", (g_TexTotalTexSize / 1024));
#endif // DEBUG_TEX_MEM

		return;
	}
	else if (rleh.rle & UNCOMPRESSED)
	{
		int		half_height = rleh.rle & COMPRESS_HALF_HEIGHT_F;

		bool bHasMask = false;
		if (rleh.rle & TEXTURE_MASK_FLAG)
			bHasMask = true;

		// if half height, the height is in fact the width
		if (half_height && mirror)
			rleh.height = rleh.width;

		Init(rleh.width, rleh.height, RAST_OPAQUE);

		unsigned char*	temp = NEW unsigned char[ rleh.datasize ];

		filpTexture->Read(temp, rleh.datasize);

		m_bHasAlphaChannel = false;

		// TODO: mirror, mask

		if (createGLTextureFlags & FLAG_TEXTURE_CREATE)
		{
			g_lib3DGL->Decode888To8888( g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
										temp, m_sizeX, m_sizeY, false);
			
			g_lib3DGL->CreateGLTexture(m_glTextureName, 
										m_pow2Width, m_pow2Height, 
										g_lib3DGL->s_decodeBuffer, 
										m_bHasAlphaChannel ? GL_RGBA : GL_RGB, m_bHasAlphaChannel ? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE
										);

			::glBindTexture(GL_TEXTURE_2D, m_glTextureName);

			int filter = (createGLTextureFlags & FLAG_TEXTURE_LINEAR) ? GL_LINEAR : GL_NEAREST;
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			
			//cdp create the key( TexUniqueId) for this name
			TTexture::CreateKeyForGLTexName( m_nTexUniqueIdForGLTexName, m_glTextureName);

			#ifdef DEBUG_TEX_MEM
				//m_glTexPixelType = GL_UNSIGNED_BYTE;
				m_videoMemSize = m_pow2Width * m_pow2Height * (m_bHasAlphaChannel ? 4 : 3);
			#endif
		}

		SAFE_DELETE_ARRAY(temp);
		
		CleanBuffers();

		A_IFile::Close(filpTexture);
		return;
	}
#endif /* USE_OGL */


	// DOUBLE SIZE HANDLING
	if (doublex)
		rleh.width *= 2;
	if (doubley)
		rleh.height *= 2;


	A_ASSERT(rleh.datasize>0);// this will crash the ngage (but not the emulator or the windows version)

    unsigned short	palette[256];	// palette  (16 or 64 or 256 elements)

	// get half height flag (only half of the texture has been saved, we need to mirror it)
	int		half_height = rleh.rle & COMPRESS_HALF_HEIGHT_F;
	rleh.rle &= ~COMPRESS_HALF_HEIGHT_F;

	bool bHasMask = false;
	if (rleh.rle & TEXTURE_MASK_FLAG)
		bHasMask = true;

	rleh.rle &= ~TEXTURE_MASK_FLAG;

	// if half height, the height is in fact the width
	if (half_height && mirror)
		rleh.height = rleh.width;

	if ((rleh.rle == COMPRESS_RLE4)||(rleh.rle == COMPRESS_PAL4))
		filpTexture->Read(palette, 2*16);
	if (rleh.rle == COMPRESS_RLE6)
		filpTexture->Read(palette, 2*64);
	if (rleh.rle == COMPRESS_PAL8)
		filpTexture->Read(palette, 2*256);

	// if custom PALETTE
	if (palettename && *palettename)
	{
		A_IFile*	pal = A_IFile::Open( palettename, A_IFile::OPEN_BINARY | A_IFile::OPEN_READ, false);
		if(filpTexture == 0)
		{
			m_blnCreationSuccess = false;
			A_ASSERT( 1 );

			TRACE("     palette load failed\n");
			return;
		}

		int nbColors = pal->GetChar() & 0xFF;
		if (nbColors == 0)
			nbColors = 256;

		pal->Read(palette, 2 * nbColors);

	    A_IFile::Close(pal);
	}

#ifdef __565_RENDERING__
	// convert the palette to 565 format
	{

		unsigned short* p = palette;
		int				nb;

		if ((rleh.rle == COMPRESS_RLE4)||(rleh.rle == COMPRESS_PAL4))
			nb = 16;
		else if (rleh.rle == COMPRESS_RLE6)
			nb = 64;
		else if (rleh.rle == COMPRESS_PAL8)
			nb = 256;
		else
			A_ASSERT( 0 );

		//after this the fully transparent will be mapped on color = 0;
		while (nb--)
		{
			unsigned short	col = *p;
			
			if (col == 0xFF0F)
			{
				// set keycolor as 0
				bHasFullyTransp = true;
				*p = 0;
			}
			else
			{
				unsigned short	r = (col & 0xF) << 1;			//5
				unsigned short	g = ((col & 0xF0)>>4) << 2;		//6
				unsigned short	b = ((col & 0xF00)>>8) << 1;	//5
				unsigned short	h = (col & 0xF000)>>12;

				// clear higher bits on source texture
				if (h & 0x1)
					r |= 0x1;
				if (h & 0x2)
					g |= 0x2;
				if (h & 0x4)
					b |= 0x1;

				col = (b << (5+6)) | (g << 5) | r;

				if (col == 0)
				{		
					//avoid fully transparent
					// if 0, set to non zero
					col = 0x1;
				}

				*p = col;
			}

			p++;
		}

	}
#endif


	Init(rleh.width, rleh.height, RAST_OPAQUE);


	//If not, we will have to modify the read function
	A_ASSERT(sizeof(unsigned short) == 2 * sizeof(unsigned char));

	unsigned char*	temp = NEW unsigned char[ rleh.datasize ];

	filpTexture->Read(temp, rleh.datasize);

	if (rleh.rle == COMPRESS_RLE4)
	{
		// decompress RLE
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			int				repeat;
			int				idx;
			
			idx = (*p) & 0x0F;
			repeat = ((*p) >> 4) + 1;

			unsigned short	col = palette[idx];
			while( repeat--)
			{
				*out++ = col;
				if (doublex)
					*out++ = col;
			}
			p++;
		}
	}
	else 
	if (rleh.rle == COMPRESS_RLE6)
	{
		// decompress RLE
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			int				repeat;
			int				idx;
			
			idx = (*p) & 0x3F;
			repeat = ((*p) >> 6) + 1;

			unsigned short	col = palette[idx];
			while( repeat--)
			{
				*out++ = col;
				if (doublex)
					*out++ = col;
			}
			p++;
		}
	}
	else
	if (rleh.rle == COMPRESS_PAL4)
	{
		// decompress 4b
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			int				idx0, idx1;
			
			idx0 = (*p) & 0x0F;
			idx1 = (*p) >> 4;

			*out++ = palette[idx0];
			if (doublex)
				*out++ = palette[idx0];
			*out++ = palette[idx1];
			if (doublex)
			*out++ = palette[idx1];
			p++;
		}
	}
	else 
	if (rleh.rle == COMPRESS_PAL8)
	{
		// decompress 8b
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			*out++ = palette[*p];
			if (doublex)
				*out++ = palette[*p];
			p++;
		}
	}


	// DOUBLEY handling
	if (doubley)
	{
        for (int i = rleh.height/2 - 1; i >= 0; i--)
		{
			memcpy( m_shtupData + ((i*2 + 1) * rleh.width), m_shtupData + (i * rleh.width), rleh.width * 2 );
			memcpy( m_shtupData + ((i*2) * rleh.width), m_shtupData + (i * rleh.width), rleh.width * 2 );
		}
	}


	DELETE_ARRAY temp;

	// MIRROR if half height and mirror needed
	if ((half_height) && mirror)
	{
		unsigned short*		out = m_shtupData + (rleh.width*(rleh.width>>1));
		unsigned short*		in = m_shtupData + (rleh.width*((rleh.width-1)>>1));
		int					i = rleh.width>>1;

		while (i--)
		{
			memcpy( out, in, rleh.width<<1);
			in -= rleh.width;
			out += rleh.width;
		}		
	}

	m_mask = 0;

	if (bHasMask)
	{
		int size = rleh.width * rleh.height;
		// rleh.datasize
		m_mask = new unsigned char[size];
		filpTexture->Read(m_mask, size);
	}


#ifdef USE_OGL
	
	m_bHasAlphaChannel = false;
	if( bHasFullyTransp || m_mask != 0 )
	{
		m_bHasAlphaChannel = true;
	}

	if (createGLTextureFlags & FLAG_TEXTURE_CREATE)
	{
#ifdef USE_TEXTURES_SHORT_FORMATS
		if (!bHasFullyTransp && !m_mask)
		{
			g_lib3DGL->GetPow2Size(m_pow2Width, m_pow2Height, m_sizeX, m_sizeY);

			memset(g_lib3DGL->s_decodeBuffer, 0, m_pow2Width * m_pow2Height * 2);
			for (int i=m_sizeY - 1; i >= 0; --i)
				memcpy(g_lib3DGL->s_decodeBuffer + i * m_pow2Width * 2, m_originalData + i * m_sizeX, (m_sizeX << 1) );

			g_lib3DGL->CreateGLTexture(m_glTextureName, 
				m_pow2Width, m_pow2Height, 
				g_lib3DGL->s_decodeBuffer,
				GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
			
#ifdef DEBUG_TEX_MEM
			//m_glTexPixelType = GL_UNSIGNED_SHORT_5_6_5;
			m_videoMemSize = m_pow2Width * m_pow2Height * 2;
#endif
		}
		else
#endif // USE_TEXTURES_SHORT_FORMATS
		{
			g_lib3DGL->Decode565To8888( g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
										m_originalData, m_sizeX, m_sizeY,		
										m_bHasAlphaChannel, (u8*)m_mask);
			
			g_lib3DGL->CreateGLTexture(m_glTextureName, 
										m_pow2Width, m_pow2Height, 
										g_lib3DGL->s_decodeBuffer, 
										m_bHasAlphaChannel ? GL_RGBA : GL_RGB, m_bHasAlphaChannel ? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE
										);
			
#ifdef DEBUG_TEX_MEM
			//m_glTexPixelType = GL_UNSIGNED_BYTE;
			m_videoMemSize = m_pow2Width * m_pow2Height * (m_bHasAlphaChannel ? 4 : 3);
#endif			
		}

		::glBindTexture(GL_TEXTURE_2D, m_glTextureName);

		int filter = (createGLTextureFlags & FLAG_TEXTURE_LINEAR) ? GL_LINEAR : GL_NEAREST;
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		//cdp create the key( TexUniqueId) for this name
		TTexture::CreateKeyForGLTexName( m_nTexUniqueIdForGLTexName, m_glTextureName);

	}

#endif /* USE_OGL*/

	if (createGLTextureFlags & FLAG_TEXTURE_FREE_BUFFER)
	{
		CleanBuffers();
	}
	
	A_IFile::Close(filpTexture);
}

void TTexture::CleanBuffers()
{
	if (!m_bReference)
	{
		SAFE_DELETE_ARRAY(m_originalData);	
		SAFE_DELETE_ARRAY(m_mask);		
		m_shtupData = NULL;
	}
}

TTexture::TTexture(const char* fileName, bool mirror, const char* palettename, bool doublex, bool doubley, int createGLTextureFlags ) // mirror if needed
    :m_blnCreationSuccess(true)
    ,m_shtupData(0)
	,m_originalData(0)
	,m_bReference(false)
	,m_globalAlpha(0xFF)
	,m_mask(0)
	,m_flags(createGLTextureFlags)
	,m_name(0)
{
	LoadTexture(fileName, mirror, palettename, doublex, doubley, createGLTextureFlags);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

TTexture::TTexture(const char* fileName, int createGLTextureFlags ) // mirror if needed
    :m_blnCreationSuccess(true)
    ,m_shtupData(0)
	,m_originalData(0)
	,m_bReference(false)
	,m_globalAlpha(0xFF)
	,m_mask(0)
	,m_flags(createGLTextureFlags)
	,m_name(0)
{
	LoadTexture(fileName, false, NULL, false, false, createGLTextureFlags);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

TTexture::TTexture(A_IFile& file, bool createGLTexture )
	:m_bReference(false)	
	,m_globalAlpha(0xFF)
	,m_shtupData(0)
	,m_originalData(0)
	,m_mask(0)
	,m_flags(0)
	,m_name(0)
{

	bool bHasFullyTransp = false;

#ifdef USE_OGL
	m_glTextureName = 0;
	m_nTexUniqueIdForGLTexName = -1;
#endif /* USE_OGL */

	RLE_HEADER	rleh;
	file.Read(&rleh, sizeof(RLE_HEADER));

	A_ASSERT(rleh.datasize>0);// this will crash the ngage (but not the emulator or the windows version)

    unsigned short	palette[256];	// palette  (16 or 64 or 256 elements)

	// get half height flag (only half of the texture has been saved, we need to mirror it)
	int		half_height = rleh.rle & COMPRESS_HALF_HEIGHT_F;
	rleh.rle &= ~COMPRESS_HALF_HEIGHT_F;

	// if half height, the height is in fact the width

	if ((rleh.rle == COMPRESS_RLE4)||(rleh.rle == COMPRESS_PAL4))
		file.Read(palette, 2*16);
	if (rleh.rle == COMPRESS_RLE6)
		file.Read(palette, 2*64);
	if (rleh.rle == COMPRESS_PAL8)
		file.Read(palette, 2*256);


#ifdef __565_RENDERING__
	// convert the palette to 565 format
	{
		unsigned short* p = palette;
		int				nb;

		if ((rleh.rle == COMPRESS_RLE4)||(rleh.rle == COMPRESS_PAL4))
			nb = 16;
		else if (rleh.rle == COMPRESS_RLE6)
			nb = 64;
		else if (rleh.rle == COMPRESS_PAL8)
			nb = 256;
		else
			A_ASSERT( 0 );

		while (nb--)
		{
			unsigned short	col = *p;
			
			if (col == 0xFF0F)
			{
				// set keycolor as 0
				*p = 0;
				bHasFullyTransp = true;
			}
			else
			{
				unsigned short	r = (col & 0xF) << 1;	//5
				unsigned short	g = ((col & 0xF0)>>4) << 2; //6
				unsigned short	b = ((col & 0xF00)>>8) << 1;	//5
				unsigned short	h = (col & 0xF000)>>12;

				// clear higher bits on source texture
				if (h & 0x1)
					r |= 0x1;
				if (h & 0x2)
					g |= 0x2;
				if (h & 0x4)
					b |= 0x1;

				col = (b << (5+6)) | (g << 5) | r;

				if (col == 0)
				{
					// if 0, set to non zero
					col = 0x1;
				}

				*p = col;
			}

			p++;
		}

	}
#endif

	Init(rleh.width, rleh.height, RAST_OPAQUE);

	//If not, we will have to modify the read function
	A_ASSERT(sizeof(unsigned short) == 2 * sizeof(unsigned char));

	unsigned char*	temp = NEW unsigned char[ rleh.datasize ];

	file.Read(temp, rleh.datasize);

	if (rleh.rle == COMPRESS_RLE4)
	{
		// decompress RLE
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			int				repeat;
			int				idx;
			
			idx = (*p) & 0x0F;
			repeat = ((*p) >> 4) + 1;

			unsigned short	col = palette[idx];
			while( repeat--)
			{
				*out++ = col;
			}
			p++;
		}
	}
	else 
	if (rleh.rle == COMPRESS_RLE6)
	{
		// decompress RLE
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			int				repeat;
			int				idx;
			
			idx = (*p) & 0x3F;
			repeat = ((*p) >> 6) + 1;

			unsigned short	col = palette[idx];
			while( repeat--)
			{
				*out++ = col;
			}
			p++;
		}
	}
	else
	if (rleh.rle == COMPRESS_PAL4)
	{
		// decompress 4b
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			int				idx0, idx1;
			
			idx0 = (*p) & 0x0F;
			idx1 = (*p) >> 4;

			*out++ = palette[idx0];
			*out++ = palette[idx1];
			p++;
		}
	}
	else 
	if (rleh.rle == COMPRESS_PAL8)
	{
		// decompress 8b
		unsigned short*		out = m_shtupData;
		unsigned char*		p = temp;
		int					nb = rleh.datasize;
		while (nb--)
		{
			*out++ = palette[*p++];
		}
	}

#ifdef USE_OGL
	
	m_bHasAlphaChannel = bHasFullyTransp;

	if (createGLTexture)
	{
		//prepare the texture buffer ... 
		//include this image in a texture with (width, height) pow2
		g_lib3DGL->Decode565To8888( g_lib3DGL->s_decodeBuffer, m_pow2Width, m_pow2Height, 
									m_originalData, m_sizeX, m_sizeY,
									m_bHasAlphaChannel, (u8*)m_mask);

		g_lib3DGL->CreateGLTexture(m_glTextureName, 
									m_pow2Width, m_pow2Height, 
									g_lib3DGL->s_decodeBuffer, 
									m_bHasAlphaChannel ? GL_RGBA : GL_RGB, m_bHasAlphaChannel ? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE
									);

		::glBindTexture(GL_TEXTURE_2D, m_glTextureName);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		//cdp create the key( TexUniqueId) for this name
		TTexture::CreateKeyForGLTexName( m_nTexUniqueIdForGLTexName, m_glTextureName);

		#ifdef DEBUG_TEX_MEM
			//m_glTexPixelType = GL_UNSIGNED_BYTE;
			m_videoMemSize = m_pow2Width * m_pow2Height * (m_bHasAlphaChannel ? 4 : 3);
		#endif
	}

#endif /* USE_OGL*/

	DELETE_ARRAY temp;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
TTexture::~TTexture()
{
    if (!m_bReference)
	{
		CleanBuffers();

		#ifdef USE_OGL
			//release the gl texture name only if this is the original texture( not a reference )

			if (m_glTextureName > 0)
			{
				g_lib3DGL->ReleaseGLTexture(m_glTextureName);
				
			#ifdef DEBUG_TEX_MEM

				//int bytesPerPixel = ( m_bHasAlphaChannel ? 4 : 3);

				//if(m_glTexPixelType == GL_UNSIGNED_SHORT_5_6_5 || m_glTexPixelType == GL_UNSIGNED_SHORT_5_5_5_1)
				//{
				//	bytesPerPixel = 2;
				//}
				//int texsize = m_pow2Width * m_pow2Height * bytesPerPixel;

				g_TexTotalTexSize -= m_videoMemSize; // texsize;
				debug_out("TTexture RELEASE[%d] TOTAL_SIZE:[%d] [%d , %d] [alpha: %d]\n", m_glTextureName, (g_TexTotalTexSize / 1024), m_pow2Width, m_pow2Height, m_bHasAlphaChannel );
			#endif //DEBUG_TEX_MEM

				//cornel to do free the key( TexUniqueId ) for this name
				TTexture::FreeKeyForGLTexName( m_nTexUniqueIdForGLTexName );
			}

			m_glTextureName = 0;
		#endif /* USE_OGL*/		
	}

	SAFE_DELETE_ARRAY(m_name);
}

//-----------------------------------------------------------------------------------------------------
//	This is ok as long as the class does not include conplex objects
//-----------------------------------------------------------------------------------------------------
TTexture::TTexture()
	:m_bReference(false)
	,m_globalAlpha(0xFF)
	,m_originalData(0)
	,m_mask(0)
	,m_flags(0)
	,m_name(0)
{
	//memset(this,0,sizeof(*this));

//#if WIN_DEBUG
//	m_name = "---";
//	gUsedTextures.insert(this);    
//#endif
	
    m_blnCreationSuccess = true;

#ifdef USE_OGL
	m_glTextureName = 0;
	m_nTexUniqueIdForGLTexName = -1;
#endif /* USE_OGL */
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

void TTexture::Init(unsigned long w,unsigned long h, e_RasterMode mode)
{
	m_sizeX = static_cast<unsigned short>(w);
	m_sizeY = static_cast<unsigned short>(h);

	// init rasterization parameters
	GetMaskShift(m_sizeX, (int&)m_drawMaskX, (int&)m_xShift);
	GetMaskShift(m_sizeY, (int&)m_drawMaskY, (int&)m_yShift);
	m_vShift = m_xShift;

	if(mode != RAST_OPAQUE)
		mode = RAST_BINALPHA;
	
	m_originalData	= NEW unsigned short[w*h];
	m_shtupData		= m_originalData;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
static void FlipHorizontal(unsigned short* buffer,const int sizeX,const int sizeY)
{
	unsigned short* pixelLine = buffer;
	const unsigned short* const endLine = buffer + (sizeX * sizeY);
	while(pixelLine != endLine)
	{
		unsigned short* pixel1 = pixelLine;
		pixelLine += sizeX;
		unsigned short* pixel2 = pixelLine-1;

		while(pixel1 < pixel2)
		{
			// swap
			const unsigned short tmp = *pixel1;
			*pixel1 = *pixel2;
			*pixel2 = tmp;
			
			pixel1++;
			pixel2--;
		}
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void TTexture::FlipHorizontal()
{	
	::FlipHorizontal(m_shtupData, m_sizeX, m_sizeY);
}


#ifdef WIN_DEBUG

static unsigned char To8Bits(unsigned short c,int shift)
{
	unsigned char col = (c >> shift) & 0x0F;

	col = col | col << 4;

	return col;
}


void TTexture::SaveTarga(const char* name) const
{
	// TGA file header
	typedef unsigned char byte;

	struct Header
	{
		 byte numIden;
		 byte colorMapType;
		 byte imageType;
		 byte colorMapSpec[5];
		 byte origX[2];
		 byte origY[2];
		 byte width[2];
		 byte height[2];
		 byte bpp;
		 byte imageDes;
	};


	int dataSize = sizeof(Header) + (m_sizeX * m_sizeY * 3);
	
	unsigned char* data = NEW unsigned char[dataSize];

	Header* header = (Header*)data;
	unsigned char*	rgbData = data + sizeof(Header);

	::memset(header,0,sizeof(Header));

	
	header->imageType    = 2;
	header->width[0] = (m_sizeX & 0x00FF);
	header->width[1] = (m_sizeX & 0xFF00) >> 8;
	header->height[0] = (m_sizeY & 0x00FF);
	header->height[1] = (m_sizeY & 0xFF00) >> 8;
	header->bpp = 24;

	for(int y=0;y<m_sizeY;y++)
	{
		for(int x=0;x<m_sizeX;x++)
		{
			unsigned char* rgb = rgbData + ((y*m_sizeX) + x) * 3;

			unsigned short pixel12bits = m_shtupData[y*m_sizeY + x];

			rgb[0] = ::To8Bits(pixel12bits,0);
			rgb[1] = ::To8Bits(pixel12bits,4);
			rgb[2] = ::To8Bits(pixel12bits,8);
		}
	}


	FILE* f = fopen(name,"wb");
	if(f)
	{
		fwrite(data,dataSize,1,f);
		fclose(f);
	}	
	DELETE_ARRAY data;
}
#endif


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
inline static int Log2(const int x)
{
	if(x & 0xF0)
	{
		if(x & 0xC0)
		{
			if(x & 0x80)
				return 7;			
			else
				return 6;
		}
		else
		{
			if(x & 0x20)
				return 5;			
			else
				return 4;
		}
	}
	else
	{
		if(x & 0x0C)
		{
			if(x & 0x08)
				return 3;
			else
				return 2;
		}
		else
		{
			if(x & 0x02)
				return 1;			
			else
				return 0;	
		}
	}
}

#ifdef USE_OGL

// TODO: optimize for speed
bool findFirstEmptyCell(unsigned char **matrix, int width, int height, int &x, int &y)
{
	int w = width / 8;
	int h = height / 8;

	bool bOk = false;

	for (int i=0; i<128 && !bOk; ++i)
	{
		for (int j=0; j<128; ++j)
		{
			if (matrix[i][j] == 1)
				continue;

			y = i;
			x = j;

			bOk = true;
			if (x + w > 128 || y + h > 128)
				bOk = false;
			
			for (int k=0; k<h && bOk; k++)
			{
				for (int l=0; l<w; l++)
				{
					if (matrix[i+k][j+l] == 1)
					{						
						bOk = false;
						break;
					}
				}
			}

			if (bOk)
				break;
		}
	}

	if (bOk)
	{
		for (int i=y; i<y+h; i++)
			for (int j=x; j<x+w; j++)
				matrix[i][j] = 1;

		x *= 8;
		y *= 8;
	}

	return bOk;
}

int findMaxHeight(unsigned char **matrix)
{
	for (int i=127; i>=0; --i)
	{
		int a = 0;
		for (int j=0; j<128; ++j)
		{
			if (matrix[i][j] != 0)
			{
				a = 1;
				break;
			}
		}

		if (a == 1)
			return ((i+1) * 8);
	}

	return 128 * 8;
}


void TTexture::CreateAtlasTexture(TTexture *textOpaque, TTexture *textAlpha,  
								  sAtlasTextureTile *textOpaqueTiles, sAtlasTextureTile *textAlphaTiles,
								  int colSize, int nCols,
								  TTexture **textures, int textures_nb)
{
	//int size = nCols * colSize;
	int sizeX = ATLAS_TEXTURE_WIDTH;
	int sizeY = ATLAS_TEXTURE_HEIGHT;

	unsigned char **fillMatrixAlpha; // 8 x 8 cells
	unsigned char **fillMatrixOpaque; // 8 x 8 cells

	fillMatrixAlpha = new unsigned char*[128];
	fillMatrixOpaque = new unsigned char*[128];	
	for (int i=0; i<128; i++)
	{
		fillMatrixAlpha[i] = new unsigned char[128];
		memset(fillMatrixAlpha[i], 0, 128);

		fillMatrixOpaque[i] = new unsigned char[128];
		memset(fillMatrixOpaque[i], 0, 128);
	}

	textOpaque->Init(sizeX, sizeY, RAST_OPAQUE);
	textAlpha->Init(sizeX, sizeY, RAST_OPAQUE);

	memset(textOpaque->m_shtupData, 0, textOpaque->SizeX() * textOpaque->SizeY() * 2);
	memset(textAlpha->m_shtupData, 0, textAlpha->SizeX() * textAlpha->SizeY() * 2);

	memset(textOpaqueTiles, 0xFF, ATLAS_TEXTURE_TILES_MAX * sizeof (sAtlasTextureTile));
	memset(textAlphaTiles, 0xFF, ATLAS_TEXTURE_TILES_MAX * sizeof (sAtlasTextureTile));

	int indexOpaque = 0;
	int indexAlpha = 0;

	unsigned short *dataOpaque = textOpaque->Data();
	unsigned short *dataAlpha = textAlpha->Data();

//	int delayed[32];
//	int nDelayed = 0;

	A_ASSERT(textures_nb <= ATLAS_TEXTURE_TILES_MAX);
	for (int i=0; i<textures_nb; i++)
	{
		TTexture *tmpTexture = textures[i];

		if (tmpTexture->m_bHasAlphaChannel)
		{
			int startX = 0;
			int startY = 0;
			bool bFound = findFirstEmptyCell(fillMatrixAlpha, tmpTexture->SizeX(), tmpTexture->SizeY(), startX, startY);

			A_ASSERT(bFound);

			for (int k=0; k<tmpTexture->SizeY(); ++k)
				memcpy(dataAlpha + (startY + k) * textAlpha->SizeX() + startX, tmpTexture->Data() + k * tmpTexture->SizeX(), tmpTexture->SizeX() * 2);

			textAlphaTiles[i].id = indexAlpha++;
			textAlphaTiles[i].x = startX;
			textAlphaTiles[i].y = startY;
			textAlphaTiles[i].w = tmpTexture->SizeX();
			textAlphaTiles[i].h = tmpTexture->SizeY();
			textAlphaTiles[i].globalAlpha = tmpTexture->GetGlobalAlpha();
		}
		else
		{
			int startX = 0;
			int startY = 0;
			bool bFound = findFirstEmptyCell(fillMatrixOpaque, tmpTexture->SizeX(), tmpTexture->SizeY(), startX, startY);

			A_ASSERT(bFound);

			for (int k=0; k<tmpTexture->SizeY(); ++k)
				memcpy(dataOpaque + (startY + k) * textOpaque->SizeX() + startX, tmpTexture->Data() + k * tmpTexture->SizeX(), tmpTexture->SizeX() * 2);

			textOpaqueTiles[i].id = indexOpaque++;
			textOpaqueTiles[i].x = startX;
			textOpaqueTiles[i].y = startY;
			textOpaqueTiles[i].w = tmpTexture->SizeX();
			textOpaqueTiles[i].h = tmpTexture->SizeY();
			textOpaqueTiles[i].globalAlpha = tmpTexture->GetGlobalAlpha();
		}
	}

	/*for (int j=0; j<nDelayed; ++j)
	{
		int i = delayed[j];
		TTexture *tmpTexture = textures[j];

		int startX = 0;
		int startY = 0;
		bool bFound = findFirstEmptyCell(fillMatrixOpaque, tmpTexture->SizeX(), tmpTexture->SizeY(), startX, startY);

		A_ASSERT(bFound);

		for (int k=0; k<tmpTexture->SizeY(); ++k)
			memcpy(dataOpaque + (startY + k) * textOpaque->SizeX() + startX, tmpTexture->Data() + k * tmpTexture->SizeX(), tmpTexture->SizeX() * 2);

		textOpaqueTiles[i].id = indexOpaque++;
		textOpaqueTiles[i].x = startX;
		textOpaqueTiles[i].y = startY;
		textOpaqueTiles[i].w = tmpTexture->SizeX();
		textOpaqueTiles[i].h = tmpTexture->SizeY();
	}
	*/

	int new_size_y = findMaxHeight(fillMatrixOpaque);

	//prepare the texture buffer ... 
	//include this image in a texture with (width, height) pow2
	
#ifdef USE_TEXTURES_SHORT_FORMATS
	
	g_lib3DGL->GetPow2Size(textOpaque->m_pow2Width, textOpaque->m_pow2Height, textOpaque->SizeX(), new_size_y);

	g_lib3DGL->CreateGLTexture(textOpaque->m_glTextureName, 
								textOpaque->m_pow2Width, textOpaque->m_pow2Height, 
								(u8*)dataOpaque, 
								GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
#else

	g_lib3DGL->Decode565To8888( g_lib3DGL->s_decodeBuffer, textOpaque->m_pow2Width, textOpaque->m_pow2Height, 
								dataOpaque, textOpaque->SizeX(), new_size_y,
								false, NULL);
	
	g_lib3DGL->CreateGLTexture(textOpaque->m_glTextureName, 
								textOpaque->m_pow2Width, textOpaque->m_pow2Height, 
								g_lib3DGL->s_decodeBuffer, 
								GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
#endif // USE_TEXTURES_SHORT_FORMATS
		
	//set filter
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//cdp create the key( TexUniqueId) for this name
	TTexture::CreateKeyForGLTexName( textOpaque->m_nTexUniqueIdForGLTexName, textOpaque->m_glTextureName );

	//unset hasAlphaChannel
	textOpaque->m_bHasAlphaChannel = false;

	#ifdef DEBUG_TEX_MEM
		#ifdef USE_TEXTURES_SHORT_FORMATS
			//textOpaque->m_glTexPixelType = GL_UNSIGNED_SHORT_5_6_5;
			textOpaque->m_videoMemSize = textOpaque->m_pow2Width * textOpaque->m_pow2Height * 2;
		#else
			//textOpaque->m_glTexPixelType = GL_UNSIGNED_BYTE;
			textOpaque->m_videoMemSize = textOpaque->m_pow2Width * textOpaque->m_pow2Height * 3;
		#endif
	#endif


	new_size_y = findMaxHeight(fillMatrixAlpha);

	g_lib3DGL->Decode565To8888( g_lib3DGL->s_decodeBuffer, textAlpha->m_pow2Width, textAlpha->m_pow2Height, 
								dataAlpha, textAlpha->SizeX(), new_size_y,
								true, NULL);

	
	g_lib3DGL->CreateGLTexture(textAlpha->m_glTextureName, 
								textAlpha->m_pow2Width, textAlpha->m_pow2Height, 
								g_lib3DGL->s_decodeBuffer, 
								GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true);
	
	//set filter
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//cdp create the key( TexUniqueId) for this name
	TTexture::CreateKeyForGLTexName( textAlpha->m_nTexUniqueIdForGLTexName, textAlpha->m_glTextureName );

	//set hasAlphaChannel
	textAlpha->m_bHasAlphaChannel = true;

	#ifdef DEBUG_TEX_MEM
		//textAlpha->m_glTexPixelType = GL_UNSIGNED_BYTE;
		textAlpha->m_videoMemSize = textAlpha->m_pow2Width * textAlpha->m_pow2Height * 4;
	#endif

	for (int i=0; i<textures_nb; i++)
	{
		textures[i]->CleanBuffers();
	}

	for (int i=0; i<128; i++)
	{
		delete[] fillMatrixAlpha[i];
		delete[] fillMatrixOpaque[i];
	}
	delete[] fillMatrixAlpha;
	delete[] fillMatrixOpaque;

	textOpaque->CleanBuffers();
	textAlpha->CleanBuffers();
}

#endif /* USE_OGL */
