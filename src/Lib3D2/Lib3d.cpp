#pragma warning(disable:4786)

// 3D context, contain required object for 3d application
// this is required for Symbian because global object are
// not allowed

#include "HG/HighGear.h"
#include "Lib3d.h"
#include "Mesh.h"
#include "color.h"
#include "board3d.h"
#include "Camera.h"
#include "fx.h"
#include "Board3d.h"
#include "DevUtil.h"

#include "Performance.h"

#include <string.h>

using namespace Lib3D;

//const int kMaxFaceBuffer		= 1200*4 + 350; // rndaskalov : face/buffer sizes balanced based on statistical reports
//const int kMaxVertexBuffer		= 1200*4 - 350;
const int kMaxFaceBuffer		= 3000 * 4 + 350; // rndaskalov : face/buffer sizes balanced based on statistical reports
const int kMaxVertexBuffer		= 3000 * 4 - 350;

#if defined(WIN32) && !defined(__BREW__)
	int gNbFacesDrawn = 0;
	int gNbPixelWritten = 0;
#endif

//int Lib3D::kSizeX = m_pHG->m_dispX;
//int Lib3D::kSizeY = m_pHG->m_dispY - kInterfaceHeigth;

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CLib3D::Line
{
public:
	Line(const Vector4s& src,const Vector4s& dst,int timeInFrame,unsigned short colour)
		:m_begin(src),m_end(dst),m_time(timeInFrame),m_colour(colour)
	{};

		
	Vector4s				m_begin;
	Vector4s				m_end;
	int							m_time;
	unsigned short	m_colour;
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

#ifdef BILINEAR_FILTERING_TEST
bool CLib3D::bEnableBilinearFiltering = true;
#endif

CLib3D::CLib3D()
	:m_colIndex(NULL),	
	m_frameCount(0),
	m_RenderingEnabled(true),
	m_pHG(CHighGear::GetInstance()),
	m_dispX(m_pHG->m_dispX),
	m_dispY(m_pHG->m_dispY),
	m_screenImage3D(0)
{
	m_camera = NEW CCameraAsphalt;
	m_board3d = NEW CBoard3D;
	m_renderer = NEW CRender(k_nFovDefault);
	m_renderer->OTTableClear();
	
	for(int i = 0; i < (k_nFovMax-k_nFovMin); ++i)
	{
		m_FovToAngle[(i<<1)+0] = GetXAngleFromFov(k_nFovMin + i);
		m_FovToAngle[(i<<1)+1] = GetYAngleFromFov(k_nFovMin + i);
	}

	m_faceBuffer = NEW TFace[kMaxFaceBuffer];
	m_faceBufferAllocated=0;

	m_vertexBuffer = NEW TVertex[kMaxVertexBuffer];
	m_vertexBufferAllocated=0;

	//removed unused buffers
#ifdef USE_OGL
	m_ShadowPtr = NULL;
#else //USE_OGL
	m_ShadowPtr = NEW unsigned char[m_dispX * m_dispY];
	memset (m_ShadowPtr, 0, m_dispX * m_dispY);
#endif //USE_OGL

	shadowConst = 1;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CLib3D::~CLib3D()
{
	MM_DELETE m_camera;
	MM_DELETE m_board3d;
	MM_DELETE m_colIndex;
	MM_DELETE m_renderer;

	DELETE_ARRAY m_faceBuffer;
	DELETE_ARRAY m_vertexBuffer;

	DELETE_ARRAY m_ShadowPtr;
//	if (m_screenImage3D)
//		MM_DELETE m_screenImage3D;
}


// ---------------------------------------------------------------------------
// CLib3D::Init()
// Post creation initialisation
// Initialise color index, shaders and blitters
// ---------------------------------------------------------------------------
void CLib3D::Init()
{
	A_ASSERT(m_colIndex==NULL);
	m_colIndex = NEW CColor;
	m_skybox_previous_rot = 0;
	m_skybox_rot = 0;

	//m_screenImage3D = new Image(m_dispX, m_dispY, GetImageBuffer());
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

#ifdef USE_OGL
void CLib3D::Draw(RoadStruct* road, int from, int to, int reverse, TTexture *texture, TTexture::sAtlasTextureTile *atlasTextureTiles, bool bRenderBillboards)
{
	m_renderer->RenderRoad(road, from, to, reverse, texture, atlasTextureTiles, bRenderBillboards);
}

#else

void CLib3D::Draw(RoadStruct* road, int from, int to, int reverse )
{
	m_renderer->RenderRoad(road,from,to,&m_camera->GetPosition(),*m_board3d, *this, reverse);
}

#endif /* USE_OGL */

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

void CLib3D::Draw(Lib3D::CSkyBox& skybox, SunFX& sunFx, bool draw_bottom)
{
	TRACE("CLib3D::Draw(Lib3D::CSkyBox& skybox) begin\n");
	PERF_COUNTER(DrawSkyBox);
		
	const Vector4s& cam_rotation = 	m_camera->GetRotation();

	Vector4s	rotation = cam_rotation;

	// compare rot
	//int		ry = rotation.y;
	int		diff = rotation.y - m_skybox_previous_rot;
	m_skybox_previous_rot = rotation.y; // save new rot
	if (diff < -1024)
		diff += 2048;
	if (diff > 1024)
		diff -= 2048;
	rotation.y = m_skybox_rot + diff + 65536;
	m_skybox_rot += diff;

	//Trace( "%d", rotation.y );

	//const Vector4s& cam_position = 	m_camera->GetPosition();

	CMatrix44& matrix = PushMatrix();
	matrix.DefRotateX(-rotation.x);
	//TEST
	//skybox.Draw(*m_renderer, *m_board3d, &rotation, draw_bottom);

	PopMatrix();	

	TRACE("CLib3D::Draw(Lib3D::CSkyBox& skybox) end\n");
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CLib3D::Draw(const Vector4s&v1,const Vector4s& v2,int colour)
{
	m_board3d->SetWireFrame(colour);
	m_renderer->RenderLine(v1,v2,*m_board3d);
	m_board3d->ClearWireFrame();
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

/*
void CLib3D::Blit(unsigned short *in_ScreenPtr,int postProcessor)
{

	PERF_COUNTER(Blit);
}
*/
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CLib3D::SetFov(int in_nFov)
{  
	m_renderer->SetFoV(in_nFov);   
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CLib3D::GetVisionAngle(int& out_AngleX, int& out_AngleY) const
{
    int nFOV = m_renderer->GetFoV();
    A_ASSERT(k_nFovMin <= nFOV && nFOV < k_nFovMax);
    out_AngleX = m_FovToAngle[((nFOV-k_nFovMin)<<1) + 0];
    out_AngleY = m_FovToAngle[((nFOV-k_nFovMin)<<1) + 1];
}

const Vector4s& CLib3D::GetCameraPosition() const {
	return m_camera->GetPosition();
}	

const Vector4s& CLib3D::GetCameraRotation() const {
	return m_camera->GetRotation();
}	


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const CMatrix44* CLib3D::GetCameraMatrix()
{
	return  &m_camera->GetMatrix();
}
void CLib3D::BeginRendering(const Vector4s& camposition,const Vector4s& camRotation, const CMatrix44& fxmat)
{
	#if defined(WIN32) && !defined(__BREW__)
		gNbPixelWritten = 0;
		gNbFacesDrawn = 0;
		//DrawFaceTotal = 0;
	#endif

	m_camera->Set(camposition, camRotation, fxmat);
	CMatrix44& matrix = m_renderer->MatrixStack().ResetStack();
	matrix.Mult(GetCameraMatrix());

	// Odd / Even frame for interlaced effect
	m_FrameEven = !m_FrameEven;
	// update state in board
	m_board3d->SetFrameEven( m_FrameEven );

#if USE_ROAD_FX == 1
	m_board3d->m_RoadFXRandSeed = 1013904223;
#endif

	m_frameCount++;

	//m_renderer->OTTableClear(); // rax - cleared when rendered

	m_faceBufferAllocated=0;
	m_vertexBufferAllocated=0;

#if USE_Z_BUFFER
	m_board3d->ClearZBuffer();
#endif // USE_Z_BUFFER
}


// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void CLib3D::EndRendering()
{	
	if (m_RenderingEnabled)
	{
		m_renderer->OTTableDraw(*this);
		//m_renderer->OTTableDrawReverse(*this);
	}
	REC_MINMAX(faceBufferAllocated,m_faceBufferAllocated);
	REC_MINMAX(vertexBufferAllocated,m_vertexBufferAllocated);

	//removed unused buffers	
#ifndef USE_OGL
	shadowConst++;
	if (shadowConst == 0xFF)
	{
		memset (m_ShadowPtr, 0, m_dispX * m_dispY);
		shadowConst = 1;
	}
#endif //!USE_OGL

//#if defined (WIN32)
//	dpf("gNbFacesDrawn: %d\n", gNbFacesDrawn);
//#endif
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CLib3D::DrawBillboard(const Vector4s& pos,bool screenSpace,const TTexture& tex,int scale,const int transparency)
{
	if(screenSpace)
	{
		//assert(pos.z > -NEAR_CLIP);
		m_board3d->DrawBillboard(pos,tex,scale,GetFoV(),transparency);
	}
	else
	{
		const CMatrix44& matrix = CurrentMatrix();
		Vector4s cameraSpace;
		matrix.TransformVectorz(&pos,&cameraSpace);
		if(cameraSpace.z > FAR_CLIP_MAX && cameraSpace.z < NEAR_CLIP)
		{
			matrix.TransformVectorxy(&pos,&cameraSpace);
			Vector4s screenSpace;
			DefProjection(&cameraSpace,&screenSpace);

			A_ASSERT(screenSpace.z > -NEAR_CLIP);
			m_board3d->DrawBillboard(screenSpace,tex,scale,GetFoV(),transparency);
		}
	}
}

void CLib3D::DrawLine(const Vector4s& v1,const Vector4s& v2,int c1, int c2,bool yesZ){
	m_board3d->DrawLine(v1,v2,c1,c2,*m_colIndex,yesZ);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CLib3D::DefProjection(const Vector4s *src, Vector4s *target)	const
{
	m_board3d->DefProjection(src,target,m_renderer->GetFoV());
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
TFace* CLib3D::AllocateFaces(int nbr)
{
	if( (m_faceBufferAllocated + nbr) > kMaxFaceBuffer)
	{
		//A_ASSERT(0);
		return NULL;
	}

	TFace* f = m_faceBuffer + m_faceBufferAllocated;
	m_faceBufferAllocated += nbr;
	return f;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
TVertex* CLib3D::AllocateVertex(int nbr)
{
	if( (m_vertexBufferAllocated + nbr) > kMaxVertexBuffer)
	{
		//A_ASSERT(0);
		return NULL;
	}

	TVertex* v = m_vertexBuffer + m_vertexBufferAllocated;
	m_vertexBufferAllocated += nbr;

	return v;
}





