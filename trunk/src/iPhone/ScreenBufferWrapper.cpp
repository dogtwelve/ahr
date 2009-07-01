
#include "ScreenBufferWrapper.h"

TextureFBO* g_texScreenBuffer = NULL;

int	g_sceneViewportW			= 0;
int	g_sceneViewportH			= 0;
bool g_bSaveInScreenBuffer = false;	


static s32 s_texViewportX = 0;
static s32 s_texViewportY = 0;
static s32 s_texViewportW = 0;
static s32 s_texViewportH = 0;

static bool s_bIsGLLinkedToScreenBuffer = false;
static f32 s_screenBufferTexCoord[4];

extern CGapi Gapi;

//-------------------------------------------------------------------------------------------------
void LinkGLToScreenBuffer( s32 viewportX, s32 viewportY, s32 viewportW, s32 viewportH )
{
	s_texViewportX = viewportX;
	s_texViewportY = viewportY;
	s_texViewportW = viewportW;
	s_texViewportH = viewportH;

	A_ASSERT(viewportW <= TEX_SCREEN_BUFFER_W);
	A_ASSERT(viewportH <= TEX_SCREEN_BUFFER_H);

	g_texScreenBuffer->saveFBOLinkage();
	g_texScreenBuffer->bindFBO();

	::glViewport( s_texViewportX, s_texViewportY, s_texViewportW, s_texViewportH);
	s_bIsGLLinkedToScreenBuffer = true;
}

//-------------------------------------------------------------------------------------------------
bool IsGLLinkedToScreenBuffer()
{
	return s_bIsGLLinkedToScreenBuffer;
}

//-------------------------------------------------------------------------------------------------
void RestoreGLLinkage()
{
	g_texScreenBuffer->restoreFBOLinkage();

	int screen_width = 0;
	int screen_height= 0;
#ifdef IPHONE
	screen_width = DISP_X;
	screen_height= DISP_Y;
#else
	screen_width = CHighGear::GetInstance()->m_dispX;
	screen_height= CHighGear::GetInstance()->m_dispY;
#endif

	::glViewport( 0, 0, screen_width, screen_height );
	s_bIsGLLinkedToScreenBuffer = false;
}

//similar to Lib3DGL
#define SCR_BUFF_MAX_QUADS  50

static s16 s_scrbuff_vtx2D[SCR_BUFF_MAX_QUADS * VTX_SIZE * VTX_COUNT_PER_QUAD] = {0};
static f32 s_scrbuff_tex[SCR_BUFF_MAX_QUADS * TEXCOOR_SIZE * VTX_COUNT_PER_QUAD] = {0};
static u8  s_scrbuff_col[SCR_BUFF_MAX_QUADS * COLOR_SIZE * VTX_COUNT_PER_QUAD] = {0};
static u16 s_scrbuff_indices[SCR_BUFF_MAX_QUADS * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI] = {0};

static int s_srcbuff_quadsNB = 0;

void InitScreenBufferIndices()
{
	memcpy(s_scrbuff_indices, g_lib3DGL->m_pIndices, sizeof(u16) * (SCR_BUFF_MAX_QUADS * TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI));
}

void FlushScreenBuffer()
{
	A_ASSERT(!s_bIsGLLinkedToScreenBuffer);	

	if(s_srcbuff_quadsNB <= 0)
		return;	
	
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

	
	//set projection matrix
	::glMatrixMode(GL_PROJECTION);
	::glPushMatrix();
	::glLoadIdentity();	
#ifdef IPHONE
	//rotate the normalized window coordinate to fit the aspect ratio choosed from glViewport
	if(Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_90)
	{
		::glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	}
	else if(Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_270)
	{
		::glRotatef(270.0f, 0.0f, 0.0f, 1.0f);
	}
#endif
	::glOrthof( 0, screen_width, screen_height, 0, -1.0f, 1.0f );
	
	//set modelview matrix
	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadIdentity();	

	//set texture matrix
	::glMatrixMode(GL_TEXTURE);
	::glPushMatrix();
	::glLoadIdentity();

#ifdef IPHONE
	if(Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_90)
	{
		//top in the left part of the texture[map (0,0) to top left in texture] ... match this with texcoord
		::glTranslatef(s_texViewportX * (1.0f /TEX_SCREEN_BUFFER_W), s_texViewportY * (1.0f /TEX_SCREEN_BUFFER_H), 0.0f);
		::glScalef( 1, -1, 1);
		::glRotatef(90, 0.0f, 0.0f, -1.0f);
	}
	else if(Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_270)
	{
		//top int right part of the texture[map (0,0) to bottom right in texture] ...match this with texcoord
		::glTranslatef((s_texViewportX + s_texViewportW)* (1.0f /TEX_SCREEN_BUFFER_W), (s_texViewportY + s_texViewportH) * (1.0f /TEX_SCREEN_BUFFER_H), 0.0f);	
		::glScalef( -1, 1, 1);
		::glRotatef(90, 0.0f, 0.0f, -1.0f);
	}
	else
#endif //IPHONE	
	{
		//[map (0,0) to bottom left in texture]
		::glTranslatef(s_texViewportX * (1.0f /TEX_SCREEN_BUFFER_W), (s_texViewportY + s_texViewportH) * (1.0f /TEX_SCREEN_BUFFER_H), 0.0f);
		::glScalef( 1, -1, 1);
	}

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	//Render current 2D primitives		
	::glEnableClientState(GL_VERTEX_ARRAY);	
	::glVertexPointer(2, GL_SHORT, 0, s_scrbuff_vtx2D);

	::glEnableClientState(GL_COLOR_ARRAY);		
	::glColorPointer(4, GL_UNSIGNED_BYTE, 0, s_scrbuff_col);		
	
	::glEnable(GL_TEXTURE_2D);					
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);	
	::glTexCoordPointer(2, GL_FLOAT, 0, s_scrbuff_tex);
		
	::glBindTexture(GL_TEXTURE_2D, g_texScreenBuffer->m_glTextureName);
	
	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		
		
	::glDrawElements(GL_TRIANGLES, VTX_COUNT_PER_TRI * TRI_COUNT_PER_QUAD * s_srcbuff_quadsNB, GL_UNSIGNED_SHORT, s_scrbuff_indices);
	
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);		

	::glDisableClientState(GL_COLOR_ARRAY);
	
	::glDisableClientState(GL_VERTEX_ARRAY);

	//restore openGL state
	::glDisable(GL_ALPHA_TEST);
	::glDisable(GL_BLEND);

	::glMatrixMode(GL_TEXTURE);
	::glPopMatrix();

	::glMatrixMode(GL_PROJECTION);
	::glPopMatrix();

	::glMatrixMode(GL_MODELVIEW);
	::glPopMatrix();	

	s_srcbuff_quadsNB =0;
}

//-------------------------------------------------------------------------------------------------
void DrawScreenBuffer( s32 x, s32 y, s32 w, s32 h, u32 color)
{	
	A_ASSERT(s_srcbuff_quadsNB < SCR_BUFF_MAX_QUADS);

	s16	*	pVtx	=	s_scrbuff_vtx2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * s_srcbuff_quadsNB );
	f32 *   pTex	=   s_scrbuff_tex + ( TEXCOOR_SIZE * VTX_COUNT_PER_QUAD * s_srcbuff_quadsNB );
	u8	*	pClr	=	s_scrbuff_col + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * s_srcbuff_quadsNB );	
	
	u8	a	=	(color >> 24) & 0xFF;
	u8	r	=	(color >> 16) & 0xFF;
	u8	g	=	(color >> 8) & 0xFF;
	u8	b	=	 color & 0xFF;

	f32 u0, v0, u1, v1;

	//coordinate in texture (0, 0) .... (s_texViewportW, s_texViewportH) .. this should be inverted if landscape	
	//will be rotated/mirror/translated at render
	u0 = 0;
	v0 = 0;


	int texCoordW, texCoordH;

#ifdef IPHONE
	if(Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_90 || Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_270)
	{
		texCoordW = s_texViewportH;
		texCoordH = s_texViewportW;
	}
	else
#endif //IPHONE		
	{
		texCoordW = s_texViewportW;
		texCoordH = s_texViewportH;
	}

	
	u1 = texCoordW * (1.0f/TEX_SCREEN_BUFFER_W);
	v1 = texCoordH * (1.0f/TEX_SCREEN_BUFFER_H);		

	//uv  (u0 v0)  (u1 v1)
	*pVtx++	=	x;		*pVtx++	=	y;
	*pTex++	=	u0;		*pTex++	=	v0;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;
	
	*pVtx++	=	x + w;	*pVtx++	=	y;
	*pTex++	=	u1;		*pTex++	=	v0;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

	*pVtx++	=	x + w;	*pVtx++	=	y + h;
	*pTex++	=	u1;		*pTex++	=	v1;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

	*pVtx++	=	x;		*pVtx++	=	y + h;
	*pTex++	=	u0;		*pTex++	=	v1; //u0 v1
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;	

	s_srcbuff_quadsNB++;	
}

typedef struct tag_QuadInfo
{
	s32 x0, y0, x1, y1, x2, y2, x3, y3;
	f32 u0, v0, u1, v1, u2, v2, u3, v3;
	u32 col0, col1, col2, col3;
} QuadInfo;

void BlitFromScreenBuffer(	QuadInfo& qInfo)
{
	A_ASSERT(s_srcbuff_quadsNB < SCR_BUFF_MAX_QUADS);

	s16	*	pVtx	=	s_scrbuff_vtx2D + ( VTX_SIZE * VTX_COUNT_PER_QUAD * s_srcbuff_quadsNB );
	f32 *   pTex	=   s_scrbuff_tex + ( TEXCOOR_SIZE * VTX_COUNT_PER_QUAD * s_srcbuff_quadsNB );
	u8	*	pClr	=	s_scrbuff_col + ( COLOR_SIZE * VTX_COUNT_PER_QUAD * s_srcbuff_quadsNB );

	volatile u8	a, r, g, b;

	//uv  (u0 v0)  (u1 v1)
	a = (qInfo.col0 >> 24) & 0xFF;
	r = (qInfo.col0 >> 16) & 0xFF;
	g = (qInfo.col0 >> 8) & 0xFF;
	b = qInfo.col0 & 0xFF;

	*pVtx++	=	qInfo.x0;		*pVtx++	=	qInfo.y0;
	*pTex++	=	qInfo.u0;		*pTex++	=	qInfo.v0;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

	a = (qInfo.col1 >> 24) & 0xFF;
	r = (qInfo.col1 >> 16) & 0xFF;
	g = (qInfo.col1 >> 8) & 0xFF;
	b = qInfo.col1 & 0xFF;

	*pVtx++	=	qInfo.x1;		*pVtx++	=	qInfo.y1;
	*pTex++	=	qInfo.u1;		*pTex++	=	qInfo.v1;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

	a = (qInfo.col2 >> 24) & 0xFF;
	r = (qInfo.col2 >> 16) & 0xFF;
	g = (qInfo.col2 >> 8) & 0xFF;
	b = qInfo.col2 & 0xFF;

	*pVtx++	=	qInfo.x2;		*pVtx++	=	qInfo.y2;
	*pTex++	=	qInfo.u2;		*pTex++	=	qInfo.v2;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;

	a = (qInfo.col3 >> 24) & 0xFF;
	r = (qInfo.col3 >> 16) & 0xFF;
	g = (qInfo.col3 >> 8) & 0xFF;
	b = qInfo.col3 & 0xFF;

	*pVtx++	=	qInfo.x3;		*pVtx++	=	qInfo.y3;
	*pTex++	=	qInfo.u3;		*pTex++	=	qInfo.v3;
	*pClr++	=	r;		*pClr++	=	g;	*pClr++	=	b;	*pClr++	=	a;	

	s_srcbuff_quadsNB++;	

}

	////rotate the normalized window coordinate to fit the aspect ratio choosed from glViewport
	//switch (Gapi.mCurrentOrientation)
	//{
	//	case ORIENTATION_LANDSCAPE_90:
	//		::glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	//		break;
	//	
	//	case ORIENTATION_LANDSCAPE_270:
	//		::glRotatef(270.0f, 0.0f, 0.0f, 1.0f);
	//		break;
	//}



void DrawBlurEffect(u32 colFadeInt, u32 colFadeExt, int offset, int deltaOffset, int numSteps)
{
	
	int viewportW = CHighGear::GetInstance()->m_dispX;
	int viewportH = CHighGear::GetInstance()->m_dispY;
	
	int texCoordW, texCoordH;
	
#ifdef IPHONE
	if(Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_90 || Gapi.mCurrentOrientation == ORIENTATION_LANDSCAPE_270)
	{
		texCoordW = s_texViewportH;
		texCoordH = s_texViewportW;
	}
	else
#endif //IPHONE		
	{
		texCoordW = s_texViewportW;
		texCoordH = s_texViewportH;
	}
	
	DrawScreenBuffer( 0, 0, viewportW, viewportH, 0xFFFFFFFF);	
	
	//left, right, top, bottom relative to texture
	//the screen is flipped in texture ... diff between SCREEN COORDINATE and in TEX COORDINATE
	volatile f32 texMarginU0, texMarginV0, texMarginU1, texMarginV1;
	volatile f32 texCenterU0, texCenterV0, texCenterU1, texCenterV1;
	volatile f32 texOffSideV0, texOffSideV1, texOffTopU, texOffBottomU;
	
	volatile s16 vtxMarginU0, vtxMarginV0, vtxMarginU1, vtxMarginV1;
	volatile s16 vtxCenterU0, vtxCenterV0, vtxCenterU1, vtxCenterV1;
	volatile s16 vtxOffSideV0, vtxOffSideV1, vtxOffTopU, vtxOffBottomU;
	volatile s16 offSideU, offTopV, offBottomV;
	f32 topPercent, bottomPercent, sidePercent;

	topPercent		= 1.0f / 2.5f;
	bottomPercent	= 1.0f / 8.0f;
	sidePercent		= 1.0f / 3.0f;

	//compute tex coord in pixels
	texMarginU0 = 0;
	texMarginV0 = 0;
	texMarginU1 = texCoordW;
	texMarginV1 = texCoordH;
	
	texCenterU0 = texMarginU0 + (int)(texCoordW*sidePercent);
	texCenterV0 = texMarginV0 + (int)(texCoordH*topPercent);
	texCenterU1 = texMarginU1 - (int)(texCoordW*sidePercent);
	texCenterV1 = texMarginV1 - (int)(texCoordH*bottomPercent);
	
	//bring tex coord in (0,1)
	texMarginU0 *= (1.0f/TEX_SCREEN_BUFFER_W);
	texMarginV0 *= (1.0f/TEX_SCREEN_BUFFER_H);
	texMarginU1 *= (1.0f/TEX_SCREEN_BUFFER_W);
	texMarginV1 *= (1.0f/TEX_SCREEN_BUFFER_H);

	texCenterU0 *= (1.0f/TEX_SCREEN_BUFFER_W);
	texCenterV0 *= (1.0f/TEX_SCREEN_BUFFER_H);
	texCenterU1 *= (1.0f/TEX_SCREEN_BUFFER_W);
	texCenterV1 *= (1.0f/TEX_SCREEN_BUFFER_H);

	//compute vertex specific to tex coord
	vtxMarginU0 = 0;
	vtxMarginV0 = 0;
	vtxMarginU1 = viewportW;
	vtxMarginV1 = viewportH;

	vtxCenterU0 = 0 + (int)(viewportW*sidePercent);
	vtxCenterV0 = 0 + (int)(viewportH*topPercent);
    vtxCenterU1 = viewportW - (int)(viewportW*sidePercent);
	vtxCenterV1 = viewportH - (int)(viewportH*bottomPercent);		
	
	QuadInfo qInfo;
	
	
	for(int i = numSteps; i>0; i-- )
	{
		//compute vtxOff end texOff ... offset represent off in screen pixels on sides		
		
		offSideU	= offset;
		offTopV		= (int)( ((float)offset * (vtxCenterV0 - vtxMarginV0)) / (vtxCenterU0 - vtxMarginU0) );
		offBottomV	= (int)( ((float)offset * (vtxMarginV1 - vtxCenterV1)) / (vtxMarginU1 - vtxCenterU1) ); 		
		
			
		//left screen side----------------------------------------------
		qInfo.x0 = vtxMarginU0 - offSideU;
		qInfo.y0 = vtxMarginV0;
		qInfo.u0 = texMarginU0;
		qInfo.v0 = texMarginV0;
		qInfo.col0 = colFadeExt;
		
		qInfo.x1 = vtxCenterU0 - offSideU;
		qInfo.y1 = vtxMarginV0;
		qInfo.u1 = texCenterU0;
		qInfo.v1 = texMarginV0;
		qInfo.col1 = colFadeInt;
		
		qInfo.x2 = vtxCenterU0 - offSideU;
		qInfo.y2 = vtxMarginV1;
		qInfo.u2 = texCenterU0;
		qInfo.v2 = texMarginV1;
		qInfo.col2 = colFadeInt;
		
		qInfo.x3 = vtxMarginU0 - offSideU;
		qInfo.y3 = vtxMarginV1;
		qInfo.u3 = texMarginU0;
		qInfo.v3 = texMarginV1;
		qInfo.col3 = colFadeExt;
		
		BlitFromScreenBuffer( qInfo );
		
		//right screen side----------------------------------------------
		qInfo.x0 = vtxCenterU1 + offSideU;
		qInfo.y0 = vtxMarginV0;
		qInfo.u0 = texCenterU1;
		qInfo.v0 = texMarginV0;
		qInfo.col0 = colFadeInt;
		
		qInfo.x1 = vtxMarginU1 + offSideU;
		qInfo.y1 = vtxMarginV0;
		qInfo.u1 = texMarginU1;
		qInfo.v1 = texMarginV0;
		qInfo.col1 = colFadeExt;
		
		qInfo.x2 = vtxMarginU1 + offSideU;
		qInfo.y2 = vtxMarginV1;
		qInfo.u2 = texMarginU1;
		qInfo.v2 = texMarginV1;
		qInfo.col2 = colFadeExt;
		
		qInfo.x3 = vtxCenterU1 + offSideU;
		qInfo.y3 = vtxMarginV1;
		qInfo.u3 = texCenterU1;
		qInfo.v3 = texMarginV1;
		qInfo.col3 = colFadeInt;
		
		BlitFromScreenBuffer( qInfo );
		
		////top screen side----------------------------------------------
		qInfo.x0 = vtxMarginU0;
		qInfo.y0 = vtxMarginV0 - offTopV;
		qInfo.u0 = texMarginU0;
		qInfo.v0 = texMarginV0;
		qInfo.col0 = colFadeExt;
		
		qInfo.x1 = vtxMarginU1;
		qInfo.y1 = vtxMarginV0 - offTopV;
		qInfo.u1 = texMarginU1;
		qInfo.v1 = texMarginV0;
		qInfo.col1 = colFadeExt;
		
		qInfo.x2 = vtxMarginU1;
		qInfo.y2 = vtxCenterV0 - offTopV;
		qInfo.u2 = texMarginU1;
		qInfo.v2 = texCenterV0;
		qInfo.col2 = colFadeInt;
		
		qInfo.x3 = vtxMarginU0;
		qInfo.y3 = vtxCenterV0 - offTopV;
		qInfo.u3 = texMarginU0;
		qInfo.v3 = texCenterV0;
		qInfo.col3 = colFadeInt;
		
		BlitFromScreenBuffer( qInfo );
		
		////bottom screen side----------------------------------------------
		qInfo.x0 = vtxMarginU0;
		qInfo.y0 = vtxCenterV1 + offBottomV;
		qInfo.u0 = texMarginU0;
		qInfo.v0 = texCenterV1;
		qInfo.col0 = colFadeInt;
		
		qInfo.x1 = vtxMarginU1;
		qInfo.y1 = vtxCenterV1 + offBottomV;
		qInfo.u1 = texMarginU1;
		qInfo.v1 = texCenterV1;
		qInfo.col1 = colFadeInt;
		
		qInfo.x2 = vtxMarginU1;
		qInfo.y2 = vtxMarginV1 + offBottomV;
		qInfo.u2 = texMarginU1;
		qInfo.v2 = texMarginV1;
		qInfo.col2 = colFadeExt;
		
		qInfo.x3 = vtxMarginU0;
		qInfo.y3 = vtxMarginV1 + offBottomV;
		qInfo.u3 = texMarginU0;
		qInfo.v3 = texMarginV1;
		qInfo.col3 = colFadeExt;
		
		BlitFromScreenBuffer( qInfo );
		

/*
		//vtxOffTopU = vtxOffBottomU = 100;//offSideU;
		//texOffTopU = texOffBottomU = 100*(1.0f/TEX_SCREEN_BUFFER_W);//offSideU * (1.0f/TEX_SCREEN_BUFFER_W);
 
		//vtxOffSideV0 = offTopV;//100;//offTopV;
		//texOffSideV0 = offTopV * (1.0f/TEX_SCREEN_BUFFER_H);//100* (1.0f/TEX_SCREEN_BUFFER_H);//offTopV * (1.0f/TEX_SCREEN_BUFFER_H);
 
		//vtxOffSideV1 = offBottomV;//100;//offBottomV;
		//texOffSideV1 = offBottomV * (1.0f/TEX_SCREEN_BUFFER_H);//100* (1.0f/TEX_SCREEN_BUFFER_H);//offBottomV * (1.0f/TEX_SCREEN_BUFFER_H);
 
 
		//for test
		//offSideU = offset;
		//offTopV = offset;
		//offBottomV = offset;
 
		//vtxOffSideV0 = vtxOffSideV0 = vtxOffTopU = vtxOffBottomU = 0;
		//texOffSideV0 = texOffSideV0 = texOffTopU = texOffBottomU = 0;
 
		//left screen side----------------------------------------------
		qInfo.x0 = vtxMarginU0 - offSideU;
		qInfo.y0 = vtxMarginV0 - vtxOffSideV0;
		qInfo.u0 = texMarginU0;
		qInfo.v0 = texMarginV0 - texOffSideV0;
		qInfo.col0 = colFadeExt;

		qInfo.x1 = vtxCenterU0 - offSideU;
		qInfo.y1 = vtxCenterV0 - vtxOffSideV0;
		qInfo.u1 = texCenterU0;
		qInfo.v1 = texCenterV0 - texOffSideV0;
		qInfo.col1 = colFadeInt;

		qInfo.x2 = vtxCenterU0 - offSideU;
		qInfo.y2 = vtxCenterV1 + vtxOffSideV1;
		qInfo.u2 = texCenterU0;
		qInfo.v2 = texCenterV1 + texOffSideV1;
		qInfo.col2 = colFadeInt;

		qInfo.x3 = vtxMarginU0 - offSideU;
		qInfo.y3 = vtxMarginV1 + vtxOffSideV1;
		qInfo.u3 = texMarginU0;
		qInfo.v3 = texMarginV1 + texOffSideV1;
		qInfo.col3 = colFadeExt;
		
		BlitFromScreenBuffer( qInfo );

		//right screen side----------------------------------------------
		qInfo.x0 = vtxCenterU1 + offSideU;
		qInfo.y0 = vtxCenterV0 - vtxOffSideV0;
		qInfo.u0 = texCenterU1;
		qInfo.v0 = texCenterV0 - texOffSideV0;
		qInfo.col0 = colFadeInt;

		qInfo.x1 = vtxMarginU1 + offSideU;
		qInfo.y1 = vtxMarginV0 - vtxOffSideV0;
		qInfo.u1 = texMarginU1;
		qInfo.v1 = texMarginV0 - texOffSideV0;
		qInfo.col1 = colFadeExt;

		qInfo.x2 = vtxMarginU1 + offSideU;
		qInfo.y2 = vtxMarginV1 + vtxOffSideV1;
		qInfo.u2 = texMarginU1;
		qInfo.v2 = texMarginV1 + texOffSideV1;
		qInfo.col2 = colFadeExt;

		qInfo.x3 = vtxCenterU1 + offSideU;
		qInfo.y3 = vtxCenterV1 + vtxOffSideV1;
		qInfo.u3 = texCenterU1;
		qInfo.v3 = texCenterV1 + texOffSideV1;
		qInfo.col3 = colFadeInt;
		
		BlitFromScreenBuffer( qInfo );

		////top screen side----------------------------------------------
		qInfo.x0 = vtxMarginU0 - vtxOffTopU;
		qInfo.y0 = vtxMarginV0 - offTopV;
		qInfo.u0 = texMarginU0 - texOffTopU;
		qInfo.v0 = texMarginV0;
		qInfo.col0 = colFadeExt;

		qInfo.x1 = vtxMarginU1 + vtxOffTopU;
		qInfo.y1 = vtxMarginV0 - offTopV;
		qInfo.u1 = texMarginU1 + texOffTopU;
		qInfo.v1 = texMarginV0;
		qInfo.col1 = colFadeExt;

		qInfo.x2 = vtxCenterU1 + vtxOffTopU;
		qInfo.y2 = vtxCenterV0 - offTopV;
		qInfo.u2 = texCenterU1 + texOffTopU;
		qInfo.v2 = texCenterV0;
		qInfo.col2 = colFadeInt;

		qInfo.x3 = vtxCenterU0 - vtxOffTopU;
		qInfo.y3 = vtxCenterV0 - offTopV;
		qInfo.u3 = texCenterU0 - texOffTopU;
		qInfo.v3 = texCenterV0;
		qInfo.col3 = colFadeInt;
		
		BlitFromScreenBuffer( qInfo );

		////bottom screen side----------------------------------------------
		qInfo.x0 = vtxCenterU0 - vtxOffBottomU;
		qInfo.y0 = vtxCenterV1 + offBottomV;
		qInfo.u0 = texCenterU0 - texOffBottomU;
		qInfo.v0 = texCenterV1;
		qInfo.col0 = colFadeInt;

		qInfo.x1 = vtxCenterU1 + vtxOffBottomU;
		qInfo.y1 = vtxCenterV1 + offBottomV;
		qInfo.u1 = texCenterU1 + texOffBottomU;
		qInfo.v1 = texCenterV1;
		qInfo.col1 = colFadeInt;

		qInfo.x2 = vtxMarginU1 + vtxOffBottomU;
		qInfo.y2 = vtxMarginV1 + offBottomV;
		qInfo.u2 = texMarginU1 + texOffBottomU;
		qInfo.v2 = texMarginV1;
		qInfo.col2 = colFadeExt;

		qInfo.x3 = vtxMarginU0 - vtxOffBottomU;
		qInfo.y3 = vtxMarginV1 + offBottomV;
		qInfo.u3 = texMarginU0 - texOffBottomU;
		qInfo.v3 = texMarginV1;
		qInfo.col3 = colFadeExt;
		
		BlitFromScreenBuffer( qInfo );*/
		
		//increment offset
		offset +=deltaOffset;
	}

}

//blur effect
bool g_bIsBlurActivated = false;

//optimize ingame_menu
bool g_bRefreshScreenBuffer = false;