
#ifdef USE_OGL

#include "Lib3DGL.h"
//#include "../Gapi.h"
#include "../HG/HighGear.h"
#include "../Lib3D2/IMath.h"
#include "../Lib3D2/FSqrt.h"

#include "../HG/shadow.h"

//global
Lib3DGL* g_lib3DGL = NULL;
extern CGapi Gapi;

u8* Lib3DGL::s_decodeBuffer = NULL;

#ifdef DEBUG_TEX_MEM
int g_TexTotalTexSize = 0;
#endif

Lib3DGL::Lib3DGL()
{
	m_nNum2DQuads	=	0;
	
	//Init 2D rendering related data
	m_pVtxBuffer2D		=	NULL;
	m_pClrBuffer2D		=	NULL;
	m_pTexBuffer2D		=	NULL;
	m_pFlags			=	NULL;
	
	color = 0xFFFFFFFF;
}

Lib3DGL::~Lib3DGL()
{
}


void Lib3DGL::fillRect(int x, int y, int w, int h, u32 color)
{
	//leave the last quad for paint2DModuleImmediateMode
	if( w <= 0 || h <= 0)
		return;

	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - 1 ));

	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID +		 ( m_nNum2DQuads);
	u8  *	pFlags	=	m_pFlags +		 ( m_nNum2DQuads);
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	color & 0xFF;

	//no texture
	*pTexID = 0;
	*pFlags = 0;
	
	*pVtx++	=	x;		*pVtx++	=	y;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x + w;	*pVtx++	=	y;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x + w;	*pVtx++	=	y + h;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x;		*pVtx++	=	y + h;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	m_nNum2DQuads++;
}

void Lib3DGL::fillRectImmediateMode( int x, int y, int w, int h, u32 color)
{
	//store vertex info in the last quad position [MAX_2D_QUADS - 1] of the storage arrays
	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * ( MAX_2D_QUADS -1 ) );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * ( MAX_2D_QUADS -1 ) );
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	color & 0xFF;

	
	*pVtx++	=	x;		*pVtx++	=	y;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x + w;	*pVtx++	=	y;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x + w;	*pVtx++	=	y + h;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x;		*pVtx++	=	y + h;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;	
	

	Begin2DRendering();		
		
	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_COLOR_ARRAY);	
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	::glDisable(GL_TEXTURE_2D);			
	
	::glVertexPointer(2, GL_SHORT, 0, m_pVtxBuffer2D);
	::glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_pClrBuffer2D);
		
	::glDrawElements( GL_TRIANGLES, TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI, GL_UNSIGNED_SHORT, m_pIndices + ( (MAX_2D_QUADS -1) * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI ) );
	
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);			
	::glDisableClientState(GL_COLOR_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);			
	
	End2DRendering();

}

void Lib3DGL::drawLine( int x_start, int y_start, int x_end, int y_end, u32 color )
{
	x_start++;
	y_start++;

	//leave the last quad for paint2DModuleImmediateMode, fillRectImmediateMode
	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - 1 ) );

	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID +		 ( m_nNum2DQuads);
	u8	*	pFlags  =	m_pFlags       + ( m_nNum2DQuads );
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	color & 0xFF;

	//no texture
	*pTexID = 0;
	
	*pVtx++	=	x_start;*pVtx++	=	y_start;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x_end;	*pVtx++	=	y_end;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;


	*pFlags = FLAG_IS_LINE;

	m_nNum2DQuads++;
}

void Lib3DGL::fillArc(int x, int y, int radius, int a1, int a2, u32 color)
{
	a1 &= ANGLEMASK;
	a2 &= ANGLEMASK;
	
	if (a2 <  a1)
		a2 += ANGLE2PI;
	
	int precision = (a2 - a1) / (ANGLE2PI / 64);
	
	//leave the last quad for paint2DModuleImmediateMode
	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - precision ) );
	
	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID +		 ( m_nNum2DQuads);
	u8  *	pFlags	=	m_pFlags +		 ( m_nNum2DQuads);
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	color & 0xFF;

	int ang1 = a2, ang2;
	for (int q = precision - 1; q >= 0; q--)
	{
		ang2 = a1 + q * (a2 - a1) / precision;
		
		*pVtx++	=	x;
		*pVtx++	=	y;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		
		*pVtx++	=	x;
		*pVtx++	=	y;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

		*pVtx++	=	x + ((radius * Cosinus(ang1)) >> COS_SIN_SHIFT);
		*pVtx++	=	y + ((radius * Sinus(ang1)) >> COS_SIN_SHIFT);
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		
		*pVtx++	=	x + ((radius * Cosinus(ang2)) >> COS_SIN_SHIFT);
		*pVtx++	=	y + ((radius * Sinus(ang2)) >> COS_SIN_SHIFT);
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		
		ang1 = ang2;

		*pTexID++ = 0;
		*pFlags++ = 0;
		m_nNum2DQuads++;
	}
}

void Lib3DGL::drawArc(int x, int y, int radius, int a1, int a2, u32 color)
{
//	return;
	a1 &= ANGLEMASK;
	a2 &= ANGLEMASK;
	
	if (a2 <  a1)
		a2 += ANGLE2PI;
	
	int precision = (a2 - a1) / (ANGLE2PI / 32);
	
	//leave the last quad for paint2DModuleImmediateMode
	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - precision ) );
	
	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID +		 ( m_nNum2DQuads);
	u8  *	pFlags	=	m_pFlags +		 ( m_nNum2DQuads);
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	color & 0xFF;
	
	int ang1 = a2, ang2;
	for (int q = precision - 1; q >= 0; q--)
	{
		ang2 = a1 + q * (a2 - a1) / precision;
		
		*pVtx++	=	x + ((radius * Cosinus(ang1)) >> COS_SIN_SHIFT);
		*pVtx++	=	y + ((radius * Sinus(ang1)) >> COS_SIN_SHIFT);
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		
		*pVtx++	=	x + ((radius * Cosinus(ang2)) >> COS_SIN_SHIFT);
		*pVtx++	=	y + ((radius * Sinus(ang2)) >> COS_SIN_SHIFT);
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

		pVtx += 4;
		pClr += 8;
		
		ang1 = ang2;

		*pTexID++ = 0;
		*pFlags++ = FLAG_IS_LINE;
		m_nNum2DQuads++;
	}
}


void Lib3DGL::setClip(int x, int y, int w, int h)
{
	//leave the last quad for paint2DModuleImmediateMode, fillRectImmediateMode
	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - 1 ) );

	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID	   + ( m_nNum2DQuads );
	u8	*	pFlags  =	m_pFlags       + ( m_nNum2DQuads );

	//no texture
	*pTexID = 0;
	
	*pVtx++	=	x;		*pVtx++	=	y;
	*pVtx++	=	w+1;	*pVtx++	=	h+1;

	*pFlags = FLAG_SET_CLIP;

	m_nNum2DQuads++;
}

void Lib3DGL::clearClip()
{
	//leave the last quad for paint2DModuleImmediateMode, fillRectImmediateMode
	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - 1 ) );

	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID +		 ( m_nNum2DQuads);
	u8	*	pFlags  =	m_pFlags       + ( m_nNum2DQuads );

	//no texture
	*pTexID = 0;

	*pFlags = FLAG_CLEAR_CLIP;

	m_nNum2DQuads++;
}

void Lib3DGL::paint2DModule(int x, int y, int w, int h, u32 texId, const f32 uv[], int flags, int angle, int rotCenterX, int rotCenterY)
{
	//leave the last quad for paint2DModuleImmediateMode, fillRectImmediateMode
	A_ASSERT( m_nNum2DQuads < ( MAX_2D_QUADS - 1 ) );

	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	f32 *   pTex	=   m_pTexBuffer2D + ( TEXCOOR_SIZE * VTX_COUNT_PER_QUAD * m_nNum2DQuads );
	u32 *	pTexID	=	m_pTexID       + ( m_nNum2DQuads );
	u8	*	pFlags  =	m_pFlags       + ( m_nNum2DQuads );

	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	 color & 0xFF;

	*pFlags = 0;
	if (color != 0xFFFFFFFF)
		*pFlags |= FLAG_HAS_COLOR;

	*pFlags |= (flags & FLAG_USE_ADDITIVE_BLEND);

	*pTexID = texId;
	
	if (angle)
	{
		angle &= ANGLEMASK;
		
		int cos = Cosinus(angle);
		int sin = Sinus(angle);
		int tmp, tx, ty;
		
		//uv  (u0 v0)  (u1 v1)
		tx = x - rotCenterX;
		ty = y - rotCenterY;
		
		// x
		tmp = tx * cos - ty * sin;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterX + tmp;
		
		// y
		tmp = tx * sin + ty * cos;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterY + tmp;
		
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[0];	*pTex++	=	uv[1]; //u0 v0
		
		
		tx = x + w - rotCenterX;
		ty = y - rotCenterY;
		
		// x
		tmp = tx * cos - ty * sin;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterX + tmp;
		
		// y
		tmp = tx * sin + ty * cos;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterY + tmp;
		
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[2];	*pTex++	=	uv[1]; //u1 v0
		
		
		tx = x + w - rotCenterX;
		ty = y + h - rotCenterY;
		
		// x
		tmp = tx * cos - ty * sin;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterX + tmp;
		
		// y
		tmp = tx * sin + ty * cos;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterY + tmp;
		
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[2];	*pTex++	=	uv[3];  //u1 v1
		
		
		tx = x - rotCenterX;
		ty = y + h - rotCenterY;
		
		// x
		tmp = tx * cos - ty * sin;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterX + tmp;
		
		// y
		tmp = tx * sin + ty * cos;
		tmp = (tmp >> COS_SIN_SHIFT) + ((tmp & (1 << (COS_SIN_SHIFT - 1))) != 0 ? 1 : 0);
		*pVtx++	=	rotCenterY + tmp;
		
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[0];	*pTex++	=	uv[3]; //u0 v1
	}
	else
	{
		//uv  (u0 v0)  (u1 v1)
		*pVtx++	=	x;		*pVtx++	=	y;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[0];	*pTex++	=	uv[1]; //u0 v0		

		*pVtx++	=	x + w;	*pVtx++	=	y;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[2];	*pTex++	=	uv[1]; //u1 v0

		*pVtx++	=	x + w;	*pVtx++	=	y + h;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[2];	*pTex++	=	uv[3];  //u1 v1

		*pVtx++	=	x;		*pVtx++	=	y + h;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pTex++	=	uv[0];	*pTex++	=	uv[3]; //u0 v1
	}

	m_nNum2DQuads++;
}


void Lib3DGL::paint2DModuleImmediateMode( int x, int y, int w, int h, u32 texId, const f32 uv[], int flags )
{
	//store vertex info in the last quad position [MAX_2D_QUADS - 1] of the storage arrays
	s16	*	pVtx	=	m_pVtxBuffer2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * ( MAX_2D_QUADS -1 ) );
	u8	*	pClr	=	m_pClrBuffer2D + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * ( MAX_2D_QUADS -1 ) );
	f32 *   pTex	=   m_pTexBuffer2D + ( TEXCOOR_SIZE * VTX_COUNT_PER_QUAD * ( MAX_2D_QUADS -1 ) );
	u32  *	pTexID	=	m_pTexID +		 ( MAX_2D_QUADS -1 );	

	int idx = MAX_2D_QUADS - 1;
	m_pIndicesStrip[0] = idx * VTX_COUNT_PER_QUAD + 0;
	m_pIndicesStrip[1] = idx * VTX_COUNT_PER_QUAD + 1;
	m_pIndicesStrip[2] = idx * VTX_COUNT_PER_QUAD + 3;
	m_pIndicesStrip[3] = idx * VTX_COUNT_PER_QUAD + 2;
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	 color & 0xFF;

	*pTexID = texId;
	//uv  (u0 v0)  (u1 v1)
	*pVtx++	=	x;		*pVtx++	=	y;
	*pTex++	=	uv[0];	*pTex++	=	uv[1]; //u0 v0
	

	*pVtx++	=	x + w;	*pVtx++	=	y;
	*pTex++	=	uv[2];	*pTex++	=	uv[1]; //u1 v0


	*pVtx++	=	x + w;	*pVtx++	=	y + h;
	*pTex++	=	uv[2];	*pTex++	=	uv[3];  //u1 v1


	*pVtx++	=	x;		*pVtx++	=	y + h;
	*pTex++	=	uv[0];	*pTex++	=	uv[3]; //u0 v1

	bool hasColor = (color != 0xFFFFFFFF);
	
	if(hasColor)
	{
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
		*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	}

	Begin2DRendering();		
		
	//Render current 2D primitives		
	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if(flags == 1 && !hasColor)
	{
		glDisable(GL_BLEND);
	}

	::glEnable(GL_TEXTURE_2D);				
	
	::glVertexPointer(2, GL_SHORT, 0, m_pVtxBuffer2D);
	if(hasColor)
	{
		::glEnableClientState(GL_COLOR_ARRAY);		
		::glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_pClrBuffer2D);		
	}
	::glTexCoordPointer(2, GL_FLOAT, 0, m_pTexBuffer2D);
		
	::glBindTexture(GL_TEXTURE_2D, m_pTexID[MAX_2D_QUADS -1]);
	
	if(hasColor)
	{
		::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		
	}
	else
	{
		::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	
	}
		
	::glDrawElements( GL_TRIANGLE_STRIP, VTX_COUNT_PER_QUAD, GL_UNSIGNED_SHORT, m_pIndicesStrip );
	
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);		

	if(hasColor)
	{
		::glDisableClientState(GL_COLOR_ARRAY);
	}

	::glDisableClientState(GL_VERTEX_ARRAY);			
	
	End2DRendering();
}

void Lib3DGL::Begin2DRendering()
{	
	int screen_width = CHighGear::GetInstance()->m_dispX;
	int screen_height = CHighGear::GetInstance()->m_dispY;
	
	::glDepthMask(false);	
	::glDisable(GL_DEPTH_TEST);		
	
	//::glEnable(GL_ALPHA_TEST);
	//::glAlphaFunc(GL_EQUAL, 1.0f);
	::glDisable(GL_ALPHA_TEST);
	
	::glEnable(GL_BLEND);
	::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	::glDisable(GL_CULL_FACE);

	::glMatrixMode(GL_PROJECTION);
	::glPushMatrix();
	::glLoadIdentity();	

#ifdef IPHONE
	//rotate the normalized window coordinate to fit the aspect ratio choosed from glViewport
	switch (Gapi.mCurrentOrientation)
	{
		case ORIENTATION_LANDSCAPE_90:
			::glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			break;
		
		case ORIENTATION_LANDSCAPE_270:
			::glRotatef(270.0f, 0.0f, 0.0f, 1.0f);
			break;
	}
#endif

	::glOrthof( 0, screen_width, screen_height, 0, -1.0f, 1.0f );
	
	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadIdentity();	
}

void Lib3DGL::End2DRendering()
{
	::glDisable(GL_ALPHA_TEST);
	::glDisable(GL_BLEND);

	::glMatrixMode(GL_PROJECTION);
	::glPopMatrix();

	::glMatrixMode(GL_MODELVIEW);
	::glPopMatrix();
}


void Lib3DGL::Flush2D()
{
	if(m_nNum2DQuads <= 0)
	{
		return;
	}
	//GLDisableFog();

	Begin2DRendering();		
		
	//Render current 2D primitives		
	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_COLOR_ARRAY);	
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	::glEnable(GL_TEXTURE_2D);
	bool bTextureEnabled = true;
	
	::glVertexPointer(2, GL_SHORT, 0, m_pVtxBuffer2D);
	::glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_pClrBuffer2D);		
	::glTexCoordPointer(2, GL_FLOAT, 0, m_pTexBuffer2D);
	
	u32 currTex = -1;
	int currFlags = 0;
	
	//use these to group quads with same texture/ fillRects / drawRects
	u32 nGroupStartIdx = 0; //first vertex
	u32 nGroupCountIdx = 0;
	bool bDrawGroup = false;
	bool bColorsEnable = false;
	bool bIsAdditiveBlendEnable = false;

	for(int i = 0; i < m_nNum2DQuads; i++)
	{
		if (currTex != m_pTexID[i] || currFlags != m_pFlags[i])
		{
			if( nGroupCountIdx > 0 )
				bDrawGroup = true;				
			else  //add first quad to the group
				nGroupCountIdx  = 1;

			currTex = m_pTexID[i];
			currFlags = m_pFlags[i];		
		}
		else if(currFlags & (/*FLAG_IS_LINE | */FLAG_SET_CLIP | FLAG_CLEAR_CLIP))
		//draw line separate ... these are added in 4-vertex slots and a continuos vertex index reading will be corrupted
		{
			bDrawGroup = true;
		}
		else //same appearance ... add to group
		{
			//add to group
			nGroupCountIdx++;
		}
		
		if( bDrawGroup )		
		{
			if(m_pFlags[nGroupStartIdx] & (FLAG_SET_CLIP | FLAG_CLEAR_CLIP))
			{
				if (m_pFlags[nGroupStartIdx] & FLAG_SET_CLIP)
				{
					::glEnable(GL_SCISSOR_TEST);
					
					int x = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 0];
					int y = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 1];
					int w = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 2];
					int h = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 3];
					
					//::glScissor(x, CHighGear::GetInstance()->m_dispY - y - h, w, h);
					GLScissor(x, y, w, h);
				}
				
				if (m_pFlags[nGroupStartIdx] & FLAG_CLEAR_CLIP)
				{
					//::glScissor(0, 0, CHighGear::GetInstance()->m_dispX, CHighGear::GetInstance()->m_dispY);
					GLScissor(0, 0, CHighGear::GetInstance()->m_dispX, CHighGear::GetInstance()->m_dispY);
					::glDisable(GL_SCISSOR_TEST);
				}
			}
			else
			{
				int mode = GL_TRIANGLES;
				int primitiveCount = TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI;
				u16 *pIndices = m_pIndices;

				if (m_pTexID[nGroupStartIdx] > 0)
				{
					if(!bTextureEnabled)
					{
						::glEnable(GL_TEXTURE_2D);
						bTextureEnabled = true;
					}

					::glBindTexture(GL_TEXTURE_2D, m_pTexID[nGroupStartIdx]);
					//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
					//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
					//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					if (m_pFlags[nGroupStartIdx] & FLAG_HAS_COLOR)
					{
						if (!bColorsEnable)
						{
							::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
							::glEnableClientState(GL_COLOR_ARRAY);

							bColorsEnable = true;
						}
					}
					else
					{
						if (bColorsEnable)
						{
							::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
							::glDisableClientState(GL_COLOR_ARRAY);

							bColorsEnable = false;
						}
					}
				}
				else
				{
					if(bTextureEnabled)
					{
						::glDisable(GL_TEXTURE_2D);
						bTextureEnabled = false;
					}

					::glEnableClientState(GL_COLOR_ARRAY);

					if (m_pFlags[nGroupStartIdx] & FLAG_IS_LINE)
					{
						mode = GL_LINES;
						primitiveCount = 2;
						pIndices = m_pLineIndices;
					}
				}

				//test if additive blending is enable or should be enabled (lensflare)		
				if( m_pFlags[nGroupStartIdx] & FLAG_USE_ADDITIVE_BLEND )			
				{
					if(!bIsAdditiveBlendEnable )
					{
						::glBlendFunc(GL_ONE, GL_ONE);
						bIsAdditiveBlendEnable = true;
					}
				}
				else if( bIsAdditiveBlendEnable )
				{
					::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					bIsAdditiveBlendEnable = false;
				}
				

				::glDrawElements( mode, nGroupCountIdx * primitiveCount, GL_UNSIGNED_SHORT, pIndices + ( nGroupStartIdx * primitiveCount ) );
			}

			//reset
			nGroupStartIdx = i;			
			nGroupCountIdx = 1; //add this index in group
			bDrawGroup = false;
		}
	} //for

	{//draw last group

		if(m_pFlags[nGroupStartIdx] & (FLAG_SET_CLIP | FLAG_CLEAR_CLIP))
		{
			if (m_pFlags[nGroupStartIdx] & FLAG_SET_CLIP)
			{
				::glEnable(GL_SCISSOR_TEST);
				
				int x = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 0];
				int y = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 1];
				int w = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 2];
				int h = m_pVtxBuffer2D[VTX_SIZE * VTX_COUNT_PER_QUAD * nGroupStartIdx + 3];
				
				//::glScissor(x, CHighGear::GetInstance()->m_dispY - y - h, w, h);
				GLScissor(x, y, w, h);
			}
			
			if (m_pFlags[nGroupStartIdx] & FLAG_CLEAR_CLIP)
			{
				//::glScissor(0, 0, CHighGear::GetInstance()->m_dispX, CHighGear::GetInstance()->m_dispY);
				GLScissor(0, 0, CHighGear::GetInstance()->m_dispX, CHighGear::GetInstance()->m_dispY);
				::glDisable(GL_SCISSOR_TEST);
			}
		}
		else
		{
			int mode = GL_TRIANGLES;
			int primitiveCount = TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI;
			u16 * pIndices = m_pIndices;

			if (m_pTexID[nGroupStartIdx] > 0)
			{
				if(!bTextureEnabled)
				{
					::glEnable(GL_TEXTURE_2D);
					bTextureEnabled = true;
				}

				::glBindTexture(GL_TEXTURE_2D, m_pTexID[nGroupStartIdx]);
				//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
				//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);
				//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				//::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				if (m_pFlags[nGroupStartIdx] & FLAG_HAS_COLOR)
				{
					if (!bColorsEnable)
					{
						::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
						::glEnableClientState(GL_COLOR_ARRAY);
					}
				}
				else
				{
					if (bColorsEnable)
					{
						::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
						::glDisableClientState(GL_COLOR_ARRAY);
					}
				}
			}
			else
			{
				if(bTextureEnabled)
				{
					::glDisable(GL_TEXTURE_2D);
					bTextureEnabled = false;
				}

				::glEnableClientState(GL_COLOR_ARRAY);

				if (m_pFlags[nGroupStartIdx] & FLAG_IS_LINE)
				{
					mode = GL_LINES;
					primitiveCount = 2;
					pIndices = m_pLineIndices;
				}
			}

			//test if additive blending is enable or should be enabled (lensflare)		
			if( m_pFlags[nGroupStartIdx] & FLAG_USE_ADDITIVE_BLEND )			
			{
				if(!bIsAdditiveBlendEnable )
				{
					::glBlendFunc(GL_ONE, GL_ONE);
					bIsAdditiveBlendEnable = true;
				}
			}
			else if( bIsAdditiveBlendEnable )
			{
				::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				bIsAdditiveBlendEnable = false;
			}

			::glDrawElements( mode, nGroupCountIdx * primitiveCount, GL_UNSIGNED_SHORT, pIndices + ( nGroupStartIdx * primitiveCount ) );

		}
	}//end draw last group


	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);			
	::glDisableClientState(GL_COLOR_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);

	::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

	End2DRendering();

	m_nNum2DQuads	=	0;
}


void Lib3DGL::Init3D()
{
	m_pVtxBuffer2D	=	NEW s16[MAX_2D_QUADS * VTX_SIZE * VTX_COUNT_PER_QUAD];
	m_pClrBuffer2D	=	NEW u8[MAX_2D_QUADS * COLOR_SIZE * VTX_COUNT_PER_QUAD]; 	
	m_pTexBuffer2D  =	NEW f32[MAX_2D_QUADS * TEXCOOR_SIZE * VTX_COUNT_PER_QUAD]; 
	m_pTexID		=	NEW u32[MAX_2D_QUADS];
	m_pFlags		=	NEW u8[MAX_2D_QUADS]; 
	m_pIndices		=	NEW u16[MAX_2D_QUADS * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI]; //2 triangles per quad
	m_pIndicesStrip =	NEW u16[4];
	m_pLineIndices	=   NEW u16[MAX_2D_QUADS * VTX_COUNT_PER_LINE]; //map line in a quad ... usefull to batch multiple lines
	

	//create indices	
	for( int i = 0; i < MAX_2D_QUADS; i++ )
	{
		//first triangle
		m_pIndices[i * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI ] = i*VTX_COUNT_PER_QUAD + 0;
		m_pIndices[i * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI + 1] = i*VTX_COUNT_PER_QUAD + 1;
		m_pIndices[i * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI + 2] = i*VTX_COUNT_PER_QUAD + 2;
		
		//second triangle
		m_pIndices[i * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI + 3] = i*VTX_COUNT_PER_QUAD + 0;
		m_pIndices[i * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI + 4] = i*VTX_COUNT_PER_QUAD + 2;
		m_pIndices[i * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI + 5] = i*VTX_COUNT_PER_QUAD + 3;

		m_pLineIndices[i * VTX_COUNT_PER_LINE + 0] = i*VTX_COUNT_PER_QUAD + 0;
		m_pLineIndices[i * VTX_COUNT_PER_LINE + 1] = i*VTX_COUNT_PER_QUAD + 1;
	}

	s_decodeBuffer = NEW u8[MAX_TEX_WIDTH * MAX_TEX_HEIGHT * MAX_BYTES_PER_COLOR];

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE
	CShadow::createShadowTexFBO();
#endif
}

void Lib3DGL::Clean3D()
{
	SAFE_DELETE_ARRAY(m_pVtxBuffer2D);
	SAFE_DELETE_ARRAY(m_pClrBuffer2D);
	SAFE_DELETE_ARRAY(m_pTexBuffer2D);
	SAFE_DELETE_ARRAY(m_pTexID);
	SAFE_DELETE_ARRAY(m_pFlags);
	SAFE_DELETE_ARRAY(m_pIndices);
	SAFE_DELETE_ARRAY(m_pIndicesStrip);
	SAFE_DELETE_ARRAY(m_pLineIndices);
	SAFE_DELETE_ARRAY(s_decodeBuffer);

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE
	CShadow::destroyShadowTexFBO();
#endif
}


f32 Lib3DGL::s_fogColor[4] = {0.0f};

void Lib3DGL::GLEnableFog(u32 mode, f32 density, f32 start, f32 end, f32 red, f32 green, f32 blue, f32 alpha)
{
	s_fogColor[0] = red;
	s_fogColor[1] = green;
	s_fogColor[2] = blue;
	s_fogColor[3] = alpha;

	::glEnable(GL_FOG);

	::glFogf(GL_FOG_MODE, mode);
	::glFogf(GL_FOG_DENSITY, density);
	::glFogf(GL_FOG_START, start);
	::glFogf(GL_FOG_END, end);
	::glFogfv(GL_FOG_COLOR, s_fogColor);
}

void Lib3DGL::GLDisableFog()
{
	::glDisable(GL_FOG);
}



//create a OpenGL texture and returns the textureGLName
void Lib3DGL::CreateGLTexture(GLuint &glTextureName, 
								u32 width, u32 height, 
								const u8* data, 
								u32 glInternalFormat, u32 glDataFormat, u32 glDataPixelType,
								bool generateMipmap
								)
{
	GLuint	textures[1];

	glEnable(GL_TEXTURE_2D);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures( 1, textures );

	glBindTexture( GL_TEXTURE_2D, textures[0] );

	if(generateMipmap)
		::glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glTexImage2D( GL_TEXTURE_2D, 0, 
				  glInternalFormat,
				  width, height, 
				  0,
				  glDataFormat,
				  glDataPixelType,
				  data
				  );

	if(generateMipmap)
		::glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	
	glTextureName = textures[0];	

	#ifdef DEBUG_TEX_MEM
		debug_out("CREATED texture name: [%d] [%d , %d] [alpha: %d]\n", glTextureName, width, height, glDataFormat == GL_RGB ? 0 : 1);

		if(glDataPixelType == GL_UNSIGNED_SHORT_5_6_5 || glDataPixelType == GL_UNSIGNED_SHORT_5_5_5_1)
			g_TexTotalTexSize += width * height * 2;
		else
			g_TexTotalTexSize += width * height * (glDataFormat == GL_RGB ? 3 : 4);		
		debug_out("TOTAL_SIZE:[%d]\n", (g_TexTotalTexSize / 1024));
	#endif //DEBUG_TEX_MEM
}

//release texture 
//free the video memory allocated for this texture name
void Lib3DGL::ReleaseGLTexture(GLuint glTextureName)
{
	GLuint	textures[1];

	textures[0] = glTextureName;	

	glDeleteTextures( 1, textures );
	
//	debug_out("RELEASE texture name: [%d] \n", glTextureName);	
}


void Lib3DGL::GLScissor(int x, int y, int w, int h)
{
#ifdef IPHONE
	int tmp;

	switch (Gapi.mCurrentOrientation)
	{
		case ORIENTATION_PORTRAIT:
			y = 480 - y - h;
			break;
			
		case ORIENTATION_LANDSCAPE_90:			
			tmp = y;
			y = x;
			x = tmp;
			
			tmp = w;
			w = h;
			h = tmp;			

			break;
			
		case ORIENTATION_LANDSCAPE_270:
			
			tmp = y;
			y = 480 - x - w;
			x = 320 - tmp - h;
			
			tmp = w;
			w = h;
			h = tmp;
			
			break;
	}

#else // IPHONE
	y = CHighGear::GetInstance()->m_dispY - y - h;
#endif // IPHONE

	::glScissor(x, y, w, h);
}

// decode 565 image to 8888 ( if bHasAlphaChannel == true )or 888 ( bHasAlphaChannel == false)
//also the decoded image will have the width (dstPow2Width) and height(dstPow2Height) pow2
void Lib3DGL::Decode565To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u16* src, int srcWidth, int srcHeight,
							  bool bHasFullyTransp, const u8* pAlphaMask )
{

	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;
	
	if( bHasFullyTransp )
	{
		//RGBA
		if (pAlphaMask)
		{
			while( pixels-- )
			{			
				
				*dst++= R8FROM565(*src);
				*dst++= G8FROM565(*src);
				*dst++= B8FROM565(*src);
				*dst++= *pAlphaMask++;

				src++;

				if( --linePixelsRemaining == 0 )
				{
					linePixelsRemaining = srcWidth;
					//point to the next line				
					dst+= ((dstPow2Width - srcWidth) * 4);
				}
			}
		}
		else
		{
			while( pixels-- )
			{			
				
				*dst++= R8FROM565(*src);
				*dst++= G8FROM565(*src);
				*dst++= B8FROM565(*src);
				*dst++= (*src == 0) ? 0x00 : 0xFF; //alpha 1.0f

				src++;

				if( --linePixelsRemaining == 0 )
				{
					linePixelsRemaining = srcWidth;
					//point to the next line				
					dst+= ((dstPow2Width - srcWidth) * 4);
				}
			}
		}
	}
	else
	{
		//RGB
		while( pixels-- )
		{
			*dst++= R8FROM565(*src);
			*dst++= G8FROM565(*src);
			*dst++= B8FROM565(*src);
		
			src++;

			if( --linePixelsRemaining == 0 )
			{
				linePixelsRemaining = srcWidth;
				//point to the next line								
				dst+= ((dstPow2Width - srcWidth) * 3);
			}
		}
	}
}


void Lib3DGL::DecodeP888To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight,
							  const u8* srcPal, int transp_index)
{

	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;
	
	if( transp_index >= 0 )
	{
		//RGBA
		while( pixels-- )
		{
			const unsigned int index = (*src) * 3;
			*dst++= srcPal[index + 2];
			*dst++= srcPal[index + 1];
			*dst++= srcPal[index + 0];
			*dst++= ((*src) == transp_index) ? 0x00 : 0xFF; //alpha 1.0f

			src++;

			if( --linePixelsRemaining == 0 )
			{
				linePixelsRemaining = srcWidth;
				//point to the next line				
				dst+= ((dstPow2Width - srcWidth) * 4);
			}
		}
	}
	else
	{
		//RGB
		while( pixels-- )
		{
			unsigned int index = (*src) * 3;
			*dst++= srcPal[index + 2];
			*dst++= srcPal[index + 1];
			*dst++= srcPal[index + 0];
		
			src++;

			if( --linePixelsRemaining == 0 )
			{
				linePixelsRemaining = srcWidth;
				//point to the next line								
				dst+= ((dstPow2Width - srcWidth) * 3);
			}
		}
	}
}

#define A8FROM8888(col)		( ( (col) >> 24) & 0xFF )
#define R8FROM8888(col)		( ( (col) >> 16) & 0xFF )
#define G8FROM8888(col)		( ( (col) >> 8) & 0xFF )
#define B8FROM8888(col)		( ( (col) >> 0) & 0xFF )


void Lib3DGL::Decode8888To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight)
{

	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;
	
	//RGBA
	while( pixels-- )
	{			
		*dst++= *(src + 1);
		*dst++= *(src + 2);
		*dst++= *(src + 3);
		*dst++= *(src + 0);

		src += 4;

		if( --linePixelsRemaining == 0 )
		{
			linePixelsRemaining = srcWidth;
			//point to the next line				
			dst+= ((dstPow2Width - srcWidth) * 4);
		}
	}
}

void Lib3DGL::Decode888To8888( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight, bool rgb)
{

	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;
	
	if (rgb)
	{
		//RGB
		while( pixels-- )
		{			
			*dst++= *(src + 0);
			*dst++= *(src + 1);
			*dst++= *(src + 2);

			src += 3;

			if( --linePixelsRemaining == 0 )
			{
				linePixelsRemaining = srcWidth;
				//point to the next line				
				dst+= ((dstPow2Width - srcWidth) * 3);
			}
		}
	}
	else
	{
		//BGR
		while( pixels-- )
		{			
			*dst++= *(src + 2);
			*dst++= *(src + 1);
			*dst++= *(src + 0);

			src += 3;

			if( --linePixelsRemaining == 0 )
			{
				linePixelsRemaining = srcWidth;
				//point to the next line				
				dst+= ((dstPow2Width - srcWidth) * 3);
			}
		}
	}
}

void Lib3DGL::DecodeP888To5551( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight,
							  const u8* srcPal, int transp_index)
{
	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;

	unsigned short *d = (unsigned short *) dst;
	
	//RGBA
	while( pixels-- )
	{
		const unsigned int index = (*src) * 3;
		int color = (((srcPal[index + 2] >> 3) << 11) | ((srcPal[index + 1] >> 3) << 6) | ((srcPal[index + 0] >> 3) << 1));	
		if (index != transp_index)
			color |= 0x01;

		*d++ = (unsigned short)color;
		src++;

		if( --linePixelsRemaining == 0 )
		{
			linePixelsRemaining = srcWidth;
			//point to the next line
			d += (dstPow2Width - srcWidth);
		}
	}
}

void Lib3DGL::DecodeP888ToLUMINANCE( u8* dst, int &dstPow2Width, int &dstPow2Height, 
									const u8* src, int srcWidth, int srcHeight,
									const u8* srcPal)
{
	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;

	u8 *d = dst;
	
	//RGBA
	while( pixels-- )
	{
		const unsigned int index = (*src) * 3;
		*d++ = (u8)((srcPal[index + 2] + srcPal[index + 1] + srcPal[index + 0]) / 3);		
		src++;

		if( --linePixelsRemaining == 0 )
		{
			linePixelsRemaining = srcWidth;
			//point to the next line				
			d += (dstPow2Width - srcWidth);
		}
	}
}

void Lib3DGL::DecodeP888To565( u8* dst, int &dstPow2Width, int &dstPow2Height, 
							  const u8* src, int srcWidth, int srcHeight,
							  const u8* srcPal)
{
	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;

	A_ASSERT(dstPow2Width * dstPow2Height <= MAX_TEX_WIDTH * MAX_TEX_HEIGHT);
	
	int pixels = srcWidth * srcHeight;
	int linePixelsRemaining = srcWidth;

	unsigned short *d = (unsigned short *) dst;
	
	//RGBA
	while( pixels-- )
	{
		const unsigned int index = (*src) * 3;
		const int color = (((srcPal[index + 2] >> 3) << 11) | ((srcPal[index + 1] >> 2) << 5) | (srcPal[index + 0] >> 3));
		*d++ = (unsigned short) color;
		src++;

		if( --linePixelsRemaining == 0 )
		{
			linePixelsRemaining = srcWidth;
			//point to the next line				
			d += (dstPow2Width - srcWidth);
		}
	}
}

void Lib3DGL::GetPow2Size(int &dstPow2Width, int &dstPow2Height,
						  int srcWidth, int srcHeight)
{
	dstPow2Width = 1;	
	while( srcWidth > dstPow2Width )
		dstPow2Width <<= 1;

	dstPow2Height = 1;
	while( srcHeight > dstPow2Height )
		dstPow2Height <<= 1;
}

#endif /* USE_OGL*/
