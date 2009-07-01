#include "softline.h"
#include "lib2d/lib2d.h"
#include "util.h"
#include "Lib2D/Image.h"
#include "Lib2D/aaline.h"
#include "Performance.h"

#if WIN_DEBUG
	static CLib3D* _gLib3d;
#endif

#ifdef __565_RENDERING__
	#define __BREW__
#endif


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSoftLine3D::DrawScanline(CLib3D& lib3d, unsigned short * const line, int const xLeft, int const xRight, int const sLeft, int const sRight, int const * const texture)
{
	// clip on X
	int const x0 = MATH_MAX(xLeft >> 16, 0);
	int const x1 = MATH_MIN(xRight >> 16, lib3d.Width());
	if (x0 >= x1)
	{
		return;
	}

	int const denom = ((xRight - xLeft) >> 8);
	if (denom == 0)
	{
		return;
	}
	int const ds = ((sRight - sLeft) << 8) / denom;	// [FIXME] use division luts for this
	int s = sLeft + (x0 - (xLeft >> 16)) * ds;

	unsigned short * const endPixel = line + x1;
	for (unsigned short * currentPixel = line + x0; currentPixel < endPixel; ++ currentPixel, s += ds)
	{
		#if WIN_DEBUG
			const int _pn = currentPixel - _gLib3d->m_board3d->GetImageBuffer();
			A_ASSERT(_pn >=0 && _pn < (_gLib3d->Width()*_gLib3d->Height()));
		#endif
		*currentPixel = CLib2D::FastColorAdd(*currentPixel, texture[s >> 16]);
	}
}


#ifdef __BREW__

	#define WHITE_TEXTURE_ARRAY {\
		RGB444TO565(0x001), RGB444TO565(0x002), RGB444TO565(0x004), RGB444TO565(0x008),\
		RGB444TO565(0x00f), RGB444TO565(0x22f), RGB444TO565(0x44f), RGB444TO565(0x88f),\
		RGB444TO565(0x88f), RGB444TO565(0x44f), RGB444TO565(0x22f), RGB444TO565(0x00f),\
		RGB444TO565(0x008), RGB444TO565(0x004), RGB444TO565(0x002), RGB444TO565(0x001)},

	#define RED_TEXTURE_ARRAY {\
		RGB444TO565(0x100), RGB444TO565(0x200), RGB444TO565(0x400), RGB444TO565(0x800),\
		RGB444TO565(0xf00), RGB444TO565(0xf22), RGB444TO565(0xf44), RGB444TO565(0xf88),\
		RGB444TO565(0xf88), RGB444TO565(0xf44), RGB444TO565(0xf22), RGB444TO565(0xf00),\
		RGB444TO565(0x800), RGB444TO565(0x400), RGB444TO565(0x200), RGB444TO565(0x100)},

	#define PURPLE_TEXTURE_ARRAY {\
		RGB444TO565(0x101), RGB444TO565(0x101), RGB444TO565(0x202), RGB444TO565(0x404),\
		RGB444TO565(0x808), RGB444TO565(0x929), RGB444TO565(0xa4a), RGB444TO565(0xc8c),\
		RGB444TO565(0xc8c), RGB444TO565(0xa4a), RGB444TO565(0x929), RGB444TO565(0x808),\
		RGB444TO565(0x404), RGB444TO565(0x202), RGB444TO565(0x101), RGB444TO565(0x101)},

	#define BLUE_TEXTURE_ARRAY {\
		RGB444TO565(0x001), RGB444TO565(0x002), RGB444TO565(0x004), RGB444TO565(0x008),\
		RGB444TO565(0x00f), RGB444TO565(0x22f), RGB444TO565(0x44f), RGB444TO565(0x88f),\
		RGB444TO565(0x88f), RGB444TO565(0x44f), RGB444TO565(0x22f), RGB444TO565(0x00f),\
		RGB444TO565(0x008), RGB444TO565(0x004), RGB444TO565(0x002), RGB444TO565(0x001)},

#else

	// helper color texture macros
	#define WHITE_TEXTURE_ARRAY {\
		0x001, 0x002, 0x004, 0x008,\
		0x00f, 0x22f, 0x44f, 0x88f,\
		0x88f, 0x44f, 0x22f, 0x00f,\
		0x008, 0x004, 0x002, 0x001},

	#define RED_TEXTURE_ARRAY {\
		0x100, 0x200, 0x400, 0x800,\
		0xf00, 0xf22, 0xf44, 0xf88,\
		0xf88, 0xf44, 0xf22, 0xf00,\
		0x800, 0x400, 0x200, 0x100},

	#define PURPLE_TEXTURE_ARRAY {\
		0x101, 0x101, 0x202, 0x404,\
		0x808, 0x929, 0xa4a, 0xc8c,\
		0xc8c, 0xa4a, 0x929, 0x808,\
		0x404, 0x202, 0x101, 0x101},

	#define BLUE_TEXTURE_ARRAY {\
		0x001, 0x002, 0x004, 0x008,\
		0x00f, 0x22f, 0x44f, 0x88f,\
		0x88f, 0x44f, 0x22f, 0x00f,\
		0x008, 0x004, 0x002, 0x001},

#endif


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSoftLine3D::Draw(CLib3D& lib3d)
{
	PERF_COUNTER(CSoftLine3D_Draw);

	// 1D texture maps
	int const textureLevels = 3;	// [FIXME] magic number
	int const texture[textureLevels][16] =
	{
		BLUE_TEXTURE_ARRAY
		PURPLE_TEXTURE_ARRAY
		RED_TEXTURE_ARRAY
	};

	int const textureLevel = Clamped((m_Severity12 * textureLevels) >> 12, 0, textureLevels - 1);

	// construct a quad
	Vector4s	vertices[4];
	int			width[4];

	vertices[0] = m_startPosition;	
	vertices[1] = m_startPosition;	
	vertices[2] = m_endPosition;	
	vertices[3] = m_endPosition;

	width[0] = 0;					// 1-D texture coordinate
	width[1] = 0xf;				// 1-D texture coordinate
	width[2] = 0;					// 1-D texture coordinate
	width[3] = 0xf;				// 1-D texture coordinate

	if (m_drawEndsParallelToGround)
	{
		vertices[0].x -= m_startWidth;
		vertices[1].x += m_startWidth;
		vertices[2].x -= m_endWidth;
		vertices[3].x += m_endWidth;
	}
	else
	{
		// [FIXME] implement the end triangles if we use them for the sparks
		/*
		// [FIXME] base it on perpendicular, and also do y!
		vertices[0].x -= 20;
		vertices[0].y -= 20;
		vertices[1].x += 20;
		vertices[1].y += 20;
		vertices[2].x -= 20;
		vertices[2].y -= 20;
		vertices[3].x += 20;
		vertices[3].y += 20;
		*/
	}

	// sort vertices by Y using a stupid sort
	for (int i = 0; i < 3; ++ i)
	{
		for (int j = i + 1; j < 4; ++ j)
		{
			if (vertices[i].y > vertices[j].y)
			{
				Util::Swap(vertices[i], vertices[j]);
			}
		}
	}

	/*
	// do the upper triangle
	int xLeft = vertices[0].x << 16;
	int sLeft = vertices[0].s << 16;
	int xRight = xLeft;
	int sRight = sLeft;
	int y = vertices[0].y;				// [FIXME] will come down from upper triangle :P
	bool const isUpperTriangleOnLeft = vertices[2].x < vertices[1].x;
	// [FIXME]
	*/

	// do the middle trapezoid
	int xLeft = vertices[0].x << 16;	// [FIXME] will come down from upper triangle :P
	int sLeft = width[0] << 16;	// [FIXME] will come down from upper triangle :P
	int xRight = vertices[1].x << 16;	// [FIXME] will come down from upper triangle :P
	int sRight = width[1] << 16;	// [FIXME] will come down from upper triangle :P
	int y = vertices[0].y;				// [FIXME] will come down from upper triangle :P

	// [FIXME] use division luts for these:
	int const denomLeft = vertices[2].y - vertices[0].y;
	int const denomRight = vertices[3].y - vertices[1].y;
	int const dxLeft = denomLeft ? ((vertices[2].x - vertices[0].x) << 16) / denomLeft : 0;		// [FIXME] will come down from upper triangle :P
	int const dsLeft = denomLeft ? ((width[2] - width[0]) << 16) / denomLeft : 0;		// [FIXME] will come down from upper triangle :P
	int const dxRight = denomRight ? ((vertices[3].x - vertices[1].x) << 16) / denomRight : 0;	// [FIXME] will come down from upper triangle :P
	int const dsRight = denomRight ? ((width[3] - width[1]) << 16) / denomRight : 0;	// [FIXME] will come down from upper triangle :P

	unsigned short * const pixels = lib3d.m_board3d->GetImageBuffer();

	#if WIN_DEBUG
		_gLib3d = &lib3d;
	#endif


	int const y1 = MATH_MIN(vertices[2].y, lib3d.Height());	// [FIXME]
	unsigned short * const lineEnd = pixels + (y1 * lib3d.Width());
	for (unsigned short * line = pixels + (y * lib3d.Width()); line < lineEnd; line += lib3d.Width(),
		xLeft += dxLeft, sLeft += dsLeft, xRight += dxRight, sRight += dsRight)
	{
		if (line < pixels)
		{
			// clip Y [FIXME] there are faster ways!
			continue;
		}
		
		DrawScanline(lib3d, line, xLeft, xRight, sLeft, sRight, texture[textureLevel]);
	}


	// [FIXME] do the lower triangle
}
#ifdef USE_OGL
void CSoftLine3D::AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor)
{
	nVtxAdded = 0;
}
#endif


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CFakeZAALine3D::Draw(CLib3D& lib3d)
{
	PERF_COUNTER(CFakeZAALine3D_Draw);

	//Image buffer(lib3d.m_dispX, lib3d.m_dispY, lib3d.GetImageBufferFull());
	DrawAALine(	*lib3d.m_screenImage3D, // buffer,
				m_startPosition,
				m_endPosition,
				Vector2s(0,0),
				Vector2s(lib3d.m_dispX, lib3d.m_dispY-14),
				m_color,
				ADDITIVE);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
/*void CFakeZAALine3DGradient::Draw(CLib3D& lib3d)
{
	PERF_COUNTER(CFakeZAALine3DGradient_Draw);

	Image buffer(m_dispX, m_dispY, lib3d.GetImageBufferFull());
	DrawAALineGradient(	buffer,
						m_startPosition,
						m_endPosition,
						Vector2s(0,0),
						Vector2s(m_dispX, m_dispY-14),
						m_Gradient + m_ColorStart,
						m_ColorEnd - m_ColorStart,
						ADDITIVE);
}*/
void CFakeZAALine3DGradient::Draw(CLib3D& lib3d)
{
	PERF_COUNTER(CFakeZAALine3DGradient_Draw);

	//Image buffer(lib3d.m_dispX, lib3d.m_dispY, lib3d.GetImageBufferFull());

	int	tot = (m_ColorStart + (m_ColorEnd - m_ColorStart));
	if (tot >= 63)
	{
		if (m_ColorStart > 62)
			m_ColorStart = 62;
		DrawAALineGradient(	*lib3d.m_screenImage3D, // buffer,
							m_startPosition,
							m_endPosition,
							Vector2s(0,0),
							Vector2s(lib3d.m_dispX, lib3d.m_dispY-14),
							m_Gradient + m_ColorStart,
							m_ColorEnd - m_ColorStart - (tot - 62),
							ADDITIVE);
	}
	else
	{
		DrawAALineGradient(	*lib3d.m_screenImage3D, // buffer,
							m_startPosition,
							m_endPosition,
							Vector2s(0,0),
							Vector2s(lib3d.m_dispX, lib3d.m_dispY-14),
							m_Gradient + m_ColorStart,
							m_ColorEnd - m_ColorStart,
							ADDITIVE);

	}
}

#ifdef USE_OGL
void CFakeZAALine3DGradient::AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor)
{
	nVtxAdded = 0;
}
#endif
