#pragma warning(disable:4786)

#include "HG/HighGear.h"
#include "Board3D.h"
#include "Face.h"
#include "Color.h"
#include "Texture.h"
#include "Constants.h"
#include "Performance.h"
#include "IMath.h"
#include "FSqrt.h"
#include "lib2d/Lib2d.h"

#include <string.h>

#include "rasterize.h"


using namespace Lib3D;


#define CLIPYEND m_dispY
#define CLIPXEND m_dispX				// define right/top border
#define CLIPXMIN 0            // define bottom/left border

// rasterizing quality
// for best quality: SCREEN_SNAP = 1, ZC_MINZ = 1500, ZC_ERRMAX = 0
// for best speed: SCREEN_SNAP = 0, ZC_MINZ = 400, ZC_ERRMAX = 1500
#define SCREEN_SNAP 0                          // snap to screen pixel (more slow, but better quality)
//#define ZC_MINZ 500                            // min distance to allow rendering face without ZC
//#define ZC_ERRMAX 100                          // max allowed error before to apply perspective correction

//#define ZC_MINZ     0                            // min distance to allow rendering face without ZC
//#define ZC_ERRMAX 300                          // max allowed error before to apply perspective correction


#define ZC_MINZ 500                            // min distance to allow rendering face without ZC
#define ZC_ERRMAX 100                          // max allowed error before to apply perspective correction

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
inline int LastLine(const TVxuv *C, int clipYend)
{
	const int y = C->Vx->y;
	
	int yEnd;
	if (y >= clipYend)
		yEnd = clipYend;
	else
		yEnd = y;
	return yEnd;
}

// ---------------------------------------------------------
//	Ctor
// ---------------------------------------------------------
CBoard3D::CBoard3D()
	:m_wireframe(0x00),
	m_pHG(CHighGear::GetInstance()),
	m_dispX(m_pHG->m_dispX),
	m_dispY(m_pHG->m_dispY)
{
	//	Allocate 8k more than necessary so if (when) someone write 
	//	past the end of the buffer, we will survive!
	//
	//	The first one who criticize this patch will get the job of identifiing ALL 
	//	buffer overwrite in the code.
	const int kSecurityBufferSize = 1 * 1024;		// was 8
	m_roadFxMax = 0; 

// use lib2d screen buffer
/*
	m_screenFullReal = NEW TPixel[m_dispX*m_dispY + kSecurityBufferSize*2];
	m_screenFull = m_screenFullReal + kSecurityBufferSize;
	recalcScreenPtr();
	::memset(m_screenFull,0,m_dispX*m_dispY * sizeof(*m_screenFull));
*/

#if USE_STENCIL_BUFFER
	m_stencilBuffer = 0;//LA:Lib2D's buffer is used for this NEW TPixel[m_dispX*m_dispY];
	m_stencilAlpha = NEW unsigned char[m_dispX*m_dispY];

	//ClearStencil(true);
		m_stencilBeginX = 0;
		m_stencilBeginY = 0;
		m_stencilEndX	= m_dispX - 1;
		m_stencilEndY	= m_dispY - 1;
#endif // USE_STENCIL_BUFFER
	InitInverseTable();
	
	m_TextureFXIdx = kTextureFxModeNormal;

	// init table
	m_256Div16[0] = 0;
	for (int i = 1; i < 16; i++)
		m_256Div16[i] = (1<<8) / i;

#if USE_Z_BUFFER
	m_zBufferReal = NEW int[m_dispX*m_dispY + kSecurityBufferSize*2];
	m_zBuffer = m_zBufferReal + kSecurityBufferSize;
//	m_zBufferDir = -1;
	ClearZBuffer();	
#endif
}


// ---------------------------------------------------------
//
// ---------------------------------------------------------
CBoard3D::~CBoard3D()
{
#if USE_STENCIL_BUFFER
 	DELETE_ARRAY m_stencilAlpha;
#endif

//	DELETE_ARRAY m_screenFullReal;

#if USE_Z_BUFFER
	DELETE_ARRAY m_zBufferReal;
#endif // USE_Z_BUFFER
}


//void CBoard3D::recalcScreenPtr()
//{
//#ifdef TOP_LETTERBOX_BAR_ONLY
//	m_screen = m_screenFull + m_dispX * kInterfaceHeigth;
//#else
//	m_screen = m_screenFull + m_dispX * kInterfaceHeigth / 2;
//#endif
//}

// ---------------------------------------------------------
//                   Init division table
// ---------------------------------------------------------
void CBoard3D::InitInverseTable(void)
{
#if USE_TABLE
	int i;
	
	const float Numerator = float(1<<INVTBL_SHIFT);
	
	for (i=1; i<DX_RANGE; i++)
	{    
		float B = Numerator/ float(i); // + 0.5f;
		CHECK_LIMIT(B);
		m_inverseTable[i] = (int)B;
	}
	m_inverseTable[0] = m_inverseTable[1];           // division by 0!
	
	
	const int num = 1 << 22;
	for (i=1; i<(DX_RANGE + 32); i++)
	{
		CHECK_LIMIT(double(num)/double(i));
		
		int Rh = num/i;
		
		//    if (Rh >= 65536)                           // mean Z <= NEAR_CLIP
		//      Rh = 65535;
		
		m_invRhTable[i] = Rh;
	}
#endif//#if USE_TABLE
}




// ---------------------------------------------------------
// (xa <= xb) && (dx > dy) required
// ---------------------------------------------------------
void CBoard3D::DrawEdgeH(int xStart, int ya, int xStop, int yb)
{
	if ((xStart >= m_dispX) || (xStop < 0))
		return;
	
	if (xStart >= xStop)
		return;
	
	// use dx as interpolation counter
	int yStart  = ya << DIV_SHIFT;
	
	int Invd = InverseTable((xStop - xStart) & DIVTBL_MASK);
	int dYStart  = ((yb - ya)*Invd) >> (INVTBL_SHIFT - 16);
	
	if (xStart < 0)
	{
		yStart -= xStart*dYStart;
		xStart  = 0;
	}
	
	if (xStop > m_dispX)
		xStop = m_dispX;
	
	while (xStart < xStop)
	{
		int yDraw = yStart >> DIV_SHIFT;
		if ((yDraw >= 0) && (yDraw < m_dispY))
			m_screen[yDraw*m_dispX + xStart] = m_wireframe;
		
		yStart  += dYStart;
		xStart++;
	}
}

// ---------------------------------------------------------
// (ya <= yb) && (dy > dx) required
// ---------------------------------------------------------
void CBoard3D::DrawEdgeV(int xa, int yStart, int xb, int yStop)
{
	if ((yStart >= m_dispY) || (yStop < 0))
		return;
	
	if (yStart >= yStop)
		return;
	
	// use dx as interpolation counter
	int xStart  = xa << DIV_SHIFT;
	
	int Invd = InverseTable((yStop - yStart) & DIVTBL_MASK);
	int dXStart  = ((xb - xa)*Invd) >> (INVTBL_SHIFT - 16); 
	
	if (yStart < 0)
	{
		xStart -= yStart*dXStart;
		yStart  = 0;
	}
	
	if (yStop > m_dispY)
		yStop = m_dispY;
	
	while (yStart < yStop)
	{
		int xDraw = xStart >> DIV_SHIFT;
		if ((xDraw >= 0) && (xDraw < m_dispX))
			m_screen[yStart*m_dispX + xDraw] = m_wireframe;
		
		xStart  += dXStart;
		yStart++;
	}
}

// ---------------------------------------------------------
//
// ---------------------------------------------------------
void CBoard3D::DrawEdgeHV(int xa, int ya, int xb, int yb)
{
	int dx = xb - xa;
	int dy = yb - ya;
	
	if (dx >= 0)
	{
		if (dy >= 0)
		{
			if (dx >= dy)
				DrawEdgeH(xa, ya, xb, yb);
			else
				DrawEdgeV(xa, ya, xb, yb);
		}
		else
		{
			if (dx >= -dy)
				DrawEdgeH(xa, ya, xb, yb);
			else
				DrawEdgeV(xb, yb, xa, ya);
		}
	}
	else
	{
		if (dy >= 0)
		{
			if (-dx >= dy)
				DrawEdgeH(xb, yb, xa, ya);
			else
				DrawEdgeV(xa, ya, xb, yb);
		}
		else
		{
			if (-dx >= -dy)
				DrawEdgeH(xb, yb, xa, ya);
			else
				DrawEdgeV(xb, yb, xa, ya);
		}
	}
}

// ---------------------------------------------------------
//
// ---------------------------------------------------------
void CBoard3D::DrawPolyEdge(const Vector4s *VxA, const Vector4s *VxB, const Vector4s *VxC)
{
	DrawEdgeHV(VxA->x, VxA->y, VxB->x, VxB->y);
	DrawEdgeHV(VxB->x, VxB->y, VxC->x, VxC->y);
	DrawEdgeHV(VxC->x, VxC->y, VxA->x, VxA->y);
}


/*
	"Advanced Rasterization" technique by Nicolas Capens

This is quite easy. First determine gradients du/dx, dv/dx, du/dy and dv/dy. 

Then for every pixel you can determine how far it is from the first vertex: dx = x - x0, dy = y - y0 (note that x0 and y0 are floating-point, x and y are integer). 
Then the texture coordinates are simply: u = u0 + du/dx * dx + du/dy * dy.

Once you got the coordinates for one pixel you obviously only have to add du/dx and/or du/dy to get the texture coordinates for the neighboring pixels. 
So I do the above calculation once per block, then use this linear interpolation for the other pixels in the block.


The easiest and most robust way is to use the plane equation.

Let's say we want to interpolate some component z linearly across the polygon (note that z stands for any interpolant). We can visualize this as a plane going through the x, y and z positions of the triangle, in 3D space. Now, the equation of a 3D plane is generally:

A * x + B * y + C * z + D = 0

From this we can derive that:

z = -A / C * x - B / C * y - D

Note that for every step in the x-direction, z increments by -A / C, and likewise it increments by -B / C for every step in the y-direction. So these are the gradients we're looking for to perform linear interpolation. In the plane equation (A, B, C) is the normal vector of the plane. It can easily be computed with a cross product.

Now that we have the gradients, let's call them dz/dx (which is -A / C) and dz/dy (which is -B / C), we can easily compute z everywhere on the triangle. We know the z value in all three vertex positions. Let's call the one of the first vertex z0, and it's position coordinates (x0, y0). Then a generic z value of a point (x, y) can be computed as:

z = z0 + dz/dx * (x - x0) + dz/dy * (y - y0)

Once you've computed the z value for the center of the starting pixel this way, you can easily add dz/dx to get the z value for the next pixel, or dz/dy for the pixel below (with the y-axis going down).

A = (z3 - z1) * (y2 - y1) - (z2 - z1) * (y3 - y1)
B = (x3 - x1) * (z2 - z1) - (x2 - x1) * (z3 - z1)
C = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)

*/

   // Block size, standard is 8x8 (must be power of two)
#define BLOCK_SIZE_SHIFT	2	// 2 = 4x4 / 3 = 8x8
#define BLOCK_SIZE			(1<<BLOCK_SIZE_SHIFT)

// not used
//void test_triangle_nokeycolor( const TFace* F, const TVxuv* vxUV[],  unsigned short* colorBuffer, CHighGear* pHG )
//{
//	const int Y1 = vxUV[0]->Vx->y;
//    const int Y2 = vxUV[1]->Vx->y;
//    const int Y3 = vxUV[2]->Vx->y;
//
//    const int X1 = vxUV[0]->Vx->x;
//    const int X2 = vxUV[1]->Vx->x;
//    const int X3 = vxUV[2]->Vx->x;
//
//    // Deltas
//    const int DX12 = X1 - X2;
//    const int DY12 = Y1 - Y2;
//	if ((DX12 | DY12) == 0)
//		return;
//
//	const int DX23 = X2 - X3;
//    const int DY23 = Y2 - Y3;
//	if ((DX23 | DY23) == 0)
//		return;
//
//	const int DX31 = X3 - X1;
//    const int DY31 = Y3 - Y1;
//	if ((DX31 | DY31) == 0)
//		return;
//
//	const Lib3D::TTexture*	texture = F->GetTexture();
//
//	// gradients
//#define GRAD_DIV_BITS		12//12
//	const int	C = (X2 - X1) * (Y3 - Y1) - (X3 - X1) * (Y2 - Y1);
//	if (C == 0)
//		return;
//
//	const int	U1 = vxUV[0]->u;
//	const int	U2 = vxUV[1]->u;
//	const int	U3 = vxUV[2]->u;
//	const int	V1 = vxUV[0]->v;
//	const int	V2 = vxUV[1]->v;
//	const int	V3 = vxUV[2]->v;
//
//	const int	Au = (U3 - U1) * (Y2 - Y1) - (U2 - U1) * (Y3 - Y1);
//	const int	Bu = (X3 - X1) * (U2 - U1) - (X2 - X1) * (U3 - U1);
//	const int	Av = (V3 - V1) * (Y2 - Y1) - (V2 - V1) * (Y3 - Y1);
//	const int	Bv = (X3 - X1) * (V2 - V1) - (X2 - X1) * (V3 - V1);
//
//	A_ASSERT( Abs(Au) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Bu) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Av) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Bv) < (1<<(31-GRAD_DIV_BITS)) );
//
//	int	dudx16 = -(Au<<GRAD_DIV_BITS) / C;	// TBD optimize this (precomputed div is not precise enough)
//	int	dudy16 = -(Bu<<GRAD_DIV_BITS) / C;
//	int	dvdx16 = -(Av<<GRAD_DIV_BITS) / C;
//	int	dvdy16 = -(Bv<<GRAD_DIV_BITS) / C;
//
//	// texture data
//	unsigned short*	t_data = texture->Data();
//	const int		t_mask = texture->DrawMask();
//	const int		t_shift = texture->VShift();
//
//    // Bounding rectangle
//    int minx = min(X1, min(X2, X3));
//    int maxx = max(X1, max(X2, X3));
//    int miny = min(Y1, min(Y2, Y3));
//    int maxy = max(Y1, max(Y2, Y3));
//
//
//    // Start in corner of qxq block
//    minx &= ~(BLOCK_SIZE - 1);
//    miny &= ~(BLOCK_SIZE - 1);
//
//    colorBuffer += miny * pHG->m_dispX;
//
//    // Half-edge constants
//    int C1 = DY12 * X1 - DX12 * Y1;
//    int C2 = DY23 * X2 - DX23 * Y2;
//    int C3 = DY31 * X3 - DX31 * Y3;
//
//    // Correct for fill convention
//    if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
//    if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
//    if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;
//	
//	int	u = (U1 << GRAD_DIV_BITS) + (dudx16 * (minx - X1)) + (dudy16 * (miny - Y1));
//	int	v = (V1 << GRAD_DIV_BITS) + (dvdx16 * (minx - X1)) + (dvdy16 * (miny - Y1));
//
//    // Loop through blocks
//    for (int y = miny; y < maxy; y += BLOCK_SIZE)
//    {
//		// corners of block
//		const int	y0 = y;
//		const int	y1 = (y + BLOCK_SIZE - 1);
//
//		// Evaluate half-space functions
//		const int	c1dx12y0 = C1 + DX12 * y0;
//		const int	c1dx12y1 = C1 + DX12 * y1;
//		const int	c2dx23y0 = C2 + DX23 * y0;
//		const int	c2dx23y1 = C2 + DX23 * y1;
//		const int	c3dx31y0 = C3 + DX31 * y0;
//		const int	c3dx31y1 = C3 + DX31 * y1;
//
//		int		nblockx = 0;
//        for (int x = minx; x < maxx; x += BLOCK_SIZE)
//        {
//			nblockx++;
//
//			// Corners of block
//			const int	x0 = x;
//			const int	x1 = (x + BLOCK_SIZE - 1);
//
//			// Evaluate half-space functions
//			const int	dy12x0 = DY12 * x0;
//			const int	dy12x1 = DY12 * x1;
//			const bool	a00 = (c1dx12y0 - dy12x0) > 0;
//			const bool	a10 = (c1dx12y0 - dy12x1) > 0;
//			const bool	a01 = (c1dx12y1 - dy12x0) > 0;
//			const bool	a11 = (c1dx12y1 - dy12x1) > 0;
//			const int	a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
//
//			const int	dy23x0 = DY23 * x0;
//			const int	dy23x1 = DY23 * x1;
//			const bool	b00 = (c2dx23y0 - dy23x0) > 0;
//			const bool	b10 = (c2dx23y0 - dy23x1) > 0;
//			const bool	b01 = (c2dx23y1 - dy23x0) > 0;
//			const bool	b11 = (c2dx23y1 - dy23x1) > 0;
//			const int	b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
//	
//			const int	dy31x0 = DY31 * x0;
//			const int	dy31x1 = DY31 * x1;
//			const bool	c00 = (c3dx31y0 - dy31x0) > 0;
//			const bool	c10 = (c3dx31y0 - dy31x1) > 0;
//			const bool	c01 = (c3dx31y1 - dy31x0) > 0;
//			const bool	c11 = (c3dx31y1 - dy31x1) > 0;
//			const int	c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);
//
//			// Skip block when outside an edge
//			if ((a == 0x0) || (b == 0x0) || (c == 0x0)) 
//			{
//				// set uv to next block on the right
//				u += (dudx16 << BLOCK_SIZE_SHIFT);
//				v += (dvdx16 << BLOCK_SIZE_SHIFT);
//
//				continue;
//			}
//
//
//            // Accept whole block when totally covered
//            if ((a == 0xF) && (b == 0xF) && (c == 0xF))
//            {
////				int		u = (U1 << GRAD_DIV_BITS) + (dudx16 * (x - X1)) + (dudy16 * (y - Y1));	// TBD optimize this !
////				int		v = (V1 << GRAD_DIV_BITS) + (dvdx16 * (x - X1)) + (dvdy16 * (y - Y1));	// optimize this !
//
//	            unsigned long *buffer = (unsigned long*)(colorBuffer + x);
//
//				int	n = BLOCK_SIZE;
//                while (n--)
//                {
//					register unsigned long	col;
//
//#define DRAW2PIXELS		col = t_data[((u >> (TTexture::TEX_UV_SHIFT + GRAD_DIV_BITS)) & t_mask) | (((v >> (TTexture::TEX_UV_SHIFT + GRAD_DIV_BITS)) & t_mask) << t_shift)];	\
//						u += dudx16;	\
//						v += dvdx16;	\
//						col |= t_data[((u >> (TTexture::TEX_UV_SHIFT + GRAD_DIV_BITS)) & t_mask) | (((v >> (TTexture::TEX_UV_SHIFT + GRAD_DIV_BITS)) & t_mask) << t_shift)] << 16;	\
//						u += dudx16;	\
//						v += dvdx16;	\
//						*buffer++ = col;
//
//#if (BLOCK_SIZE == 2)
//					DRAW2PIXELS
//#elif (BLOCK_SIZE == 4)
//					DRAW2PIXELS
//					DRAW2PIXELS
//#elif (BLOCK_SIZE == 8)
//					DRAW2PIXELS
//					DRAW2PIXELS
//					DRAW2PIXELS
//					DRAW2PIXELS
//#elif
//					case not handled !
//#endif
//
//                    buffer += (pHG->m_dispX - BLOCK_SIZE) >> 1;
//
//					// set uv to next line
//					u += dudy16 - (dudx16 << BLOCK_SIZE_SHIFT);
//					v += dvdy16 - (dvdx16 << BLOCK_SIZE_SHIFT);
//                }
//            }
//            else   // Partially covered block
//            {
//				unsigned short *buffer = colorBuffer + x;
//
//                int		CY1 = C1 + DX12 * y0 - DY12 * x0;
//                int		CY2 = C2 + DX23 * y0 - DY23 * x0;
//                int		CY3 = C3 + DX31 * y0 - DY31 * x0;
//
////				int		u = (U1 << GRAD_DIV_BITS) + (dudx16 * (x - X1)) + (dudy16 * (y - Y1));	// TBD optimize this !
////				int		v = (V1 << GRAD_DIV_BITS) + (dvdx16 * (x - X1)) + (dvdy16 * (y - Y1));	// optimize this !
//
//				int		n = BLOCK_SIZE;
//                while (n--)
//                {
//                    int		CX1 = CY1;
//                    int		CX2 = CY2;
//                    int		CX3 = CY3;
//
//#define	DRAWONEPIXEL	if ((CX1 > 0) && (CX2 > 0) && (CX3 > 0))	\
//							*buffer = t_data[((u >> (TTexture::TEX_UV_SHIFT + GRAD_DIV_BITS)) & t_mask) | (((v >> (TTexture::TEX_UV_SHIFT + GRAD_DIV_BITS)) & t_mask) << t_shift)];	\
//						buffer++;		\
//                        CX1 -= DY12;	\
//                        CX2 -= DY23;	\
//                        CX3 -= DY31;	\
//						u += dudx16;	\
//						v += dvdx16;	
//
//#if (BLOCK_SIZE == 2)
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//#elif (BLOCK_SIZE == 4)
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//#elif (BLOCK_SIZE == 8)
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//					DRAWONEPIXEL
//#elif
//					case not handled !
//#endif
//
//                    CY1 += DX12;
//                    CY2 += DX23;
//                    CY3 += DX31;
//
//                    buffer += (pHG->m_dispX - BLOCK_SIZE);
//
//					// set uv to next line
//					u += dudy16 - (dudx16 << BLOCK_SIZE_SHIFT);
//					v += dvdy16 - (dvdx16 << BLOCK_SIZE_SHIFT);
//                }
//            }
//
//			// set uv to start of next block on the right
//			u += ((dudx16 - dudy16) << BLOCK_SIZE_SHIFT);
//			v += ((dvdx16 - dvdy16) << BLOCK_SIZE_SHIFT);
//        }
//
//		// set uv to start of first block next block line
//		u += ((dudy16 - dudx16*nblockx) << BLOCK_SIZE_SHIFT);
//		v += ((dvdy16 - dvdx16*nblockx) << BLOCK_SIZE_SHIFT);
//
//        colorBuffer += (BLOCK_SIZE * pHG->m_dispX);
//    }
//}

// not used
//void test_triangle_corrected( const TFace* F, const TVxuv* vxUV[],  unsigned short* colorBuffer, CHighGear *pHG )
//{
//	const int Y1 = vxUV[0]->Vx->y;
//    const int Y2 = vxUV[1]->Vx->y;
//    const int Y3 = vxUV[2]->Vx->y;
//
//    const int X1 = vxUV[0]->Vx->x;
//    const int X2 = vxUV[1]->Vx->x;
//    const int X3 = vxUV[2]->Vx->x;
//
//    // Deltas
//    const int DX12 = X1 - X2;
//    const int DY12 = Y1 - Y2;
//	if ((DX12 | DY12) == 0)
//		return;
//
//	const int DX23 = X2 - X3;
//    const int DY23 = Y2 - Y3;
//	if ((DX23 | DY23) == 0)
//		return;
//
//	const int DX31 = X3 - X1;
//    const int DY31 = Y3 - Y1;
//	if ((DX31 | DY31) == 0)
//		return;
//
//	const Lib3D::TTexture*	texture = F->GetTexture();
//
//	// gradients
//#define GRAD_DIV_BITS		12
//	const int	C = (X2 - X1) * (Y3 - Y1) - (X3 - X1) * (Y2 - Y1);
//	if (C == 0)
//		return;
//#define W_DIV_BITS			16//16
//#define UV_DIV_BITS			8//8
//#define UV_DIV_BITS_LEFT	(/*W_DIV_BITS -*/ UV_DIV_BITS)
//	const int	W1 = (1<<W_DIV_BITS) / vxUV[0]->Vx->z;
//	const int	W2 = (1<<W_DIV_BITS) / vxUV[1]->Vx->z;
//	const int	W3 = (1<<W_DIV_BITS) / vxUV[2]->Vx->z;
//	const int	U1 = (vxUV[0]->u * W1) >> UV_DIV_BITS_LEFT;
//	const int	U2 = (vxUV[1]->u * W2) >> UV_DIV_BITS_LEFT;
//	const int	U3 = (vxUV[2]->u * W3) >> UV_DIV_BITS_LEFT;
//	const int	V1 = (vxUV[0]->v * W1) >> UV_DIV_BITS_LEFT;
//	const int	V2 = (vxUV[1]->v * W2) >> UV_DIV_BITS_LEFT;
//	const int	V3 = (vxUV[2]->v * W3) >> UV_DIV_BITS_LEFT;
//
//	const int	Au = (U3 - U1) * (Y2 - Y1) - (U2 - U1) * (Y3 - Y1);
//	const int	Bu = (X3 - X1) * (U2 - U1) - (X2 - X1) * (U3 - U1);
//	const int	Av = (V3 - V1) * (Y2 - Y1) - (V2 - V1) * (Y3 - Y1);
//	const int	Bv = (X3 - X1) * (V2 - V1) - (X2 - X1) * (V3 - V1);
//	const int	Aw = (W3 - W1) * (Y2 - Y1) - (W2 - W1) * (Y3 - Y1);
//	const int	Bw = (X3 - X1) * (W2 - W1) - (X2 - X1) * (W3 - W1);
//
//	A_ASSERT( Abs(Au) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Bu) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Av) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Bv) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Aw) < (1<<(31-GRAD_DIV_BITS)) );
//	A_ASSERT( Abs(Bw) < (1<<(31-GRAD_DIV_BITS)) );
//
//	int	dudx16 = -(Au<<GRAD_DIV_BITS) / C;	// TBD optimize this (precomputed div is not precise enough)
//	int	dudy16 = -(Bu<<GRAD_DIV_BITS) / C;
//	int	dvdx16 = -(Av<<GRAD_DIV_BITS) / C;
//	int	dvdy16 = -(Bv<<GRAD_DIV_BITS) / C;
//	int	dwdx16 = -(Aw<<GRAD_DIV_BITS) / C;
//	int	dwdy16 = -(Bw<<GRAD_DIV_BITS) / C;
//
//	// texture data
//	unsigned short*	t_data = texture->Data();
//	int				t_mask = texture->DrawMask();
//	int				t_shift = texture->VShift();
//
//    // Bounding rectangle
//    int minx = min(X1, min(X2, X3));
//    int maxx = max(X1, max(X2, X3));
//    int miny = min(Y1, min(Y2, Y3));
//    int maxy = max(Y1, max(Y2, Y3));
//
//    // Start in corner of qxq block
//    minx &= ~(BLOCK_SIZE - 1);
//    miny &= ~(BLOCK_SIZE - 1);
//
//    colorBuffer += miny * pHG->m_dispX;
//
//    // Half-edge constants
//    int C1 = DY12 * X1 - DX12 * Y1;
//    int C2 = DY23 * X2 - DX23 * Y2;
//    int C3 = DY31 * X3 - DX31 * Y3;
//
//    // Correct for fill convention
//    if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
//    if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
//    if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;
//
//    // Loop through blocks
//    for(int y = miny; y < maxy; y += BLOCK_SIZE)
//    {
//        for(int x = minx; x < maxx; x += BLOCK_SIZE)
//        {
//			// Corners of block
//			const int x0 = x;
//			const int x1 = (x + BLOCK_SIZE - 1);
//			const int y0 = y;
//			const int y1 = (y + BLOCK_SIZE - 1);
//
//			// Evaluate half-space functions
//			const bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
//			const bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
//			const bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
//			const bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
//			const int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
//	
//			const bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
//			const bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
//			const bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
//			const bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
//			const int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
//	
//			const bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
//			const bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
//			const bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
//			const bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
//			const int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);
//
//			// Skip block when outside an edge
//			if (a == 0x0 || b == 0x0 || c == 0x0) continue;
//
//
//            unsigned short *buffer = colorBuffer;
//
//            // Accept whole block when totally covered
//            if(a == 0xF && b == 0xF && c == 0xF)
//            {
//				int		u = (U1 << GRAD_DIV_BITS) + (dudx16 * (x - X1)) + (dudy16 * (y - Y1));	// TBD optimize this !
//				int		v = (V1 << GRAD_DIV_BITS) + (dvdx16 * (x - X1)) + (dvdy16 * (y - Y1));	// optimize this !
//				int		w = (W1 << GRAD_DIV_BITS) + (dwdx16 * (x - X1)) + (dwdy16 * (y - Y1));	// optimize this !
//
//                for(int iy = 0; iy < BLOCK_SIZE; iy++)
//                {
//                    for(int ix = x; ix < x + BLOCK_SIZE; ix++)	// optimize this + 32 bits copy + block length 1/w linear interpolation
//                    {
//						const int				ru = ((u>>GRAD_DIV_BITS)<<UV_DIV_BITS) / (w>>GRAD_DIV_BITS);	// TBD optimize DIV
//						const int				rv = ((v>>GRAD_DIV_BITS)<<UV_DIV_BITS) / (w>>GRAD_DIV_BITS);
//						const int				idx = ((ru >> (TTexture::TEX_UV_SHIFT)) & t_mask) | (((rv >> (TTexture::TEX_UV_SHIFT)) & t_mask) << t_shift);
//						const unsigned short	col = t_data[idx];
//						
//						if (!(col & 0xF000))
//							buffer[ix] = col;
//
//						u += dudx16;
//						v += dvdx16;
//						w += dwdx16;
//                    }
//
//                    buffer += pHG->m_dispX;
//
//					u += dudy16 - BLOCK_SIZE*dudx16;
//					v += dvdy16 - BLOCK_SIZE*dvdx16;
//					w += dwdy16 - BLOCK_SIZE*dwdx16;
//                }
//            }
//            else   // Partially covered block
//            {
//                int		CY1 = C1 + DX12 * y0 - DY12 * x0;
//                int		CY2 = C2 + DX23 * y0 - DY23 * x0;
//                int		CY3 = C3 + DX31 * y0 - DY31 * x0;
//
//				int		u = (U1 << GRAD_DIV_BITS) + (dudx16 * (x - X1)) + (dudy16 * (y - Y1));	// TBD optimize this !
//				int		v = (V1 << GRAD_DIV_BITS) + (dvdx16 * (x - X1)) + (dvdy16 * (y - Y1));	// optimize this !
//				int		w = (W1 << GRAD_DIV_BITS) + (dwdx16 * (x - X1)) + (dwdy16 * (y - Y1));	// optimize this !
//
//                for(int iy = y; iy < y + BLOCK_SIZE; iy++)
//                {
//                    int		CX1 = CY1;
//                    int		CX2 = CY2;
//                    int		CX3 = CY3;
//
//					for(int ix = x; ix < x + BLOCK_SIZE; ix++)
//                    {
//                        if(CX1 > 0 && CX2 > 0 && CX3 > 0)
//                        {
//							const int				ru = ((u>>GRAD_DIV_BITS)<<UV_DIV_BITS) / (w>>GRAD_DIV_BITS);	// TBD optimize DIV
//							const int				rv = ((v>>GRAD_DIV_BITS)<<UV_DIV_BITS) / (w>>GRAD_DIV_BITS);
//							const int				idx = ((ru >> (TTexture::TEX_UV_SHIFT)) & t_mask) | (((rv >> (TTexture::TEX_UV_SHIFT)) & t_mask) << t_shift);
//							const unsigned short	col = t_data[idx];
//
//							if (!(col & 0xF000))
//								buffer[ix] = col;
//                        }
//
//                        CX1 -= DY12;
//                        CX2 -= DY23;
//                        CX3 -= DY31;
//
//						u += dudx16;
//						v += dvdx16;
//						w += dwdx16;
//                    }
//
//                    CY1 += DX12;
//                    CY2 += DX23;
//                    CY3 += DX31;
//
//                    buffer += pHG->m_dispX;
//
//					u += dudy16 - BLOCK_SIZE*dudx16;
//					v += dvdy16 - BLOCK_SIZE*dvdx16;
//					w += dwdy16 - BLOCK_SIZE*dwdx16;
//                }
//            }
//        }
//
//        colorBuffer += BLOCK_SIZE * pHG->m_dispX;
//    }
//}

#if 0
void test_triangle_flat( const TFace* F, const TVxuv* vxUV[],  unsigned short* colorBuffer )
{

	const unsigned short	color16 = (unsigned short)F;
	const unsigned long		color32 = color16 | (color16<<16);


	const int Y1 = vxUV[0]->Vx->y;
    const int Y2 = vxUV[1]->Vx->y;
    const int Y3 = vxUV[2]->Vx->y;

    const int X1 = vxUV[0]->Vx->x;
    const int X2 = vxUV[1]->Vx->x;
    const int X3 = vxUV[2]->Vx->x;

    // Deltas
    const int DX12 = X1 - X2;
    const int DY12 = Y1 - Y2;
	if ((DX12 | DY12) == 0)
		return;

	const int DX23 = X2 - X3;
    const int DY23 = Y2 - Y3;
	if ((DX23 | DY23) == 0)
		return;

	const int DX31 = X3 - X1;
    const int DY31 = Y3 - Y1;
	if ((DX31 | DY31) == 0)
		return;

    // Bounding rectangle
    int minx = min(X1, min(X2, X3));
    int maxx = max(X1, max(X2, X3));
    int miny = min(Y1, min(Y2, Y3));
    int maxy = max(Y1, max(Y2, Y3));

    // Start in corner of qxq block
    minx &= ~(BLOCK_SIZE - 1);
    miny &= ~(BLOCK_SIZE - 1);

    colorBuffer += miny * m_dispX;

    // Half-edge constants
    int C1 = DY12 * X1 - DX12 * Y1;
    int C2 = DY23 * X2 - DX23 * Y2;
    int C3 = DY31 * X3 - DX31 * Y3;

    // Correct for fill convention
    if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
    if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
    if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

    // Loop through blocks
    for(int y = miny; y < maxy; y += BLOCK_SIZE)
    {
        for(int x = minx; x < maxx; x += BLOCK_SIZE)
        {
			// Corners of block
			const int x0 = x;
			const int x1 = (x + BLOCK_SIZE - 1);
			const int y0 = y;
			const int y1 = (y + BLOCK_SIZE - 1);

			// Evaluate half-space functions
			const bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
			const bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
			const bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
			const bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
			const int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
	
			const bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
			const bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
			const bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
			const bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
			const int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
	
			const bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
			const bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
			const bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
			const bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
			const int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

			// Skip block when outside an edge
			if (a == 0x0 || b == 0x0 || c == 0x0) continue;


            // Accept whole block when totally covered
            if(a == 0xF && b == 0xF && c == 0xF)
            {
	            unsigned long *buffer = (unsigned long*)(colorBuffer + x);

                for(int iy = 0; iy < BLOCK_SIZE; iy++)
                {
#define	FILL_2PIXELS	*buffer++ = color32;

#if (BLOCK_SIZE == 2)
					FILL_2PIXELS
#elif (BLOCK_SIZE == 4)
					FILL_2PIXELS
					FILL_2PIXELS
#elif (BLOCK_SIZE == 8)
					FILL_2PIXELS
					FILL_2PIXELS
					FILL_2PIXELS
					FILL_2PIXELS
#elif
					case not handled !
#endif

                    buffer += m_dispX - BLOCK_SIZE;
                }
            }
            else   // Partially covered block
            {
	            unsigned short *buffer = colorBuffer + x;

                int		CY1 = C1 + DX12 * y0 - DY12 * x0;
                int		CY2 = C2 + DX23 * y0 - DY23 * x0;
                int		CY3 = C3 + DX31 * y0 - DY31 * x0;

                for(int iy = y; iy < y + BLOCK_SIZE; iy++)
                {
                    int		CX1 = CY1;
                    int		CX2 = CY2;
                    int		CX3 = CY3;

					unsigned short*	buffer_end = buffer + BLOCK_SIZE;
                    while (buffer < buffer_end)
                    {
                        if(CX1 > 0 && CX2 > 0 && CX3 > 0)
                        {
							*buffer = color16;
                        }

                        CX1 -= DY12;
                        CX2 -= DY23;
                        CX3 -= DY31;

						buffer++;
                    }

                    CY1 += DX12;
                    CY2 += DX23;
                    CY3 += DX31;

                    buffer += m_dispX - BLOCK_SIZE;
                }
            }
        }

        colorBuffer += BLOCK_SIZE * m_dispX;
    }
}
#endif //#if 0

bool PrepareFace2(const TFace* F,const TVxuv** vxUV)
{	
	const Vector4s&	VxA = F->ScrVectorA();
	const Vector4s&	VxB = F->ScrVectorB();
	const Vector4s&	VxC = F->ScrVectorC() ;
	
	if ((((VxA.x - VxB.x) * (VxA.y - VxC.y)) - ((VxA.y - VxB.y) * (VxA.x - VxC.x))) >= 0)
	{
		vxUV[1] = F->GetVxuvA();
		vxUV[0] = F->GetVxuvB();
		vxUV[2] = F->GetVxuvC();
	}
	else
	{
		vxUV[0] = F->GetVxuvA();
		vxUV[1] = F->GetVxuvB();
		vxUV[2] = F->GetVxuvC();
	}

	return true;
}

// ---------------------------------------------------------
// for DrawPolyGT3 works, yA <= yB <= yC is 
//	required, this code reorder vertex and draw face
//
// order face vertex and draw face depending of PolyMode
// ---------------------------------------------------------
void CBoard3D::DrawFace(const TFace *F, unsigned int tface_flag )
{	
#ifdef WIN_DEBUG
	extern int gNbFacesDrawn;
	gNbFacesDrawn++;	
#endif

	m_bindTexture = F->GetTexture();
	
	A_ASSERT (m_bindTexture != NULL);

	//	PERF_COUNTER(CBoard3D_DrawFace);

	const TVxuv* vxUV[3];

	if (PrepareFace(F, vxUV))
	{
		if (tface_flag & TFACE_FLAG_FLAT)
			DrawPolyG3(vxUV[0], vxUV[1], vxUV[2], (tface_flag & TFACE_FLAG_TRANS),(tface_flag & TFACE_FLAG_TRANS_ADDITIVE));
		else if (tface_flag & TFACE_FLAG_ZCORRECTED)
		{
//			if (tface_flag & TFACE_FLAG_ENVMAP)
//				DrawPolyGT3zCar(vxUV[0], vxUV[1], vxUV[2]);
//			else DrawPolyGT3z(vxUV[0], vxUV[1], vxUV[2]);
			DrawPolyGT3z(vxUV[0], vxUV[1], vxUV[2]);
		}
		else if(tface_flag & TFACE_FLAG_TRANS_ADDITIVE)
			DrawPolyGT3Additive(vxUV[0], vxUV[1], vxUV[2]);
		else if(tface_flag & TFACE_FLAG_TRANS_SUBSTRATIVE)
			DrawPolyGT3Substractive(vxUV[0], vxUV[1], vxUV[2]);
		else if (tface_flag & TFACE_FLAG_TRANS_SHADOW)
			DrawPolyGT3Shadow(vxUV[0], vxUV[1], vxUV[2]);
		else if (tface_flag & TFACE_FLAG_TRANS_ADDITIVE_ALPHA)
			DrawPolyGT3AdditiveAlpha(vxUV[0], vxUV[1], vxUV[2]);
		else if (tface_flag & TFACE_FLAG_ALPHA_MASK)
			DrawPolyGT3AlphaMask(vxUV[0], vxUV[1], vxUV[2]);
		else
		{
			// rax - no MIP textures
			// mip only normal faces
			//if (/*(m_TextureFXIdx == kTextureFxModeNormal) &&*/ (((TTexture*)m_bindTexture)->HasMIP()))
			//	m_bindTexture = ((TTexture*)m_bindTexture)->GetMIPz(vxUV[0]->Vx->z);

			DrawPolyGT3(vxUV[0], vxUV[1], vxUV[2]);
		}
	}
}

// ---------------------------------------------------------
//	
// ---------------------------------------------------------
void CBoard3D::Clear()
{
	TPixel*	pVScreen	= m_screen;
	
	const TPixel* End = pVScreen + (m_dispX*m_dispY);
	
	while(pVScreen < End)
	{
		*(pVScreen++)	= (short)kClearColour;
	}
}

void CBoard3D::Clear(unsigned short in_clearColor)
{
	TPixel*	pVScreen	= m_screen;
	
	const TPixel* End = pVScreen + (m_dispX*m_dispY);
	
	while(pVScreen < End)
	{
		*(pVScreen++)	= in_clearColor;
	}
}

#if USE_STENCIL_BUFFER

void CBoard3D::ClearStencil(bool force) {
	if(force) {
		m_stencilBeginX = 0;
		m_stencilBeginY = 0;
		m_stencilEndX	= m_dispX - 1;
		m_stencilEndY	= m_dispY - 1;
	}


	// LAngelov: Stencil buffer not used this frame, so it's already cleared
	if(m_stencilEndX == -1)
		return;

	A_ASSERT(m_stencilBeginX	>= 0 && m_stencilBeginX	< m_dispX);
	A_ASSERT(m_stencilBeginY	>= 0 && m_stencilBeginY	< m_dispY);
	A_ASSERT(m_stencilEndX	>= 0 && m_stencilEndX	< m_dispX);
	A_ASSERT(m_stencilEndY	>= 0 && m_stencilEndY	< m_dispY);

	int stencilSizeX = m_stencilEndX - m_stencilBeginX + 1;
	if (stencilSizeX <= 0)
		return;

	int beginOffset		= ((m_stencilBeginY * m_dispX) + m_stencilBeginX);
	int stencilRowJump	= (m_dispX - (m_stencilEndX - m_stencilBeginX)) - 1;

	TPixel*			pStencilBuffer	= m_stencilBuffer	+ beginOffset;
	unsigned char*	pStencilAlpha	= m_stencilAlpha	+ beginOffset;
	
	if (pStencilBuffer && pStencilAlpha)
	{
		for(int y = m_stencilEndY - m_stencilBeginY; y >= 0; y--)
		{
			//for(int x=m_stencilBeginX; x<=m_stencilEndX; x++)
			//{
			//	*(pStencilBuffer++)	= 0;
			//	*(pStencilAlpha++)	= 0;
			//}
			//pStencilBuffer	+= stencilRowJump;
			//pStencilAlpha	+= stencilRowJump;

			memset(pStencilBuffer, 0, stencilSizeX * sizeof(*pStencilBuffer));
			memset(pStencilAlpha,  0, stencilSizeX * sizeof(*pStencilAlpha));
	
			pStencilBuffer	+= stencilSizeX + stencilRowJump;
			pStencilAlpha	+= stencilSizeX + stencilRowJump;
		}
	}

	m_stencilBeginX = m_dispX+1;
	m_stencilBeginY = m_dispY+1;
	m_stencilEndX	= -1;
	m_stencilEndY	= -1;
}

//#define DEBUG_STENCIL_BUFFER

void CBoard3D::FlushStencilBuffer()
{
	// LAngelov: Stencil buffer not used this frame, so no flush is needed
	if(m_stencilEndX == -1)
		return;

	A_ASSERT(m_stencilBeginX	>= 0 && m_stencilBeginX	< m_dispX);
	A_ASSERT(m_stencilBeginY	>= 0 && m_stencilBeginY	< m_dispY);
	A_ASSERT(m_stencilEndX	>= 0 && m_stencilEndX	< m_dispX);
	A_ASSERT(m_stencilEndY	>= 0 && m_stencilEndY	< m_dispY);

	int beginOffset		= ((m_stencilBeginY * m_dispX) + m_stencilBeginX);
	int stencilRowJump	= (m_dispX - (m_stencilEndX - m_stencilBeginX)) - 1;

	TPixel*			screen			= m_screen			+ beginOffset;
	TPixel*			stencilBuffer	= m_stencilBuffer	+ beginOffset;
	unsigned char*	stencilAlpha	= m_stencilAlpha	+ beginOffset;

	for (int y = m_stencilEndY - m_stencilBeginY; y >= 0; --y)
	{
		for(int x = m_stencilEndX - m_stencilBeginX; x >= 0; --x)
		{
			if(*stencilBuffer && *stencilAlpha)
			{
				*screen = CLib2D::MixColor(*stencilBuffer, *screen, *stencilAlpha & 0xF);
			}

#ifdef DEBUG_STENCIL_BUFFER
			if(x==m_stencilBeginX || x==m_stencilEndX || y==m_stencilBeginY || y==m_stencilEndY) {
				*screen = 0xF81F;
			}
#endif

			++screen;
			++stencilBuffer;
			++stencilAlpha;
		}
		
		screen			+= stencilRowJump;
		stencilBuffer	+= stencilRowJump;
		stencilAlpha	+= stencilRowJump;
	}
}
#endif // USE_STENCIL_BUFFER
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Draw a textured triangle with gouraud, not perspective correction, no HRS
// Required : ya <= yb <= yc 
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

void CBoard3D::DrawPolyGT3(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;

#if PLAYERCARFRESNEL
	int const dzAC = C->Vx->z - A->Vx->z;
	int const dzAB = B->Vx->z - A->Vx->z;
	Vector4s const cross = Vector4s::Cross(Vector4s(dxAB, dyAB, dzAB), Vector4s(dxAC, dyAC, dzAC));
	int const crossLength = cross.SafeLength();
	int const dot8temp = Clamped(crossLength ? (Abs(cross.z) << 8) / crossLength : 0, 0, 256);
	int const alpha = Clamped((Square(Square(256 - dot8temp) >> 8) >> 12) * 3 + 5, 0, 0x30);
#endif

	// ----------------------------------
	//const int k = (dyAB) ? (dyAB*InverseTable(dyAC)) >> (INVTBL_SHIFT - 16) :  0;
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;
	
	
	CHK_MULT(dxAC,k);
	
	//int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	int x10 = dxAB - DownShift16(dxAC*k);
	
	if (!x10)
		return;
	
	const int duAC = C->u - A->u;
	const int dvAC = C->v - A->v;

	const int duAB = B->u - A->u;
	const int dvAB = B->v - A->v;
	
	CHK_MULT(duAC,k);	CHK_MULT(dvAC,k);
	
	//const int uABC = duAB - ((duAC*k) >> 16);
	//const int vABC = dvAB - ((dvAC*k) >> 16);
	
	const int uABC = duAB - DownShift16(duAC*k);
	const int vABC = dvAB - DownShift16(dvAC*k);
	
	m_uvShift			= DIV_SHIFT- TTexture::TEX_UV_SHIFT;
	int invShift	= INVTBL_SHIFT;
	
	int invk_x10;
	
	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}
	
	CHK_MULT(uABC,invk_x10);
	CHK_MULT(vABC,invk_x10);
	
	m_du = (uABC * invk_x10) >> (invShift - m_uvShift);
	m_dv = (vABC * invk_x10) >> (invShift - m_uvShift);
	
	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);
	
	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}
	
	CHK_MULT(duAC,invk_dyAC);
	CHK_MULT(dvAC,invk_dyAC);
	
	
	const LineParamNoZ	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT),
		0,
		(duAC * invk_dyAC) >> (invShift - m_uvShift),
		(dvAC * invk_dyAC) >> (invShift - m_uvShift));
	
	// initial values
	LineParamNoZ valAC(	A->Vx->x << DIV_SHIFT,
		A->Vx->z << DIV_SHIFT,
		A->u << m_uvShift ,
		A->v << m_uvShift);
	
	int YDraw = A->Vx->y;
	
	
	//const int averageDUDV = (m_du + m_dv) >> 1;

	
	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZ valAB,deltaAB;
	LineParamNoZ valBC,deltaBC;
	
	int RDir;
	
	int InvdyBC, InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{
		
		InvdyBC = InverseTable(dyBC);
		
		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			deltaBC.u = ((C->u - B->u) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.v = ((C->v - B->v) * InvdyBC) >> (INVTBL_SHIFT - 12);
			
			valBC.Set(B->Vx->x << 16,B->Vx->z << 16,
				B->u << 12,B->v << 12);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		
		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);
		
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			deltaAB.u = (duAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.v = (dvAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			
			valAB= valAC;      
			
			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
#if PLAYERCARFRESNEL
				Lib3D::DrawScanLine(*this,YDraw,valAC,x2,alpha);
#else
				Lib3D::DrawScanLine(*this,YDraw,valAC,x2);
#endif
			++YDraw;
			
			valAC.AddDelta(deltaAC);      
		}
		
		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part
			
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
#if PLAYERCARFRESNEL
				Lib3D::DrawScanLine(*this,YDraw,valAC,x2,alpha);
#else
				Lib3D::DrawScanLine(*this,YDraw,valAC,x2);
#endif

			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;
				
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
#if PLAYERCARFRESNEL
					Lib3D::DrawScanLine(*this,YDraw,valAB,x2,alpha);
#else
					Lib3D::DrawScanLine(*this,YDraw,valAB,x2);
#endif

				// top triangle part
				
				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
#if PLAYERCARFRESNEL
					Lib3D::DrawScanLine(*this,YDraw,valBC,x2,alpha);
#else
					Lib3D::DrawScanLine(*this,YDraw,valBC,x2);
#endif
				
				valBC.AddDelta(deltaBC);        
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Draw a textured triangle with gouraud, not perspective correction, no HRS
// Required : ya <= yb <= yc 
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

void CBoard3D::DrawPolyGT3Additive(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;

	// ----------------------------------
	//const int k = (dyAB) ? (dyAB*InverseTable(dyAC)) >> (INVTBL_SHIFT - 16) :  0;
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;
	
	
	CHK_MULT(dxAC,k);
	
	//int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	int x10 = dxAB - DownShift16(dxAC*k);
	
	if (!x10)
		return;
	
	const int duAC = C->u - A->u;
	const int dvAC = C->v - A->v;

	const int duAB = B->u - A->u;
	const int dvAB = B->v - A->v;
	
	CHK_MULT(duAC,k);	CHK_MULT(dvAC,k);	
	
	//const int uABC = duAB - ((duAC*k) >> 16);
	//const int vABC = dvAB - ((dvAC*k) >> 16);
	
	const int uABC = duAB - DownShift16(duAC*k);
	const int vABC = dvAB - DownShift16(dvAC*k);
	
	m_uvShift		= DIV_SHIFT- TTexture::TEX_UV_SHIFT;
	int invShift	= INVTBL_SHIFT;
	
	int invk_x10;
	
	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}
	
	CHK_MULT(uABC,invk_x10);
	CHK_MULT(vABC,invk_x10);
	
	m_du = (uABC * invk_x10) >> (invShift - m_uvShift);
	m_dv = (vABC * invk_x10) >> (invShift - m_uvShift);
	
	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);
	
	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}
	
	CHK_MULT(duAC,invk_dyAC);
	CHK_MULT(dvAC,invk_dyAC);
	
	
	const LineParamNoZ	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT),
		0,
		(duAC * invk_dyAC) >> (invShift - m_uvShift),
		(dvAC * invk_dyAC) >> (invShift - m_uvShift));
	
	// initial values
	LineParamNoZ valAC(	A->Vx->x << DIV_SHIFT,
		A->Vx->z << DIV_SHIFT,
		A->u << m_uvShift ,
		A->v << m_uvShift);
	
	int YDraw = A->Vx->y;
	
	
	//const int averageDUDV = (m_du + m_dv) >> 1;

	
	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZ valAB,deltaAB;
	LineParamNoZ valBC,deltaBC;
	
	int RDir;
	
	int InvdyBC, InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{
		
		InvdyBC = InverseTable(dyBC);
		
		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			deltaBC.u = ((C->u - B->u) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.v = ((C->v - B->v) * InvdyBC) >> (INVTBL_SHIFT - 12);
			
			valBC.Set(B->Vx->x << 16,B->Vx->z << 16,
				B->u << 12,B->v << 12);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		
		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);
		
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			deltaAB.u = (duAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.v = (dvAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			
			valAB= valAC;      
			
			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineAdditive(*this,YDraw,valAC,x2);
			++YDraw;
			
			valAC.AddDelta(deltaAC);      
		}
		
		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part
			
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
				Lib3D::DrawScanLineAdditive(*this,YDraw,valAC,x2);

			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;
				
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineAdditive(*this,YDraw,valAB,x2);

				// top triangle part
				
				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineAdditive(*this,YDraw,valBC,x2);
				
				valBC.AddDelta(deltaBC);        
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CBoard3D::DrawPolyGT3AdditiveAlpha(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;

	// ----------------------------------
	//const int k = (dyAB) ? (dyAB*InverseTable(dyAC)) >> (INVTBL_SHIFT - 16) :  0;
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;
	
	
	CHK_MULT(dxAC,k);
	
	//int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	int x10 = dxAB - DownShift16(dxAC*k);
	
	if (!x10)
		return;
	
	const int duAC = C->u - A->u;
	const int dvAC = C->v - A->v;

	const int duAB = B->u - A->u;
	const int dvAB = B->v - A->v;
	
	CHK_MULT(duAC,k);	CHK_MULT(dvAC,k);	
	
	//const int uABC = duAB - ((duAC*k) >> 16);
	//const int vABC = dvAB - ((dvAC*k) >> 16);
	
	const int uABC = duAB - DownShift16(duAC*k);
	const int vABC = dvAB - DownShift16(dvAC*k);
	
	m_uvShift		= DIV_SHIFT- TTexture::TEX_UV_SHIFT;
	int invShift	= INVTBL_SHIFT;
	
	int invk_x10;
	
	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}
	
	CHK_MULT(uABC,invk_x10);
	CHK_MULT(vABC,invk_x10);
	
	m_du = (uABC * invk_x10) >> (invShift - m_uvShift);
	m_dv = (vABC * invk_x10) >> (invShift - m_uvShift);
	
	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);
	
	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}
	
	CHK_MULT(duAC,invk_dyAC);
	CHK_MULT(dvAC,invk_dyAC);
	
	
	const LineParamNoZ	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT),
		0,
		(duAC * invk_dyAC) >> (invShift - m_uvShift),
		(dvAC * invk_dyAC) >> (invShift - m_uvShift));
	
	// initial values
	LineParamNoZ valAC(	A->Vx->x << DIV_SHIFT,
		A->Vx->z << DIV_SHIFT,
		A->u << m_uvShift ,
		A->v << m_uvShift);
	
	int YDraw = A->Vx->y;
	
	
	//const int averageDUDV = (m_du + m_dv) >> 1;

	
	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZ valAB,deltaAB;
	LineParamNoZ valBC,deltaBC;
	
	int RDir;
	
	int InvdyBC, InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{
		
		InvdyBC = InverseTable(dyBC);
		
		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			deltaBC.u = ((C->u - B->u) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.v = ((C->v - B->v) * InvdyBC) >> (INVTBL_SHIFT - 12);
			
			valBC.Set(B->Vx->x << 16,B->Vx->z << 16,
				B->u << 12,B->v << 12);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		
		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);
		
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			deltaAB.u = (duAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.v = (dvAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			
			valAB= valAC;      
			
			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineAdditiveAlpha(*this,YDraw,valAC,x2);
			++YDraw;
			
			valAC.AddDelta(deltaAC);      
		}
		
		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part
			
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
				Lib3D::DrawScanLineAdditiveAlpha(*this,YDraw,valAC,x2);

			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;
				
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineAdditiveAlpha(*this,YDraw,valAB,x2);

				// top triangle part
				
				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineAdditiveAlpha(*this,YDraw,valBC,x2);
				
				valBC.AddDelta(deltaBC);        
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CBoard3D::DrawPolyGT3AlphaMask(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;

	// ----------------------------------
	//const int k = (dyAB) ? (dyAB*InverseTable(dyAC)) >> (INVTBL_SHIFT - 16) :  0;
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;
	
	
	CHK_MULT(dxAC,k);
	
	//int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	int x10 = dxAB - DownShift16(dxAC*k);
	
	if (!x10)
		return;
	
	const int duAC = C->u - A->u;
	const int dvAC = C->v - A->v;

	const int duAB = B->u - A->u;
	const int dvAB = B->v - A->v;
	
	CHK_MULT(duAC,k);	CHK_MULT(dvAC,k);	
	
	//const int uABC = duAB - ((duAC*k) >> 16);
	//const int vABC = dvAB - ((dvAC*k) >> 16);
	
	const int uABC = duAB - DownShift16(duAC*k);
	const int vABC = dvAB - DownShift16(dvAC*k);
	
	m_uvShift		= DIV_SHIFT- TTexture::TEX_UV_SHIFT;
	int invShift	= INVTBL_SHIFT;
	
	int invk_x10;
	
	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}
	
	CHK_MULT(uABC,invk_x10);
	CHK_MULT(vABC,invk_x10);
	
	m_du = (uABC * invk_x10) >> (invShift - m_uvShift);
	m_dv = (vABC * invk_x10) >> (invShift - m_uvShift);
	
	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);
	
	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}
	
	CHK_MULT(duAC,invk_dyAC);
	CHK_MULT(dvAC,invk_dyAC);
	
	
	const LineParamNoZ	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT),
		0,
		(duAC * invk_dyAC) >> (invShift - m_uvShift),
		(dvAC * invk_dyAC) >> (invShift - m_uvShift));
	
	// initial values
	LineParamNoZ valAC(	A->Vx->x << DIV_SHIFT,
		A->Vx->z << DIV_SHIFT,
		A->u << m_uvShift ,
		A->v << m_uvShift);
	
	int YDraw = A->Vx->y;
	
	
	//const int averageDUDV = (m_du + m_dv) >> 1;

	
	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZ valAB,deltaAB;
	LineParamNoZ valBC,deltaBC;
	
	int RDir;
	
	int InvdyBC, InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{
		
		InvdyBC = InverseTable(dyBC);
		
		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			deltaBC.u = ((C->u - B->u) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.v = ((C->v - B->v) * InvdyBC) >> (INVTBL_SHIFT - 12);
			
			valBC.Set(B->Vx->x << 16,B->Vx->z << 16,
				B->u << 12,B->v << 12);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		
		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);
		
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			deltaAB.u = (duAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.v = (dvAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			
			valAB= valAC;      
			
			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineAlphaMask(*this,YDraw,valAC,x2);
			++YDraw;
			
			valAC.AddDelta(deltaAC);      
		}
		
		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part
			
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
				Lib3D::DrawScanLineAlphaMask(*this,YDraw,valAC,x2);

			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;
				
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineAlphaMask(*this,YDraw,valAB,x2);

				// top triangle part
				
				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineAlphaMask(*this,YDraw,valBC,x2);
				
				valBC.AddDelta(deltaBC);        
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CBoard3D::DrawPolyGT3Shadow(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;

	// ----------------------------------
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;

	CHK_MULT(dxAC,k);

	int x10 = dxAB - DownShift16(dxAC*k);

	if (!x10)
		return;

	int invShift	= INVTBL_SHIFT;

	int invk_x10;

	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}
	
	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);

	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}



	const LineParamNoZUV	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT), 0);

	// initial values
	LineParamNoZUV valAC(	A->Vx->x << DIV_SHIFT, A->Vx->z << DIV_SHIFT);

	int YDraw = A->Vx->y;

	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZUV valAB,deltaAB;
	LineParamNoZUV valBC,deltaBC;

	int RDir;

	int InvdyBC, InvdyAB;

	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{

		InvdyBC = InverseTable(dyBC);

		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);

		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);

		if (deltaAC.x == deltaBC.x)
			return;

		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;

			valBC.Set(B->Vx->x << 16,B->Vx->z << 16);

			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}

	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{

		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);

		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);

		if (deltaAC.x == deltaAB.x)
			return;

		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			valAB= valAC;      

			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}

	const int Yend = ::LastLine(C, m_dispY);


	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);

			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part

			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineShadow(*this,YDraw,valAC,x2);
			++YDraw;

			valAC.AddDelta(deltaAC);      
		}

		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);

			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part


			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
				Lib3D::DrawScanLineShadow(*this,YDraw,valAC,x2);

			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;

				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);

				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineShadow(*this,YDraw,valAB,x2);

				// top triangle part

				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);

				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineShadow(*this,YDraw,valBC,x2);

				valBC.AddDelta(deltaBC);
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}


// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Draw a textured triangle with gouraud, not perspective correction, no HRS
// Required : ya <= yb <= yc 
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

void CBoard3D::DrawPolyGT3Substractive(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;

	// ----------------------------------
	//const int k = (dyAB) ? (dyAB*InverseTable(dyAC)) >> (INVTBL_SHIFT - 16) :  0;
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;


	CHK_MULT(dxAC,k);

	//int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	int x10 = dxAB - DownShift16(dxAC*k);

	if (!x10)
		return;

	const int duAC = C->u - A->u;
	const int dvAC = C->v - A->v;

	const int duAB = B->u - A->u;
	const int dvAB = B->v - A->v;

	CHK_MULT(duAC,k);	CHK_MULT(dvAC,k);	

	//const int uABC = duAB - ((duAC*k) >> 16);
	//const int vABC = dvAB - ((dvAC*k) >> 16);

	const int uABC = duAB - DownShift16(duAC*k);
	const int vABC = dvAB - DownShift16(dvAC*k);

	m_uvShift			= DIV_SHIFT- TTexture::TEX_UV_SHIFT;
	int invShift	= INVTBL_SHIFT;

	int invk_x10;

	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}

	CHK_MULT(uABC,invk_x10);
	CHK_MULT(vABC,invk_x10);

	m_du = (uABC * invk_x10) >> (invShift - m_uvShift);
	m_dv = (vABC * invk_x10) >> (invShift - m_uvShift);

	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);

	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}

	CHK_MULT(duAC,invk_dyAC);
	CHK_MULT(dvAC,invk_dyAC);


	const LineParamNoZ	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT),
		0,
		(duAC * invk_dyAC) >> (invShift - m_uvShift),
		(dvAC * invk_dyAC) >> (invShift - m_uvShift));

	// initial values
	LineParamNoZ valAC(	A->Vx->x << DIV_SHIFT,
		A->Vx->z << DIV_SHIFT,
		A->u << m_uvShift ,
		A->v << m_uvShift);

	int YDraw = A->Vx->y;


	//const int averageDUDV = (m_du + m_dv) >> 1;


	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZ valAB,deltaAB;
	LineParamNoZ valBC,deltaBC;

	int RDir;

	int InvdyBC, InvdyAB;

	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{

		InvdyBC = InverseTable(dyBC);

		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);

		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);

		if (deltaAC.x == deltaBC.x)
			return;

		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;

			deltaBC.u = ((C->u - B->u) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.v = ((C->v - B->v) * InvdyBC) >> (INVTBL_SHIFT - 12);

			valBC.Set(B->Vx->x << 16,B->Vx->z << 16,
				B->u << 12,B->v << 12);

			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}

	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{

		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);

		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);

		if (deltaAC.x == deltaAB.x)
			return;

		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;

			deltaAB.u = (duAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.v = (dvAB * InvdyAB) >> (INVTBL_SHIFT - 12);

			valAB= valAC;      

			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}

	const int Yend = ::LastLine(C, m_dispY);


	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);

			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part

			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineSubstractive(*this,YDraw,valAC,x2);
			++YDraw;

			valAC.AddDelta(deltaAC);      
		}

		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);

			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part


			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
				Lib3D::DrawScanLineSubstractive(*this,YDraw,valAC,x2);

			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;

				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);

				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineSubstractive(*this,YDraw,valAB,x2);

				// top triangle part

				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);

				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineSubstractive(*this,YDraw,valBC,x2);

				valBC.AddDelta(deltaBC);        
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Draw a flat triangle 
// Required : ya <= yb <= yc 
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

void CBoard3D::DrawPolyG3(const TVxuv *A, const TVxuv *B, const TVxuv *C, unsigned int trans, unsigned int neon)
{
	int dyAC = C->Vx->y - A->Vx->y;
	int dyAB = B->Vx->y - A->Vx->y;
	int dxAC = C->Vx->x - A->Vx->x;
	int dxAB = B->Vx->x - A->Vx->x;
	
	// ----------------------------------
	const int k = (dyAB) ? DivideShift16(dyAB , dyAC) :  0;
	
	
	CHK_MULT(dxAC,k);
	
	int x10 = dxAB - DownShift16(dxAC*k);
	
	if (!x10)
		return;
	
	int invShift	= INVTBL_SHIFT;
	
	int invk_x10;
	
	if (x10 >= 0)
	{
		invk_x10 = InverseTable(x10);
		if(invk_x10 & 0xFFFF0000)
		{
			invk_x10 >>= 2;
			invShift -= 2;
		}
	}
	else
	{
		invk_x10 = InverseTable(-x10);
		if((invk_x10) & 0xFFFF0000)
		{
			invk_x10	>>= 2;
			invShift	-= 2;
		}
		invk_x10 = -invk_x10;
	}
	
	invShift	= INVTBL_SHIFT;
	int invk_dyAC = InverseTable(dyAC);
	
	if(invk_dyAC & 0xFFFF0000)
	{
		invk_dyAC >>= 2;
		invShift  -= 2;
	}
	
	
	
	
	
	const LineParamNoZ	deltaAC(	(dxAC * invk_dyAC) >> (invShift - DIV_SHIFT), 0,0,0);
	
	// initial values
	LineParamNoZ valAC(	A->Vx->x << DIV_SHIFT,
		A->Vx->z << DIV_SHIFT,
		A->u << m_uvShift ,
		A->v << m_uvShift);
	
	int YDraw = A->Vx->y;
	

	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	LineParamNoZ valAB,deltaAB;
	LineParamNoZ valBC,deltaBC;
	
	int RDir;
	
	int InvdyBC, InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	if ((B->Vx->y < CLIPYEND) && dyBC)               // bottom face part visible and not flat
	{
		
		InvdyBC = InverseTable(dyBC);
		
		CHK_MULT( (C->Vx->x - B->Vx->x) , InvdyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			
			valBC.Set(B->Vx->x << 16,B->Vx->z << 16, 0, 0);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		
		InvdyAB = InverseTable(dyAB);
		CHK_MULT((B->Vx->x - A->Vx->x) , InvdyAB);
		
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			valAB.x = valAC.x;
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			valAB= valAC;      
			
			if (A->Vx->y < 0)
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	if (RDir)                                      // face oriented on right
	{
		int x2;      
		while (YDraw < B->Vx->y && YDraw<Yend)
		{      
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valAB.x);
			valAB.x += deltaAB.x;                         // up triangle part
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
			{
				if (neon)
					Lib3D::DrawScanLineFlatTransNeon(*this,YDraw,valAC,x2);
				else if(trans)
					Lib3D::DrawScanLineFlatTrans(*this,YDraw,valAC,x2);
				else
					Lib3D::DrawScanLineFlat(*this,YDraw,valAC,x2);
			}	
			++YDraw;
			valAC.AddDelta(deltaAC);      
		}
		
		while (YDraw < Yend)
		{      
			//const int x1 = iAC_x >> 16;
			const int x1 = DownShift16(valAC.x);
			
			x2 = DownShift16(valBC.x);
			valBC.x += deltaBC.x;                         // down triangle part
			
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))      				
			{
				if(neon)
					Lib3D::DrawScanLineFlatTransNeon(*this,YDraw,valAC,x2);
				else if (trans)
					Lib3D::DrawScanLineFlatTrans(*this,YDraw,valAC,x2);
				else
					Lib3D::DrawScanLineFlat(*this,YDraw,valAC,x2);
			}
			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = iAB_x >> 16;
				//x2 = iAC_x >> 16;
				
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				{
					if(neon)
						Lib3D::DrawScanLineFlatTransNeon(*this,YDraw,valAB,x2);
					else if (trans)
						Lib3D::DrawScanLineFlatTrans(*this,YDraw,valAB,x2);
					else
						Lib3D::DrawScanLineFlat(*this,YDraw,valAB,x2);
				}					
				// top triangle part
				
				valAB.AddDelta(deltaAB);        
			}
			else
			{
				// bottom triangle part
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))	
				{
					if (neon)
						Lib3D::DrawScanLineFlatTransNeon(*this,YDraw,valBC,x2);	
					else if (trans)
						Lib3D::DrawScanLineFlatTrans(*this,YDraw,valBC,x2);	
					else
						Lib3D::DrawScanLineFlat(*this,YDraw,valBC,x2);	
				}					
				valBC.AddDelta(deltaBC);        
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}



bool CBoard3D::PrepareFace(const TFace* F,const TVxuv** vxUV)
{	
	const Vector4s&	vxA = F->ScrVectorA();
	const Vector4s&	vxB = F->ScrVectorB();
	const Vector4s&	vxC = F->ScrVectorC() ;
	
	
	// note: only 1% to 2% of faces are rejected using this code. todo: test speed gain/loss
	// check if face is out of the screen
	if (vxA.x > vxB.x)
	{
		if (vxA.x > vxC.x)
		{
			if (vxA.x < 0) 
				return false;
		}
		else
			// xC > xA
			if (vxC.x < 0) 
				return false;
			
			// xB < xA
			if (vxC.x < vxB.x)
			{
				if (vxC.x >= m_dispX) 
					return false;
			}
			else
				// xB < xC 
				if (vxB.x >= m_dispX) 
					return false;
	}
	else  // xB > xA
	{
		if (vxB.x > vxC.x)
		{
			if (vxB.x < 0) 
				return false;
		}
		else
			// xC > xB
			if (vxC.x < 0) 
				return false;
			
			// xA < xB
			if (vxC.x < vxA.x)
			{
				if (vxC.x >= m_dispX) 
					return false;
			}
			else
				// xA < xC
				if (vxA.x >= m_dispX) 
					return false;
	}
	
	
	// for DrawPolyGT3 works, yA <= yB <= yC is required
	if (vxA.y <= vxB.y)
	{
		if (vxB.y <= vxC.y)
		{
			// draw acb
			if ((vxA.y >= m_dispY) || (vxC.y < 0) || (vxC.y <= vxA.y)) return false;
			vxUV[0] = F->GetVxuvA();
			vxUV[1] = F->GetVxuvB();
			vxUV[2] = F->GetVxuvC();
		}
		else
		{
			if (vxA.y <= vxC.y)
			{
				// draw acb
				if ((vxA.y >= m_dispY) || (vxB.y < 0)  || (vxB.y <= vxA.y)) return false;
				vxUV[0] = F->GetVxuvA();
				vxUV[1] = F->GetVxuvC();
				vxUV[2] = F->GetVxuvB();
			}
			else
			{
				// draw cab
				if ((vxC.y >= m_dispY) || (vxB.y < 0)  || (vxB.y <= vxC.y)) return false;
				vxUV[0] = F->GetVxuvC();
				vxUV[1] = F->GetVxuvA();
				vxUV[2] = F->GetVxuvB();
			}
		}
	}
	else
	{
		if (vxC.y >= vxA.y)
		{
			// draw bac
			if ((vxB.y >= m_dispY) || (vxC.y < 0)  || (vxC.y <= vxB.y)) return false;
			vxUV[0] = F->GetVxuvB();
			vxUV[1] = F->GetVxuvA();
			vxUV[2] = F->GetVxuvC();
		}
		else
		{
			if (vxC.y <= vxB.y)
			{
				// draw cba
				if ((vxC.y >= m_dispY) || (vxA.y < 0) || (vxA.y <= vxC.y)) return false;
				vxUV[0] = F->GetVxuvC();
				vxUV[1] = F->GetVxuvB();
				vxUV[2] = F->GetVxuvA();
			}
			else
			{
				// draw bca
				if ((vxB.y >= m_dispY) || (vxA.y < 0) || (vxA.y <= vxB.y)) return false;
				vxUV[0] = F->GetVxuvB();
				vxUV[1] = F->GetVxuvC();
				vxUV[2] = F->GetVxuvA();
			}
		}
	}
	return true;
}






// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Draw a textured triangle with gouraud, not perspective correction, no HRS
// Required : ya <= yb <= yc 
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

void CBoard3D::DrawPolyGT3z(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{	
	//	PERF_COUNTER(DrawPolyGT3z);
	const int hA = InverseTable(A->Vx->z);
	const int hB = InverseTable(B->Vx->z);
	const int hC = InverseTable(C->Vx->z);
	
	const int dhAC = hC - hA;
	const int dhAB = hB - hA;
	
	// check if can draw without correction
	if ((A->Vx->z > ZC_MINZ) && ((dhAC * dhAC) + (dhAB * dhAB)) < ZC_ERRMAX)  
	{

		// ##################################################
		// TEMP TBD mip only normal faces
		// rax - no MIP textures
		//if (m_TextureFXIdx == kTextureFxModeNormal)
		//	m_bindTexture = ((TTexture*)m_bindTexture)->GetMIPz(A->Vx->z);
		// ##################################################


		DrawPolyGT3(A, B, C);
		return;
	}
	
	
	const int dyAC = C->Vx->y - A->Vx->y;
	const int dyAB = B->Vx->y - A->Vx->y;
	const int dxAC = C->Vx->x - A->Vx->x;
	const int dxAB = B->Vx->x - A->Vx->x;	
	
	int k;
	if(dyAB)
	{
		int invDyAC = InverseTable(dyAC);
		if(invDyAC & 0xFFFF0000)
			k = (dyAB*(invDyAC>>2)) >> (INVTBL_SHIFT - 2 - DIV_SHIFT);
		else
			k = (dyAB*invDyAC) >> (INVTBL_SHIFT - DIV_SHIFT);
	}
	else
		k=0;
	
	const int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	if (!x10)
		return;
	
	// u/z -> u*(1/z)
	const int uA = A->u*hA;
	const int vA = A->v*hA;
	const int uB = B->u*hB;
	const int vB = B->v*hB;
	const int uC = C->u*hC;
	const int vC = C->v*hC;
	
	const int duAC = (uC - uA) >> 8;
	const int dvAC = (vC - vA) >> 8;
	const int dzAC = C->Vx->z - A->Vx->z;
	
	const int duAB = uB - uA;
	const int dvAB = vB - vA;
	
	const int dzAB = B->Vx->z - A->Vx->z;
	
	//const int hABC = dhAB - ((dhAC*k) >> 16);
	const int hABC = dhAB - DownShift16(dhAC*k);
	const int k8 = k >> 8;                               // reduce value to avoid overflow
	
	const int uABC = (duAB - (duAC*k8)) >> 8;
	const int vABC = (dvAB - (dvAC*k8)) >> 8;
	
	//const int zABC = dzAB - ((dzAC*k) >> 16);
	const int zABC = dzAB - DownShift16(dzAC*k);
	
	int Invk;
	if (x10 >= 0)
	{
		int Index = x10 & DIVTBL_MASK;
		Invk = InverseTable(Index);
	}
	else
	{
		int Index = (-x10) & DIVTBL_MASK;
		Invk = -InverseTable(Index);
	}
	
	Invk >>= 2;        // required to ensure no overflow
	int invertShift = INVTBL_SHIFT-2;
	
	m_uvShift = DIV_SHIFT - TTexture::TEX_UV_SHIFT - 4;
	
	CHK_MULT(hABC , Invk);CHK_MULT(uABC , Invk);CHK_MULT(vABC , Invk);
	
	m_dh =  (hABC * Invk) >> (invertShift - 12) ;
	m_du =  (uABC * Invk) >> (invertShift - m_uvShift);
	m_dv =  (vABC * Invk) >> (invertShift - m_uvShift);
	m_dz = (zABC * Invk) >> (INVTBL_SHIFT - 16 - 2);
	
	
	invertShift = INVTBL_SHIFT;
	Invk = InverseTable(dyAC);
	if(Invk  & 0xFFFF0000)
	{
		Invk >>=2;
		invertShift -=2;
	}
	
	CHK_MULT(dhAC , Invk);CHK_MULT((duAC >> 2) , Invk);CHK_MULT((dvAC >> 2) , Invk);
	
	CHK_MULT(dzAC,Invk);
	CHK_MULT(dxAC,Invk);
	
	LineParamZ deltaAC(	(dxAC * Invk) >> (invertShift - 16),
		(dzAC * Invk) >> (invertShift - 16),
		(dhAC * Invk) >> (invertShift - 12),
		((duAC >> 2) * Invk) >> (invertShift - m_uvShift -2),
		((dvAC >> 2) * Invk) >> (invertShift - m_uvShift -2));
	
	// initial values
	LineParamZ valAC	(	A->Vx->x << 16,
		A->Vx->z << 16,
		hA << 12,
		uA,vA);
	
	int YDraw = A->Vx->y;
	
	//const int averageDUDV = (m_du + m_dv) >> 1;
	
	//const CTextureAccessor textureAccessor = m_bindTexture->GetAccessor(averageDUDV >> (m_uvShift + TTexture::TEX_UV_SHIFT));
	
	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	int RDir;
	int InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	
	LineParamZ	valBC;
	LineParamZ deltaBC;
	
	if ((B->Vx->y < CLIPYEND) && dyBC) // bottom face part visible and not flat
	{
		const int InvdyBC = InverseTable(dyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			deltaBC.h = ((hC - hB) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.u = (((uC - uB) >> (8 + 2)) * InvdyBC) >> (INVTBL_SHIFT - 8 - 2);
			deltaBC.v = (((vC - vB) >> (8 + 2)) * InvdyBC) >> (INVTBL_SHIFT - 8 - 2);
			
			deltaBC.z = ((C->Vx->z - B->Vx->z) * InvdyBC) >> (INVTBL_SHIFT - 16);
			
			valBC.Set(	B->Vx->x << 16,
				B->Vx->z << 16,
				hB << 12,
				uB,vB);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	LineParamZ valAB;
	LineParamZ	deltaAB;
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		valAB.x = valAC.x;
		InvdyAB = InverseTable(dyAB);
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			CHK_MULT(dhAB,InvdyAB);
			CHK_MULT(dzAB,InvdyAB);
			
			deltaAB.h = (dhAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.u = ((duAB >> (8 + 2)) * InvdyAB) >> (INVTBL_SHIFT - 8 - 2);
			deltaAB.v = ((dvAB >> (8 + 2)) * InvdyAB) >> (INVTBL_SHIFT - 8 - 2);			
			deltaAB.z = (dzAB * InvdyAB) >> (INVTBL_SHIFT - 16);
			
			
			// initial values
			valAB.h = valAC.h;
			valAB.u = valAC.u;
			valAB.v = valAC.v;
			
			valAB.z = valAC.z;
			
			if (A->Vx->y < 0)      
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	
	if (RDir)                                      // face oriented on right
	{
		while (YDraw < Yend)
		{
			
			const int x1 = valAC.x >> 16;
			
			int x2;
			if (YDraw < B->Vx->y)
			{
				//x2 = valAB.x >> 16;
				x2 = DownShift16(valAB.x);
				valAB.x += deltaAB.x;                         // up triangle part
			}
			else
			{
				//x2 = valBC.x >> 16;
				x2 = DownShift16(valBC.x);
				valBC.x += deltaBC.x;                         // down triangle part
			}
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineZCorrected(*this,YDraw,valAC,x2);
			
			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = valAB.x >> 16;
				//x2 = valAC.x >> 16;
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineZCorrected(*this,YDraw,valAB,x2);
				
				valAB.AddDelta(deltaAB);// top triangle part        
			}
			else
			{
				//x1 = valBC.x >> 16;
				//x2 = valAC.x >> 16;
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineZCorrected(*this,YDraw,valBC,x2);
				
				// bottom triangle part
				valBC.AddDelta(deltaBC);
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Draw a textured triangle with gouraud, not perspective correction, no HRS
// Required : ya <= yb <= yc 
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

void CBoard3D::DrawPolyGT3zCar(const TVxuv *A, const TVxuv *B, const TVxuv *C)
{	
	//	PERF_COUNTER(DrawPolyGT3z);
	const int hA = InverseTable(A->Vx->z);
	const int hB = InverseTable(B->Vx->z);
	const int hC = InverseTable(C->Vx->z);
	
	const int dhAC = hC - hA;
	const int dhAB = hB - hA;
	
	// check if can draw without correction
	if ((A->Vx->z > ZC_MINZ) && ((dhAC * dhAC) + (dhAB * dhAB)) < ZC_ERRMAX)  
	{

		// ##################################################
		// TEMP TBD mip only normal faces
		// rax - no MIP textures
		//if (m_TextureFXIdx == kTextureFxModeNormal)
		//	m_bindTexture = ((TTexture*)m_bindTexture)->GetMIPz(A->Vx->z);
		// ##################################################


		DrawPolyGT3(A, B, C);
		return;
	}
	

	const int dyAC = C->Vx->y - A->Vx->y;
	const int dyAB = B->Vx->y - A->Vx->y;
	const int dxAC = C->Vx->x - A->Vx->x;
	const int dxAB = B->Vx->x - A->Vx->x;	
	
	int k;
	if(dyAB)
	{
		int invDyAC = InverseTable(dyAC);
		if(invDyAC & 0xFFFF0000)
			k = (dyAB*(invDyAC>>2)) >> (INVTBL_SHIFT - 2 - DIV_SHIFT);
		else
			k = (dyAB*invDyAC) >> (INVTBL_SHIFT - DIV_SHIFT);
	}
	else
		k=0;
	
	const int x10 = dxAB - (((dxAC*k) + (1 << 15)) >> 16);
	if (!x10)
		return;
	
	// u/z -> u*(1/z)
	const int uA = A->u*hA;
	const int vA = A->v*hA;
	const int uB = B->u*hB;
	const int vB = B->v*hB;
	const int uC = C->u*hC;
	const int vC = C->v*hC;
	
	const int duAC = (uC - uA) >> 8;
	const int dvAC = (vC - vA) >> 8;
	const int dzAC = C->Vx->z - A->Vx->z;
	
	const int duAB = uB - uA;
	const int dvAB = vB - vA;
	
	const int dzAB = B->Vx->z - A->Vx->z;
	
	//const int hABC = dhAB - ((dhAC*k) >> 16);
	const int hABC = dhAB - DownShift16(dhAC*k);
	const int k8 = k >> 8;                               // reduce value to avoid overflow
	
	const int uABC = (duAB - (duAC*k8)) >> 8;
	const int vABC = (dvAB - (dvAC*k8)) >> 8;
	
	//const int zABC = dzAB - ((dzAC*k) >> 16);
	const int zABC = dzAB - DownShift16(dzAC*k);
	
	int Invk;
	if (x10 >= 0)
	{
		int Index = x10 & DIVTBL_MASK;
		Invk = InverseTable(Index);
	}
	else
	{
		int Index = (-x10) & DIVTBL_MASK;
		Invk = -InverseTable(Index);
	}
	
	Invk >>= 2;        // required to ensure no overflow
	int invertShift = INVTBL_SHIFT-2;
	
	m_uvShift = DIV_SHIFT - TTexture::TEX_UV_SHIFT - 4;
	
	CHK_MULT(hABC , Invk);CHK_MULT(uABC , Invk);CHK_MULT(vABC , Invk);
	
	m_dh =  (hABC * Invk) >> (invertShift - 12) ;
	m_du =  (uABC * Invk) >> (invertShift - m_uvShift);
	m_dv =  (vABC * Invk) >> (invertShift - m_uvShift);
	m_dz = (zABC * Invk) >> (INVTBL_SHIFT - 16 - 2);
	
	
	invertShift = INVTBL_SHIFT;
	Invk = InverseTable(dyAC);
	if(Invk  & 0xFFFF0000)
	{
		Invk >>=2;
		invertShift -=2;
	}
	
	CHK_MULT(dhAC , Invk);CHK_MULT((duAC >> 2) , Invk);CHK_MULT((dvAC >> 2) , Invk);
	
	CHK_MULT(dzAC,Invk);
	CHK_MULT(dxAC,Invk);
	
	LineParamZ deltaAC(	(dxAC * Invk) >> (invertShift - 16),
		(dzAC * Invk) >> (invertShift - 16),
		(dhAC * Invk) >> (invertShift - 12),
		((duAC >> 2) * Invk) >> (invertShift - m_uvShift -2),
		((dvAC >> 2) * Invk) >> (invertShift - m_uvShift -2));
	
	// initial values
	LineParamZ valAC	(	A->Vx->x << 16,
		A->Vx->z << 16,
		hA << 12,
		uA,vA);
	
	int YDraw = A->Vx->y;
	
	//const int averageDUDV = (m_du + m_dv) >> 1;
	
	//const CTextureAccessor textureAccessor = m_bindTexture->GetAccessor(averageDUDV >> (m_uvShift + TTexture::TEX_UV_SHIFT));
	
	// -----------------------------------------------
	//                Draw triangle 
	// -----------------------------------------------
	int RDir;
	int InvdyAB;
	
	const int dyBC = C->Vx->y - B->Vx->y;
	
	LineParamZ	valBC;
	LineParamZ deltaBC;
	
	if ((B->Vx->y < CLIPYEND) && dyBC) // bottom face part visible and not flat
	{
		const int InvdyBC = InverseTable(dyBC);
		
		deltaBC.x = ((C->Vx->x - B->Vx->x) * InvdyBC) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaBC.x)
			return;
		
		if (deltaAC.x > deltaBC.x)
		{
			valBC.x = B->Vx->x << 16;
			RDir = 1;
			if (B->Vx->y < 0)
				valBC.x -= B->Vx->y*deltaBC.x;
		}
		else
		{
			RDir = 0;
			
			deltaBC.h = ((hC - hB) * InvdyBC) >> (INVTBL_SHIFT - 12);
			deltaBC.u = (((uC - uB) >> (8 + 2)) * InvdyBC) >> (INVTBL_SHIFT - 8 - 2);
			deltaBC.v = (((vC - vB) >> (8 + 2)) * InvdyBC) >> (INVTBL_SHIFT - 8 - 2);
			
			deltaBC.z = ((C->Vx->z - B->Vx->z) * InvdyBC) >> (INVTBL_SHIFT - 16);
			
			valBC.Set(	B->Vx->x << 16,
				B->Vx->z << 16,
				hB << 12,
				uB,vB);
			
			if (B->Vx->y < 0)
				valBC.Substact(B->Vx->y,deltaBC);
		}
	}
	
	LineParamZ valAB;
	LineParamZ	deltaAB;
	
	if ((B->Vx->y >= 0) && dyAB)                     // up face part visible and not flat
	{
		valAB.x = valAC.x;
		InvdyAB = InverseTable(dyAB);
		deltaAB.x = ((B->Vx->x - A->Vx->x) * InvdyAB) >> (INVTBL_SHIFT - 16);
		
		if (deltaAC.x == deltaAB.x)
			return;
		
		if (deltaAC.x < deltaAB.x)
		{
			RDir = 1;
			if (A->Vx->y < 0)
				valAB.x -= A->Vx->y*deltaAB.x;
		}
		else
		{
			RDir = 0;
			
			CHK_MULT(dhAB,InvdyAB);
			CHK_MULT(dzAB,InvdyAB);
			
			deltaAB.h = (dhAB * InvdyAB) >> (INVTBL_SHIFT - 12);
			deltaAB.u = ((duAB >> (8 + 2)) * InvdyAB) >> (INVTBL_SHIFT - 8 - 2);
			deltaAB.v = ((dvAB >> (8 + 2)) * InvdyAB) >> (INVTBL_SHIFT - 8 - 2);			
			deltaAB.z = (dzAB * InvdyAB) >> (INVTBL_SHIFT - 16);
			
			
			// initial values
			valAB.h = valAC.h;
			valAB.u = valAC.u;
			valAB.v = valAC.v;
			
			valAB.z = valAC.z;
			
			if (A->Vx->y < 0)      
				valAB.Substact(A->Vx->y,deltaAB);
		}
	}
	
	const int Yend = ::LastLine(C, m_dispY);
	
	if (YDraw < 0)                                 // jump into screen
	{
		valAC.Substact(YDraw,deltaAC);    
		YDraw = 0;
	}
	
	if (RDir)                                      // face oriented on right
	{
		while (YDraw < Yend)
		{
			
			const int x1 = valAC.x >> 16;
			
			int x2;
			if (YDraw < B->Vx->y)
			{
				//x2 = valAB.x >> 16;
				x2 = DownShift16(valAB.x);
				valAB.x += deltaAB.x;                         // up triangle part
			}
			else
			{
				//x2 = valBC.x >> 16;
				x2 = DownShift16(valBC.x);
				valBC.x += deltaBC.x;                         // down triangle part
			}
			
			if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
				Lib3D::DrawScanLineZCorrectedCar(*this,YDraw,valAC,x2);
			
			++YDraw;
			valAC.AddDelta(deltaAC);
		}
	}
	else
	{
		while (YDraw < Yend)
		{
			int x1, x2;
			if (YDraw < B->Vx->y)
			{
				//x1 = valAB.x >> 16;
				//x2 = valAC.x >> 16;
				x1 = DownShift16(valAB.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineZCorrectedCar(*this,YDraw,valAB,x2);
				
				valAB.AddDelta(deltaAB);// top triangle part        
			}
			else
			{
				//x1 = valBC.x >> 16;
				//x2 = valAC.x >> 16;
				x1 = DownShift16(valBC.x);
				x2 = DownShift16(valAC.x);
				
				if ((x1 < CLIPXEND) && (x2 > CLIPXMIN) && (x1 < x2))
					Lib3D::DrawScanLineZCorrectedCar(*this,YDraw,valBC,x2);
				
				// bottom triangle part
				valBC.AddDelta(deltaBC);
			}
			++YDraw;
			valAC.x += deltaAC.x;
		}
	}
}

// ---------------------------------------------------------------------------
//InvRhTable
// ---------------------------------------------------------------------------

#if !USE_TABLE
int CBoard3D::InvRhTable(int i)
{
	REC_MINMAX(InvRhTable,i);
	
#ifdef WIN_DEBUG
	if(i<0)
	{
		int _setBreakPointHere = 0;
	}
#endif
	
	if( i <= (1<<6) )
		return int(USHRT_MAX);
	else
	{
		const int rh = (1<<22)/i;
		A_ASSERT(rh<=USHRT_MAX);
		return rh;
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int CBoard3D::InverseTable(int i)
{
#ifdef WIN_DEBUG
	REC_MINMAX(InverseTable,i);
	if(i<0)
	{
		int __setBreakPointHere = 0;
	}
	
	if(i> DX_RANGE)
	{
		int __setBreakPointHere = 0;
	}
#endif
	
	if(i==0)
		return 1<<INVTBL_SHIFT;
	else
		return int( (1<<INVTBL_SHIFT) / i); // + 0.5f;
}


#endif	//USE_TABLE

void CBoard3D::DrawBillboard(	const Vector4s& screenSpaceCentre,const TTexture& tex,int i_scale_x, int i_scale_y,
								int fov,const int transparency, int tex_start_x, int tex_end_x)
{
	const int kShift = 8;
	
	// Get the scale at Z:
	const int inv =InvRhTable(screenSpaceCentre.z);	
	
	const int k = (inv*fov + 0x1F) >> (22 - 16); CHK_MULT(inv,fov); // 22 is shifted value of InvRhTable
	
	CHK_MULT(k,i_scale_x);
	CHK_MULT(k,i_scale_y);
	
	const int scale_x = DownShift16(k*i_scale_x);
	const int scale_y = DownShift16(k*i_scale_y);
	
	if (scale_x == 0)
        return;
	if (scale_y == 0)
        return;
	
	// Calculate the actual image bound:
	int startx	= screenSpaceCentre.x - (scale_x>>1);
	int stopx		= screenSpaceCentre.x + (scale_x>>1);
	int starty	= screenSpaceCentre.y - (scale_y>>1);
	int stopy		= screenSpaceCentre.y + (scale_y>>1);
	
	// test for outside Screen:
	if(startx>=m_dispX)
		return;
	
	if(stopx <=0)
		return;
	
	if(starty >= m_dispY)
		return;
	
	if(stopy < 0)
		return;
	
	
	int textureSize_y = tex.SizeY();
	int textureSize_x = tex_end_x - tex_start_x;
	
	const int delta16_x = (textureSize_x << kShift) / scale_x;	// Number of textel per screen pixel, upshited by 16
	const int delta16_y = (textureSize_y << kShift) / scale_y;	// Number of textel per screen pixel, upshited by 16
	
	int u16 = (tex_start_x) <<kShift;
	int v16 = (tex.SizeY()-1) <<kShift;
	
	if(startx<0)
	{
		u16 += (-startx)*delta16_x;
		startx=0;		
	}
	
	if(starty<0)
	{
		v16 -= (-starty)*delta16_y;
		starty=0;
	}
	if(stopx>=m_dispX)
		stopx = m_dispX;
	
	if(stopy>=m_dispY)
		stopy = m_dispY;
	
	//	const int z = screenSpaceCentre.z << 16;
	unsigned short* textureData = tex.Data();
	const int drawMaskX = tex.DrawMaskX();
	const int drawMaskY = tex.DrawMaskY();
	const int vShift = tex.VShift();
	const int	startU = u16;
	
	if (transparency == BILLBOARD_TRANS_NONE)
	{
		// Draw... opaque
		for(int y=starty;y<stopy;++y)
		{
			u16 = startU;
			unsigned short* pixel = m_screen + (y * m_dispX) + startx;
			for(int x = startx;x<stopx;++x)
			{
				const int rv = ((v16 >> kShift) & drawMaskY) << vShift;
				const int ru = (u16 >> kShift) & drawMaskX;
				
				const int index = rv  | ru ;
				
				A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
				
				const unsigned colour = textureData[index];
				if (colour)
					*pixel = colour;

				u16 += delta16_x;
				++pixel;
			}
			v16 -= delta16_y;
		}
	}
	else if (transparency == BILLBOARD_TRANS_MID)
	{
		// Draw... transparent (1 pixel out of 2)
		for(int y=starty;y<stopy;++y)
		{
			u16 = startU;
			unsigned short* pixel = m_screen + (y * m_dispX) + startx;
			
			bool	dither;
			if ((y & 0x1) ^ (((unsigned long)pixel & 0x2)>>1))
				dither = false;
			else
				dither = true;
			
			for(int x = startx;x<stopx;++x)
			{
				if (dither)
				{
					
					const int rv = v16 >> kShift;
					const int ru = u16 >> kShift;
					
					int index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
					
					
					A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
					
					
					const unsigned colour = textureData[index];
					if (colour)
						*pixel = colour;
				}
				dither = !dither;
				u16 += delta16_x;
				++pixel;
			}
			v16 -= delta16_y;
		}
	}
	else if (transparency == BILLBOARD_TRANS_ADD)
	{
		// Draw... transparent add
		for(int y=starty;y<stopy;++y)
		{
			u16 = startU;
			unsigned short* pixel = m_screen + (y * m_dispX) + startx;
			for(int x = startx;x<stopx;++x)
			{
				const int rv = v16 >> kShift;
				const int ru = u16 >> kShift;
				
				int const index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
				
				A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
				
				register unsigned int const colour = textureData[index];
				if (colour)
				{
					*pixel = CLib2D::FastColorAdd(colour, *pixel);
				}
					
				u16 += delta16_x;
				++pixel;
			}
			v16 -= delta16_y;
		}
	}
	else if (transparency == BILLBOARD_TRANS_ALPHA)
	{
		// Draw... alpha add
		for(int y=starty;y<stopy;++y)
		{
			u16 = startU;
			unsigned short* pixel = m_screen + (y * m_dispX) + startx;
			for(int x = startx;x<stopx;++x)
			{
				const int rv = v16 >> kShift;
				const int ru = u16 >> kShift;
				
				int const index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
				
				A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
				
				register unsigned int const colour = textureData[index];
				if (colour)
				{
					//*pixel = CLib2D::FastColorAdd(colour, *pixel);
					*pixel = colour; //CLib2D::MixColor(colour, *pixel, 0xF);
//Jogy_ Check this
				}
					
				u16 += delta16_x;
				++pixel;
			}
			v16 -= delta16_y;
		}
	}
}

// ---------------------------------------------------------------------------
//
// Transparency is from 0 to 255. 0 is opaque, 255 is fully transparent
// ---------------------------------------------------------------------------
void CBoard3D::DrawBillboard(const Vector4s& screenSpaceCentre,const TTexture& tex,int i_scale,int fov,const int transparency, const int alpha)
{
#if WIN_DEBUG
	static int _nbrCall = 0;
	_nbrCall++;
#endif

	const int kShift = 8;
	
	//rax A_ASSERT(tex.SizeX()==tex.SizeY());
	
	// Get the scale at Z:
	const int inv =InvRhTable(screenSpaceCentre.z);	
	const int k = (inv*fov + 0x1F) >> (22 - 16); CHK_MULT(inv,fov); // 22 is shifted value of InvRhTable
	
	CHK_MULT(k,i_scale);
	
	const int xShift = tex.XShift();
	const int yShift = tex.YShift();
	const int scaleX = DownShift16(k * i_scale);	
	const int scaleY = (xShift > yShift) ? (scaleX >> (xShift - yShift)): (scaleX << (yShift - xShift));
	
	if (scaleX == 0 || scaleY == 0)
        return;
	
	// Calculate the actual image bound:
	int startx	= screenSpaceCentre.x - (scaleX >> 1);

	// test for outside Screen:
	if(startx >= m_dispX)
		return;

	int stopx	= screenSpaceCentre.x + (scaleX >> 1);

	if(stopx <= 0)
		return;

	int starty	= screenSpaceCentre.y - (scaleY >> 1);

	if(starty >= m_dispY)
		return;

	int stopy	= screenSpaceCentre.y + (scaleY >> 1);
	
	if(stopy < 0)
		return;
	
	int textureSizeX = tex.SizeX();
	int textureSizeY = tex.SizeY();
	
	const int delta16X = (textureSizeX << kShift) / scaleX;	// Number of textel per screen pixel, upshited by 16
	const int delta16Y = (textureSizeY << kShift) / scaleY;	// Number of textel per screen pixel, upshited by 16
	register int u16 = 0;
	register int v16 = (textureSizeY - 1) << kShift;
	
	if(startx < 0)
	{
		u16 = (-startx) * delta16X;
		startx=0;		
	}
	
	if(starty < 0)
	{
		v16 -= (-starty) * delta16Y;
		starty=0;
	}

	if(stopx >= m_dispX)
		stopx = m_dispX;
	
	if(stopy >= m_dispY)
		stopy = m_dispY;
	
	unsigned short*	textureData = tex.Data();
	const int drawMaskX = tex.DrawMaskX();
	const int drawMaskY = tex.DrawMaskY();
	const int vShift = tex.VShift();
	int	startU = u16;
	
	switch (transparency)
	{
		case BILLBOARD_TRANS_NONE:
		{
			// Draw... opaque
			unsigned short* screen = m_screen + (starty * m_dispX);
			for(int y = starty; y < stopy; ++y)
			{
				u16 = startU;
				unsigned short *pixel = screen + startx;
				for(int x = startx; x < stopx; ++x)
				{
					const int rv = v16 >> kShift;
					const int ru = u16 >> kShift;
					
					int index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
					A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
					const unsigned colour = textureData[index];
					if (colour)
						*pixel = colour;

					u16 += delta16X;
					++pixel;
				}
				v16 -= delta16Y;
				screen += m_dispX;
			}
			break;
		}
		case BILLBOARD_TRANS_MID:	// never used
		{
			// Draw... transparent (1 pixel out of 2)
			unsigned short* screen = m_screen + (starty * m_dispX);
			for(int y = starty; y < stopy; ++y)
			{
				u16 = startU;
				register unsigned short* pixel = screen + startx;
				
				bool	dither;
				if ((y & 0x1) ^ (((unsigned long)pixel & 0x2)>>1))
					dither = false;
				else
					dither = true;
				
				for(int x = startx;x<stopx;++x)
				{
					if (dither)
					{
						
						const int rv = v16 >> kShift;
						const int ru = u16 >> kShift;
						
						int index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
						A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));						
						
						const unsigned colour = textureData[index];
						if (!(colour & 0xF000))
							*pixel = colour;
					}
					dither = !dither;
					u16 += delta16X;
					++pixel;
				}
				v16 -= delta16Y;
				screen += m_dispX;
			}
			break;
		}
		case BILLBOARD_TRANS_ADD:
		{
			// Draw... transparent add
			unsigned short* screen = m_screen + (starty * m_dispX);
			for(int y = starty; y < stopy; ++y)
			{
				u16 = startU;
				register unsigned short* pixel = screen + startx;
				for(int x = startx; x < stopx; ++x)
				{
					const int rv = v16 >> kShift;
					const int ru = u16 >> kShift;
					
					int index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
					A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
					
					register unsigned int const colour = textureData[index];
					if (colour)
					{
						*pixel = CLib2D::FastColorAdd(colour, *pixel);
					}

					u16 += delta16X;
					++pixel;
				}
				v16 -= delta16Y;
				screen += m_dispX;
			}
			break;
		}
#if USE_STENCIL_BUFFER
		case BILLBOARD_TRANS_STENCIL:
		{
			// Draw... transparent add
			register unsigned short* screen = m_screen + (starty * m_dispX);
			for(int y = starty; y < stopy; ++y)
			{
				u16 = startU;
				register unsigned short* pixel = screen + startx;

				TPixel*			stencilBuffer	= m_stencilBuffer	+ (y * m_dispX) + startx;
				unsigned char*	stencilAlpha	= m_stencilAlpha	+ (y * m_dispX) + startx;
				for(int x = startx;x<stopx;++x)
				{
					const int rv = v16 >> kShift;
					const int ru = u16 >> kShift;
					
					int index = ((rv & drawMaskY) << vShift) | (ru & drawMaskX);
					A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));
					
					register unsigned int const colour = textureData[index];
					if (colour)
					{
						*stencilBuffer = CLib2D::FastColorAdd(colour, *stencilBuffer);
						if(*stencilAlpha < 0xE)
							(*stencilAlpha)++;

						m_stencilBeginX = min(m_stencilBeginX,	x);
						m_stencilBeginY = min(m_stencilBeginY,	y);
						m_stencilEndX	= max(m_stencilEndX,	x);
						m_stencilEndY	= max(m_stencilEndY,	y);
					}

					u16 += delta16X;
					++pixel;
					++stencilBuffer;
					++stencilAlpha;
				}
				v16 -= delta16Y;
				screen += m_dispX;
			}
			break;
		}
#endif // USE_STENCIL_BUFFER
		case BILLBOARD_TRANS_ALPHA:
		{
			// Draw... alpha add
			unsigned short* screen = m_screen + (starty * m_dispX);
			for(int y = starty; y < stopy; ++y)
			{
				u16 = startU;
				register unsigned short* pixel = screen + startx;
				for(int x = startx;x<stopx;++x)
				{
					const int rv = v16 >> kShift;
					const int ru = u16 >> kShift;
					
					int index = ((rv & drawMaskX) << vShift) | (ru & drawMaskY);
					A_ASSERT(index >=0 && index < (tex.SizeX() * tex.SizeY()));					
					
					register unsigned int const colour = textureData[index];
					if (colour)
					{
						*pixel = CLib2D::MixColor(CLib2D::FastColorAdd(colour, *pixel), *pixel, alpha);
					}
						
					u16 += delta16X;
					++pixel;
				}
				v16 -= delta16Y;
				screen += m_dispX;
			}
			break;
		}		
	}
}

void CBoard3D::DrawLine(Vector4s v1,Vector4s v2,int c1,int c2,const CColor& colorIndex, bool yesZ){
	
	int dx=v2.x-v1.x;
	int dy=v2.y-v1.y;
	
	const int ds=0xf;
	int da=((c2&0xf000)>>12)-((c1&0xf000)>>12);
	
	
	int a;
	int as=0;
	int aa=0;
	
	int rx;
	int ry;
	int rs=0;
	int ra=((c1&0xf000)>>12);
	
	int inc,incpix,incalpha;
	
	Vector4s interv;
	int interi;
	
	// Initialise screen pointer
	unsigned short * pix;
	
	incalpha=(da>=0)?(1):(-1);
	da=Lib3D::Abs(da);
	
	// DX > DY
	dx=Lib3D::Abs(dx);
	dy=Lib3D::Abs(dy);
	
	if(Lib3D::Abs(dx)>Lib3D::Abs(dy)){
		a=dx>>1;
		
		// Swap vectors if the x are inverted
		if(v2.x<v1.x){ 
			interv=v1;
			v1=v2;
			v2=interv;
			interi=c1;
			c1=c2;
			c2=interi;
		}
		
		rx=v1.x;
		ry=v1.y;
		pix=(unsigned short*)m_screen+rx+ry*m_dispX;
		
		
		// Is the slope positive or negative ?
		inc=(v2.y>v1.y)?(1):(-1);
		incpix=(inc==1)?m_dispX:-m_dispX;
		
		while(rx<v2.x&&rx<m_dispX){
			
			a+=dy;
			if(a>dx){		
				a-=dx;
				ry+=inc;
				pix+=incpix; 
			}
			as+=ds;
			if(as>dx){
				as-=dx;
				rs++;
			}
			if(rs>0xf)rs=0xf;
			
			aa+=da;
			if(aa>dx){
				aa-=dx;
				ra+=incalpha;
			}
			if(ra>0xf)ra=0xf;
			
			
			if(rx>0&&ry>0&&ry<m_dispY)
			{
				*pix = c1; // TBD
			}
			pix++;			
			rx++;
		}
	}else{
		a=dy>>1;
		
		// Swap vectors if the y are inverted
		if(v2.y<v1.y)
		{ 
			interv=v1;
			v1=v2;
			v2=interv;
			interi=c1;
			c1=c2;
			c2=interi;
		}
		
		rx=v1.x;
		ry=v1.y;
		pix=(unsigned short*)m_screen+rx+ry*m_dispX;
		
		
		// Is the slope positive or negative ?
		inc=(v2.x>v1.x)?(1):(-1);
		
		while(ry<v2.y&&ry<m_dispY){
			
			a+=dx;
			if(a>dy){		
				a-=dy;
				rx+=inc;
				pix+=inc; 
			}
			
			as+=ds;
			if(as>dy){
				as-=dy;
				rs++;
			}
			if(rs>0xf)rs=0xf;
			
			aa+=da;
			if(aa>dy){
				aa-=dy;
				ra+=incalpha;
			}
			if(ra>0xf)ra=0xf;
			
			
			if(rx>0&&ry>0&&rx<m_dispX)
			{
				*pix = c1; // TBD
			}
			pix+=m_dispX;			
			ry++;
		}
	}
}

#if USE_Z_BUFFER
void CBoard3D::ClearZBuffer()
{
	::memset(m_zBuffer, 0, sizeof(int) * m_dispX * m_dispY);
	//m_zBufferDir = -m_zBufferDir;
}
#endif // USE_Z_BUFFER
