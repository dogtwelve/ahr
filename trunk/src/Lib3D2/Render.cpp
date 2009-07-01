#pragma warning(disable:4786)

#include "config.h"

#include "Render.h"

#include "Board3d.h"
#include "Render.h"
#include "lib3d.h"
#include "Mesh.h"
#include "texture.h"
#include "IMath.h"
#include "devUtil.h"
#include "Performance.h"


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "Gapi.h"
#include "HG/highgear.h"
extern CGapi Gapi;

#ifdef WIN32
	extern int gNbFacesDrawn;
#endif // WIN32


#define INTERPOLATE1 0                 // use interpolation method 1(more fast), else use method 2(more accurate ?)

#define USE_OTTABLE_GROUPS 1

using namespace Lib3D;

const int SUBDIV_MAX = 32;	// max subdivision count for 1 face

const int FRUSTRUM_CLIP = -6024;            // do not frustrum clip face more far than this limit

#define FACE_POOL_SIZE		(SECTION_SHOWN_MAX*(12*2 + 4) + 4)	// 16 sections * (12 quads * 2 (tris) + 4 billboards)
#define VERT_POOL_SIZE		(SECTION_SHOWN_MAX*2*2 + 4)	// 16 sections * 2 vert * 2 sides)
#define BILLBOARD_POOL_SIZE (SECTION_SHOWN_MAX*2*2 + 4)	// 16 sections * 2 vert * 2 sides)

#ifdef USE_OGL
//TEST
//extern bool AddBillboardToAlphaAppGrps(Lib3D::CRender& renderer, TBillboard& b, const Vector4s& v);

//definition for the static vtx arrays	
f32 CRender::s_pVtx[ FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_VTX_COMPONENT_COUNT ] = {0};
f32 CRender::s_pTex[ FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_TEX_COMPONENT_COUNT ] = {0};
u8  CRender::s_pColor[ FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_COLOR_COMPONENT_COUNT ] = {0};
u16 CRender::s_pIndices[ FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI ] = {0};
u16 CRender::s_pIndicesTris[ FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_TRI_COUNT] = {0};
OTGroupInfo CRender::s_pOTGroupInfo[ FACE_COMMON_TRI_COUNT ] = { OTGroupInfo() };
int CRender::s_nOpaqueFenceQuads = 0;

void CRender::InitVtxStorageArrays()
{
	for(int i=0; i < FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI; i++ )
		s_pIndices[i] = i;

	for(int i=0; i < FACE_MAX_TRI; i++ )
	{
		s_pIndicesTris[i * FACE_TRI_COUNT * FACE_VTX_COUNT_PER_TRI ] = i*VTX_COUNT_PER_QUAD + 0;
		s_pIndicesTris[i * FACE_TRI_COUNT * FACE_VTX_COUNT_PER_TRI + 1] = i*VTX_COUNT_PER_QUAD + 1;
		s_pIndicesTris[i * FACE_TRI_COUNT * FACE_VTX_COUNT_PER_TRI + 2] = i*VTX_COUNT_PER_QUAD + 2;
		
		//second triangle
		s_pIndicesTris[i * FACE_TRI_COUNT * FACE_VTX_COUNT_PER_TRI + 3] = i*VTX_COUNT_PER_QUAD + 0;
		s_pIndicesTris[i * FACE_TRI_COUNT * FACE_VTX_COUNT_PER_TRI + 4] = i*VTX_COUNT_PER_QUAD + 2;
		s_pIndicesTris[i * FACE_TRI_COUNT * FACE_VTX_COUNT_PER_TRI + 5] = i*VTX_COUNT_PER_QUAD + 3;
	}

#ifdef USE_VERTEX_COLOR_TEST

	u8 triColors[] = {
		0xFF, 0x00, 0x00, 0xF0,
		0x00, 0xFF, 0x00, 0xF0,
		0x00, 0x00, 0xFF, 0xF0
	};

	for(int i=0; i < FACE_MAX_TRI; i++ )
	{
		memcpy (s_pColor + i * FACE_VTX_COUNT_PER_TRI * FACE_COLOR_COMPONENT_COUNT, triColors, FACE_VTX_COUNT_PER_TRI * FACE_COLOR_COMPONENT_COUNT);
	}

#endif // USE_VERTEX_COLOR_TEST
}

void CRender::RenderOpaqueFenceQuads()
{
	if(s_nOpaqueFenceQuads <= 0)
		return;

	f32* pFenceVtx = s_pVtx + (FACE_COMMON_FENCE_TRI_START_IDX * FACE_VTX_COUNT_PER_TRI * FACE_VTX_COMPONENT_COUNT);
	f32* pFenceTex = s_pTex + (FACE_COMMON_FENCE_TRI_START_IDX * FACE_VTX_COUNT_PER_TRI * FACE_TEX_COMPONENT_COUNT);

	//TEST
//	TTexture *tempAtlasTexOpaque = CHighGear::GetInstance()->m_PlayingGame->m_Map->TrackTextureOpaque;

	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_LEQUAL);
	::glDepthMask(true);	
	::glDisable(GL_ALPHA_TEST);

	::glDisable(GL_BLEND);

	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY); //ACTIVATE texture
	::glDisableClientState(GL_COLOR_ARRAY);		   //DEACTIVATE color

	::glVertexPointer(3, GL_FLOAT, 0, pFenceVtx);
	::glTexCoordPointer(2, GL_FLOAT, 0, pFenceTex );		

	::glEnable( GL_TEXTURE_2D );	

	//TEST
//	::glBindTexture(GL_TEXTURE_2D, tempAtlasTexOpaque->m_glTextureName);

	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	CMatrix44 matrix = m_matrixStack.CurrentMatrix();
	matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadMatrixf(CMatrix44::s_matrixFloat);

	::glDrawElements( GL_TRIANGLES, TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI * s_nOpaqueFenceQuads, GL_UNSIGNED_SHORT, s_pIndicesTris);

	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);

	::glPopMatrix();

#ifdef WIN32
	gNbFacesDrawn += s_nOpaqueFenceQuads * TRI_COUNT_PER_QUAD;
#endif // WIN32
}
	
#endif /* USE_OGL*/

// --------------------------------------------------------------------------
// ------------------         RENDERER METHODS          ---------------------
// --------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// init rendering with default values
// ---------------------------------------------------------------------------
CRender::CRender(int fov)	
	:m_fov(0)
{
//	m_sort				= NEW CFSort;

	m_pHG = CHighGear::GetInstance();

	m_subFaceList = NEW TFace*[ SUBDIV_MAX ];
	m_subVertexList = NEW TVertex[SUBDIV_MAX*3];	// vertex for subdivided opaque face list

	SetFoV(fov);

	// init subdivision faces
	int j=0;
	for (int i=0; i<SUBDIV_MAX; i++)
	{
		m_subFaceList[i]= NEW TFace(m_subVertexList+j);
		j+=3;
	}
	face_pool = NEW TFace[ FACE_POOL_SIZE ];
	vert_pool = NEW TVertex[ VERT_POOL_SIZE ];
	billboard_pool = NEW TBillboard[BILLBOARD_POOL_SIZE];

#ifdef USE_OGL
	CRender::InitVtxStorageArrays();
#endif /* USE_OGL */
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CRender::~CRender()
{
	DELETE_ARRAY vert_pool;
	DELETE_ARRAY face_pool;
	DELETE_ARRAY billboard_pool;

	for (int i=0; i<SUBDIV_MAX; i++)
		MM_DELETE m_subFaceList[i];

	DELETE_ARRAY m_subVertexList;
	DELETE_ARRAY m_subFaceList;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

void CRender::SetFoV(int fov)
{
	//fov represent here the near clip distance
	if(m_fov != fov)
	{
		m_fov = fov;

		// define frustrum planes normal  
#ifdef USE_OGL
		//cdp temporary
		#pragma REMINDER("remove this when all faces skip frustum clipping ... NORMALS TO BIG")
		float scale = float(1 << NFRUST_SHIFT);      // scaled value of frustrum normal vector
#else /*USE_OGL*/
		float scale = float(1 << NFRUST_SHIFT);
#endif /*USE_OGL*/

		float a = float(fov-1/*security*/)/float(m_pHG->m_dispY/2);

		float d = float(::sqrt(1 + a*a));

		float z = ((a*scale)/d) + 0.5f;
		float y = (scale/d) + 0.5f;

		m_frustrumBTLR[0].Init(0, int(-z), int(y)); //botoom clip frustum
		m_frustrumBTLR[1].Init(0, int(z), int(y)) ; //top clip frustum

		a = float(fov-1/*security*/)/float(m_pHG->m_dispX/2);
		d = float(::sqrt(1 + a*a));
		z = ((a*scale)/d) + 0.5f;
		float x = (scale/d) + 0.5f;

		m_frustrumBTLR[2].Init((int)-z, 0, (int)x); //left clip frustum
		m_frustrumBTLR[3].Init((int)z, 0, (int)x);  //right clip frustum
	}

#ifdef USE_OGL

	::glMatrixMode(GL_PROJECTION);
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

	//adjust the near plane with the same aspect ratio
	//set nearClip small enough so the near plane doesn't intersect the road
	
	float nearClip = (float)(m_fov)/ (8.0f);	
	float top = (float)(m_pHG->m_dispY)/(16.0f);
	float right = (float)(m_pHG->m_dispX)/(16.0f);
	float farClip = 36000.0f;
	
	::glFrustumf(
			-right,
			right,
			-top,
			top,
			nearClip,
			farClip
			);	
	
	::glMatrixMode(GL_MODELVIEW);
	::glLoadIdentity();

#endif /*USE_OGL*/
}



// ---------------------------------------------------------------------------
// crossing point on edge
// ---------------------------------------------------------------------------
class MidPoint
	:public Vector4s
{
public:
	MidPoint(){};

	UVType u;
	UVType v;
private:

	MidPoint(const MidPoint&);
	void operator=(const MidPoint&);
};



// ---------------------------------------------------------------------------
//	Vector4s *Vx;
//	short u, v;
// ---------------------------------------------------------------------------
class VertexUV
	:public TVxuv
{
public:
	VertexUV(){};

	void operator =(const MidPoint& M)
	{  
		Vx[VEC_WRD].Init(&M);
		u = M.u;
		v = M.v;
	}	

	inline void operator=(const VertexUV& Src)
	{
		Vx[VEC_WRD] = Src.Vx[VEC_WRD];

		u = Src.u;
		v = Src.v;
	}

private:
	void operator=(const TVxuv& Src);
};


static inline int InterpolateShift16(int a,int b,int k)
{
#if INTERPOLATE1
	CHK_ADD(b,-a);
	CHK_MULT( (b-a),k);
	CHK_ADD(a, (((b - a) * k) >> 16));

	const int res =  a + (((b - a) * k) >> 16);

#else
	const int kRoundingBias = 1 << 15;

	const int l = (1 << 16) - k;
// rax - temp
//	CHK_MULT(a,l);
//	CHK_MULT(b,k);
//	CHK_ADD(a*l , b*k);
//
//	CHK_ADD(a*l + b*k,kRoundingBias);

	const int res = ((a*l + b*k) + kRoundingBias) >> 16;


	#if WIN_DEBUG
		double kd = double(k) / double( 1<<16);
		double ld = 1.0 - kd;		
		double dr	= a*ld + b*kd;

		double error = ::fabs(dr - res);

		if(error > 1.0)
			int _Set_breakpoint_here = 0;
	#endif
#endif
	return res;
}


#if WIN_DEBUG
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
static inline int InterpolateShift16(short a,short b,int k)
{
	int res = InterpolateShift16(int(a),int(b),k);
	CHECK_SHORT_LIMIT(res);
	return res;
}
#endif



// ---------------------------------------------------------------------------
// subdivide a face with a frustrum plane
// ---------------------------------------------------------------------------
void CRender::FaceSubdivide(TFace *Face,const CBoard3D& board)
{
	const Vector4s::Type *Dot = &Face->ScrVectorA().x; // retreive dot product list
	
	VertexUV *VxuvA = (VertexUV *)Face->GetVxuvA();	// points to VxA
	
	MidPoint midpoint[3];// find crossing edges and crossing point M
	int CrossCtr = 0;
	bool cross[4];

	static const char kNextVertexIndex[] = { 1, 2, 0};
	
	for (int i=2; i>=0; --i)
	{	
		const VertexUV *A,*B;
		cross[i] = false;
		int j = kNextVertexIndex[i];
		
		// check if edge cross plane
		if (Dot[i] > 0)
		{
			if (Dot[j] > 0)
				continue;
		}
		else
			if (Dot[j] <= 0)
				continue;
			
			int Di, Dj;
			if (Dot[i] < Dot[j])               // always cut in same order
			{
				A = &VxuvA[i];
				B = &VxuvA[j];
				
				Di = Dot[i];
				Dj = Dot[j];
			}
			else
				if (Dot[i] > Dot[j])
				{
					A  = &VxuvA[j];      
					B  = &VxuvA[i];
					
					Di = Dot[j];
					Dj = Dot[i];
				}
				else
				{
					A_ASSERT(false);
					continue;                        // should never reach this line
				}
				
				//int k = board.DivideShift16(-Di,Dj - Di);
				int k = (-Di << 16) / (Dj - Di);
				
				const Vector4s& vA = A->Vx[VEC_WRD];
				const Vector4s& vB = B->Vx[VEC_WRD];
				
				midpoint[i].x = ::InterpolateShift16(vA.x,vB.x,k);
				midpoint[i].y = ::InterpolateShift16(vA.y,vB.y,k);
				midpoint[i].z = ::InterpolateShift16(vA.z,vB.z,k);
				//M[i].s = ::InterpolateShift16(vA.s,vB.s,k);
				
				midpoint[i].u = ::InterpolateShift16(A->u,B->u,k);
				midpoint[i].v = ::InterpolateShift16(A->v,B->v,k);
				
				
				cross[i] = true;
				CrossCtr++;
	}
	
	if (CrossCtr != 2)                   // need at least 2 cross point to subdivide
	{
		if (Dot[0] > 0)
			Face->SetFlag(1);	// Don't display the face
		return;
	}
	
	A_ASSERT(m_subFaceListSize>=0 && m_subFaceListSize <SUBDIV_MAX);
	
	TFace *SubFace = m_subFaceList[m_subFaceListSize];
	VertexUV *VxuvB = (VertexUV *)SubFace->GetVxuvA();
	
	if (cross[0])
	{
		if (Dot[0] <= 0)                   // A in frustrum
		{
			if (cross[2])                   // simple cut
			{
				VxuvA[1] = midpoint[0];
				VxuvA[2] = midpoint[2];
			}
			else                             // cut 2 faces
			{
				VxuvB[0] = VxuvA[0];   // set up face B
				VxuvB[1] = midpoint[1];
				VxuvB[2] = VxuvA[2];
				
				SubFace->InitSubDivFace(*Face);        
				SubFace->ClearFlag(1); // NEEDED !!!
				m_subFaceListSize++;
				
				VxuvA[1] = midpoint[0];       // modify face A
				VxuvA[2] = midpoint[1];
			}
		}
		else                               // A not in frustrum
		{
			if (cross[2])                   // simple cut
			{
				VxuvB[0] = midpoint[0];
				VxuvB[1] = VxuvA[2];   // set up face B
				VxuvB[2] = midpoint[2];
				
				SubFace->InitSubDivFace(*Face);        
				SubFace->ClearFlag(1);// NEEDED !!!
				++m_subFaceListSize;
				
				VxuvA[0] = midpoint[0];
			}
			else                             // cut 2 faces
			{
				VxuvA[0] = midpoint[0];
				VxuvA[2] = midpoint[1];
			}
		}
	}
	else
	{
		if (Dot[0] <= 0)                   // A in frustrum
		{
			VxuvB[0] = VxuvA[0];     // set up face B
			VxuvB[1] = midpoint[1];
			VxuvB[2] = midpoint[2];
			
			SubFace->InitSubDivFace(*Face);      
			SubFace->ClearFlag(1);// NEEDED !!!
			++m_subFaceListSize;
			
			VxuvA[2] = midpoint[1];
		}
		else
		{
			VxuvA[0] = midpoint[1];
			VxuvA[1] = VxuvA[2];
			VxuvA[2] = midpoint[2];
		}
	}
}

// not used
//inline static void Projection(Lib3D::CLib3D& lib3d,const Vector4s& src,Vector4s& target)
//{
//	if(src.z==0)
//		target.z=0;
//	else
//	{
//		const int sizex = lib3d.Width();
//		const int sizey = lib3d.Height();
//		
//		
//		target.x = sizex>>1;
//		target.y = sizey>>1;
//		
//		target.z = -src.z;
//		
//		const int fov = lib3d.GetFoV();
//		
//		CHK_MULT(fov,src.x);
//		CHK_MULT(fov,src.y);
//		
//		target.x += (fov*src.x)/target.z;
//		target.y -= (fov*src.y)/target.z;
//		
//		
//		if(	target.x > ( sizex<<1)	 || 
//			target.x < (-sizex<<1) ||
//			target.y > ( sizey<<1) ||
//			target.y < (-sizey<<1))
//			target.z = 0;
//	}
//}

// TBD this was a test, but now it is used in the game, OPTIMIZE THIS !
// rax: actually not used
//void CRender::FrustrumSubdiv4(TFace *Face,CBoard3D& board,unsigned int tface_flag )
//{
//	TFace		subFace[4];
//	TVertex		vert[6];
//	int			u[6];
//	int			v[6];
//	
//	vert[0] = *Face->VertexA();
//	vert[1] = *Face->VertexB();
//	vert[2] = *Face->VertexC();
//
//	vert[3].WorldTPos = (vert[0].WorldTPos + vert[1].WorldTPos) / 2;
//	vert[4].WorldTPos = (vert[1].WorldTPos + vert[2].WorldTPos) / 2;
//	vert[5].WorldTPos = (vert[2].WorldTPos + vert[0].WorldTPos) / 2;
//
//	u[0] = Face->GetuA();
//	v[0] = Face->GetvA();
//	u[1] = Face->GetuB();
//	v[1] = Face->GetvB();
//	u[2] = Face->GetuC();
//	v[2] = Face->GetvC();
//
//	u[3] = (u[0] + u[1]) / 2;
//	v[3] = (v[0] + v[1]) / 2;
//	u[4] = (u[1] + u[2]) / 2;
//	v[4] = (v[1] + v[2]) / 2;
//	u[5] = (u[2] + u[0]) / 2;
//	v[5] = (v[2] + v[0]) / 2;
//
//	subFace[0].SetTexture( Face->GetTexture() );
//	subFace[1].SetTexture( Face->GetTexture() );
//	subFace[2].SetTexture( Face->GetTexture() );
//	subFace[3].SetTexture( Face->GetTexture() );
//
//	subFace[0].SetVectorA( vert + 0 );
//	subFace[0].SetVectorB( vert + 3 );
//	subFace[0].SetVectorC( vert + 5 );
//
//	subFace[1].SetVectorA( vert + 3 );
//	subFace[1].SetVectorB( vert + 1 );
//	subFace[1].SetVectorC( vert + 4 );
//
//	subFace[2].SetVectorA( vert + 5 );
//	subFace[2].SetVectorB( vert + 4 );
//	subFace[2].SetVectorC( vert + 2 );
//
//	subFace[3].SetVectorA( vert + 3 );
//	subFace[3].SetVectorB( vert + 5 );
//	subFace[3].SetVectorC( vert + 4 );
//
//	subFace[0].SetuA( u[0] );
//	subFace[0].SetvA( v[0] );
//	subFace[0].SetuB( u[3] );
//	subFace[0].SetvB( v[3] );
//	subFace[0].SetuC( u[5] );
//	subFace[0].SetvC( v[5] );
//
//	subFace[1].SetuA( u[3] );
//	subFace[1].SetvA( v[3] );
//	subFace[1].SetuB( u[1] );
//	subFace[1].SetvB( v[1] );
//	subFace[1].SetuC( u[4] );
//	subFace[1].SetvC( v[4] );
//
//	subFace[2].SetuA( u[5] );
//	subFace[2].SetvA( v[5] );
//	subFace[2].SetuB( u[4] );
//	subFace[2].SetvB( v[4] );
//	subFace[2].SetuC( u[2] );
//	subFace[2].SetvC( v[2] );
//
//	subFace[3].SetuA( u[3] );
//	subFace[3].SetvA( v[3] );
//	subFace[3].SetuB( u[5] );
//	subFace[3].SetvB( v[5] );
//	subFace[3].SetuC( u[4] );
//	subFace[3].SetvC( v[4] );
//
//	board.DefProjection(&vert[0].WorldTPos, &vert[0].ScreenPos, m_fov);
//	board.DefProjection(&vert[1].WorldTPos, &vert[1].ScreenPos, m_fov);
//	board.DefProjection(&vert[2].WorldTPos, &vert[2].ScreenPos, m_fov);
//	board.DefProjection(&vert[3].WorldTPos, &vert[3].ScreenPos, m_fov);
//	board.DefProjection(&vert[4].WorldTPos, &vert[4].ScreenPos, m_fov);
//	board.DefProjection(&vert[5].WorldTPos, &vert[5].ScreenPos, m_fov);
//
//	board.DrawFace(subFace+0,tface_flag);
//	board.DrawFace(subFace+1,tface_flag);
//	board.DrawFace(subFace+2,tface_flag);
//	board.DrawFace(subFace+3,tface_flag);
//}

// TBD this was a test, but now it is used in the game, OPTIMIZE THIS !
// rax: actually not used
//void CRender::FrustrumSubdiv4Front(TFace *Face,CBoard3D& board,unsigned int tface_flag )
//{
//	TFace		subFace[4];
//	TVertex		vert[6];
//	int			u[6];
//	int			v[6];
//	
//	subFace[0].ClearAllFlags();
//	subFace[1].ClearAllFlags();
//	subFace[2].ClearAllFlags();
//	subFace[3].ClearAllFlags();
//
//	vert[0].WorldTPos = Face->VertexA()->WorldTPos;
//	vert[1].WorldTPos = Face->VertexB()->WorldTPos;
//	vert[2].WorldTPos = Face->VertexC()->WorldTPos;
//	vert[0].ClearVMask();
//	vert[0].AddVMask( Face->VertexA()->Vmask() );
//	vert[1].ClearVMask();
//	vert[1].AddVMask( Face->VertexB()->Vmask() );
//	vert[2].ClearVMask();
//	vert[2].AddVMask( Face->VertexC()->Vmask() );
//
//	vert[3].WorldTPos = (vert[0].WorldTPos + vert[1].WorldTPos) / 2;
//	vert[4].WorldTPos = (vert[1].WorldTPos + vert[2].WorldTPos) / 2;
//	vert[5].WorldTPos = (vert[2].WorldTPos + vert[0].WorldTPos) / 2;
//
//	u[0] = Face->GetuA();
//	v[0] = Face->GetvA();
//	u[1] = Face->GetuB();
//	v[1] = Face->GetvB();
//	u[2] = Face->GetuC();
//	v[2] = Face->GetvC();
//
//	u[3] = (u[0] + u[1]) / 2;
//	v[3] = (v[0] + v[1]) / 2;
//	u[4] = (u[1] + u[2]) / 2;
//	v[4] = (v[1] + v[2]) / 2;
//	u[5] = (u[2] + u[0]) / 2;
//	v[5] = (v[2] + v[0]) / 2;
//
//	subFace[0].SetTexture( Face->GetTexture() );
//	subFace[1].SetTexture( Face->GetTexture() );
//	subFace[2].SetTexture( Face->GetTexture() );
//	subFace[3].SetTexture( Face->GetTexture() );
//
//	subFace[0].SetVectorA( vert + 0 );
//	subFace[0].SetVectorB( vert + 3 );
//	subFace[0].SetVectorC( vert + 5 );
//
//	subFace[1].SetVectorA( vert + 3 );
//	subFace[1].SetVectorB( vert + 1 );
//	subFace[1].SetVectorC( vert + 4 );
//
//	subFace[2].SetVectorA( vert + 5 );
//	subFace[2].SetVectorB( vert + 4 );
//	subFace[2].SetVectorC( vert + 2 );
//
//	subFace[3].SetVectorA( vert + 3 );
//	subFace[3].SetVectorB( vert + 4 );
//	subFace[3].SetVectorC( vert + 5 );
//
//	subFace[0].SetuA( u[0] );
//	subFace[0].SetvA( v[0] );
//	subFace[0].SetuB( u[3] );
//	subFace[0].SetvB( v[3] );
//	subFace[0].SetuC( u[5] );
//	subFace[0].SetvC( v[5] );
//
//	subFace[1].SetuA( u[3] );
//	subFace[1].SetvA( v[3] );
//	subFace[1].SetuB( u[1] );
//	subFace[1].SetvB( v[1] );
//	subFace[1].SetuC( u[4] );
//	subFace[1].SetvC( v[4] );
//
//	subFace[2].SetuA( u[5] );
//	subFace[2].SetvA( v[5] );
//	subFace[2].SetuB( u[4] );
//	subFace[2].SetvB( v[4] );
//	subFace[2].SetuC( u[2] );
//	subFace[2].SetvC( v[2] );
//
//	subFace[3].SetuA( u[3] );
//	subFace[3].SetvA( v[3] );
//	subFace[3].SetuB( u[4] );
//	subFace[3].SetvB( v[4] );
//	subFace[3].SetuC( u[5] );
//	subFace[3].SetvC( v[5] );
//
//	if (!(vert[0].Vmask() & kRejectNear))
//		board.DefProjection(&vert[0].WorldTPos, &vert[0].ScreenPos, m_fov);
//	if (!(vert[1].Vmask() & kRejectNear))
//		board.DefProjection(&vert[1].WorldTPos, &vert[1].ScreenPos, m_fov);
//	if (!(vert[2].Vmask() & kRejectNear))
//		board.DefProjection(&vert[2].WorldTPos, &vert[2].ScreenPos, m_fov);
//
//	vert[3].ClearVMask();
//	if (vert[3].WorldTPos.z > NEAR_CLIP)
//		vert[3].AddVMask( kRejectNear );
//	else
//		board.DefProjection(&vert[3].WorldTPos, &vert[3].ScreenPos, m_fov);
//
//	vert[4].ClearVMask();
//	if (vert[4].WorldTPos.z > NEAR_CLIP)
//		vert[4].AddVMask( kRejectNear );
//	else
//		board.DefProjection(&vert[4].WorldTPos, &vert[4].ScreenPos, m_fov);
//
//	vert[5].ClearVMask();
//	if (vert[5].WorldTPos.z > NEAR_CLIP)
//		vert[5].AddVMask( kRejectNear );
//	else
//		board.DefProjection(&vert[5].WorldTPos, &vert[5].ScreenPos, m_fov);
//
//
//	if (kRejectNear & (vert[0].Vmask() | vert[3].Vmask() | vert[5].Vmask()))
//		FrustrumSubdiv( subFace+0, board, tface_flag);
//	else
//		board.DrawFace(subFace+0,tface_flag);
//
//	if (kRejectNear & (vert[3].Vmask() | vert[1].Vmask() | vert[4].Vmask()))
//		FrustrumSubdiv( subFace+1, board, tface_flag);
//	else
//		board.DrawFace(subFace+1,tface_flag);
//
//	if (kRejectNear & (vert[5].Vmask() | vert[4].Vmask() | vert[2].Vmask()))
//		FrustrumSubdiv( subFace+2, board, tface_flag);
//	else
//		board.DrawFace(subFace+2,tface_flag);
//
//	if (kRejectNear & (vert[3].Vmask() | vert[4].Vmask() | vert[5].Vmask()))
//		FrustrumSubdiv( subFace+3, board, tface_flag);
//	else
//		board.DrawFace(subFace+3,tface_flag);
//}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

void CRender::FrustrumSubdiv(const TFace *Face,CBoard3D& board,unsigned int tface_flag, bool cull )
{
	// Ce code est une gracieusetee d'un Ostie d'cave  == Putain de tarÈ!

	//const VertexUV* VxuvA = (const VertexUV *)(((unsigned long*)Face) + 2);
	//VertexUV* VxuvB = (VertexUV *)(((unsigned long*)m_subFaceList[0]) + 2);

	const int	mask = Face->VertexA()->Vmask() | Face->VertexB()->Vmask() | Face->VertexC()->Vmask();

	if (cull)	// can be done only if not front culled of course !
	{
		// Culling Test

		// projection
		if (!Face->ScrVectorA().z)
			board.DefProjection(&Face->VectorA(VEC_WRD), (Vector4s*)&Face->ScrVectorA(),m_fov);

		if (!Face->ScrVectorB().z)
			board.DefProjection(&Face->VectorB(VEC_WRD), (Vector4s*)&Face->ScrVectorB(),m_fov);

		if (!Face->ScrVectorC().z)
			board.DefProjection(&Face->VectorC(VEC_WRD), (Vector4s*)&Face->ScrVectorC(),m_fov);

		register Vector4s*	VxA = (Vector4s*)Face->VertexA();
		register Vector4s*	VxB = (Vector4s*)Face->VertexB();
		register Vector4s*	VxC = (Vector4s*)Face->VertexC();

		if ((((VxA->x - VxB->x) * (VxA->y - VxC->y)) - ((VxA->y - VxB->y) * (VxA->x - VxC->x))) >= 0)
		{
#if defined WIN32
			if (Face->m_flag & TFACE_FLAG_ENVMAP)
				otface_culled_2_nb++;
#endif
			return;
		}

		// do we need to clip ? (avoid doing so if not necessary !)
		if (mask == 0)
		{
			board.DrawFace(Face,tface_flag);
			return;
		}
	}


	const VertexUV* VxuvA = (const VertexUV *)Face->GetVxuvA();
	VertexUV* VxuvB = (VertexUV *)m_subFaceList[0]->GetVxuvA();

	VxuvB[0] = VxuvA[0];
	VxuvB[1] = VxuvA[1];
	VxuvB[2] = VxuvA[2];

	m_subFaceList[0]->InitSubDivFace(*Face);
	m_subFaceList[0]->ClearAllFlags();
	m_subFaceListSize = 1;

	TFace *SubFace = m_subFaceList[0];

	// use VxA to store dot products
	Vector4s& dot = SubFace->ScrVectorA();

	dot.x = SubFace->VectorA(VEC_WRD).z - NEAR_CLIP;
	dot.y = SubFace->VectorB(VEC_WRD).z - NEAR_CLIP;
	dot.z = SubFace->VectorC(VEC_WRD).z - NEAR_CLIP;

	FaceSubdivide(SubFace,board);

	// unoptimized 3D CLIPPING for ZCorrected faces only
	if (Face->m_flag & TFACE_FLAG_ZCORRECTED)
	{
		for (int fidx = 0; fidx < 4; ++fidx)
		{
			if (mask & (1<<fidx))	// only if needed -> crossing the plane
			{
				int	n = m_subFaceListSize;
				for (int i = 0; i < n; ++i)
				{
					SubFace = m_subFaceList[i];

					if (!(SubFace->m_flag & 0x1))
					{
						Vector4s& dot = SubFace->ScrVectorA();

//#define SCALEVEC		4
#define SCALEVEC_SHIFT	2
						dot.x = (SubFace->VectorA(VEC_WRD).x * (m_frustrumBTLR[fidx].x >> SCALEVEC_SHIFT) + SubFace->VectorA(VEC_WRD).y * (m_frustrumBTLR[fidx].y >> SCALEVEC_SHIFT) + SubFace->VectorA(VEC_WRD).z * (m_frustrumBTLR[fidx].z >> SCALEVEC_SHIFT) ) / (1 << (NFRUST_SHIFT - SCALEVEC_SHIFT));
						dot.y = (SubFace->VectorB(VEC_WRD).x * (m_frustrumBTLR[fidx].x >> SCALEVEC_SHIFT) + SubFace->VectorB(VEC_WRD).y * (m_frustrumBTLR[fidx].y >> SCALEVEC_SHIFT) + SubFace->VectorB(VEC_WRD).z * (m_frustrumBTLR[fidx].z >> SCALEVEC_SHIFT) ) / (1 << (NFRUST_SHIFT - SCALEVEC_SHIFT));
						dot.z = (SubFace->VectorC(VEC_WRD).x * (m_frustrumBTLR[fidx].x >> SCALEVEC_SHIFT) + SubFace->VectorC(VEC_WRD).y * (m_frustrumBTLR[fidx].y >> SCALEVEC_SHIFT) + SubFace->VectorC(VEC_WRD).z * (m_frustrumBTLR[fidx].z >> SCALEVEC_SHIFT) ) / (1 << (NFRUST_SHIFT - SCALEVEC_SHIFT));
/*
// Draw road triangle lines
						CColor col;

						board.DrawLine( SubFace->ScrVectorA(), SubFace->ScrVectorB(), 0xF00F, 0xF00F, col, false );
						board.DrawLine( SubFace->ScrVectorB(), SubFace->ScrVectorC(), 0xF00F, 0xF00F, col, false );
						board.DrawLine( SubFace->ScrVectorC(), SubFace->ScrVectorA(), 0xF00F, 0xF00F, col, false );
*/
						FaceSubdivide(SubFace,board);
					}
				}
			}
		}
	}
  
	// draw subdivided face
	for (int i=0; i<m_subFaceListSize; ++i)
	{
		A_ASSERT(i<SUBDIV_MAX);
		TFace *FSub = m_subFaceList[i];
		if (!FSub->TestAnyFlag())
		{
			board.DefProjection(&FSub->VectorA(VEC_WRD), &FSub->ScrVectorA(),m_fov);
			board.DefProjection(&FSub->VectorB(VEC_WRD), &FSub->ScrVectorB(),m_fov);
			board.DefProjection(&FSub->VectorC(VEC_WRD), &FSub->ScrVectorC(),m_fov);
/*
// Draw road triangle lines
			CColor col;
			
			board.DrawLine( FSub->ScrVectorA(), FSub->ScrVectorB(), 0xF00F, 0xF00F, col, false );
			board.DrawLine( FSub->ScrVectorB(), FSub->ScrVectorC(), 0xF00F, 0xF00F, col, false );
			board.DrawLine( FSub->ScrVectorC(), FSub->ScrVectorA(), 0xF00F, 0xF00F, col, false );
*/

			board.DrawFace(FSub,tface_flag);
		}
	}
}

// ---------------------------------------------------------------------------
// perform following operations:
// transform WorldTPos position using current view matrix if position >= -NEAR_CLIP
//    - check position of vertex relatively to frustrum:
//    - set space partition flags (Vmask)
//    - flag as not screen transformed (ScreenPos.z = 0)
// ---------------------------------------------------------------------------
unsigned long CRender::TransformVertexFakeZ(TVertex *V,TVertex* lastVertex) const
{
	const CMatrix44& matrix = m_matrixStack.CurrentMatrix();

	unsigned long orFlag = 0x00;

	while(V!=lastVertex)
	{
		unsigned int vMask = 0;
		const int z = matrix.TransformVectorz(&V->InitialPos);
		const int x = matrix.TransformVectorx(&V->InitialPos);
		const int y = matrix.TransformVectory(&V->InitialPos);

	
		if ((z > -FRUSTRUM_CLIP) || (z <= FAR_CLIP))      // too far behind camera, avoid to compute space area or too far
			vMask |=kRejectFar;

		if (z > NEAR_CLIP)
			vMask |= kRejectNear;

#ifndef USE_OGL

		// optimized dot product, one component is always null
		int Dot = m_frustrumBTLR[0].y*y + m_frustrumBTLR[0].z*z;
		if (Dot > CLIP_REJECT)
			vMask |= kRejectR;

		Dot = m_frustrumBTLR[1].y*y + m_frustrumBTLR[1].z*z;
		if (Dot > CLIP_REJECT)
			vMask |= kRejectL;

		Dot = m_frustrumBTLR[2].x*x + m_frustrumBTLR[2].z*z;
		if (Dot > CLIP_REJECT)
			vMask |= kRejectT;

		Dot = m_frustrumBTLR[3].x*x + m_frustrumBTLR[3].z*z;
		if (Dot > CLIP_REJECT)
			vMask |= kRejectB;
#endif /* !USE_OGL */

		V->WorldTPos.x = x;
		V->WorldTPos.y = y;
		V->WorldTPos.z = z;
		V->SetVMask(vMask);
		V->ScreenPos.z = 0; // flag that screen projection not yet done
		orFlag |= vMask;
		V++;
	}
	return orFlag;
}


// ---------------------------------------------------------------------------
// perform following operations:
// transform WorldTPos position using current view matrix if position >= -NEAR_CLIP
//    - check position of vertex relatively to frustrum:
//    - set space partition flags (Vmask)
//    - flag as not screen transformed (ScreenPos.z = 0)
// ---------------------------------------------------------------------------
unsigned long CRender::TransformVertex(TVertex *V, TVertex* lastVertex) const
{
	const CMatrix44& matrix = m_matrixStack.CurrentMatrix();

	unsigned long orFlag = 0x00;

	while( V != lastVertex)
	{
		unsigned int vMask = 0x00;
		const int z = matrix.TransformVectorz(&V->InitialPos);

		if ((z > -FRUSTRUM_CLIP) || (z <= FAR_CLIP))      // too far behind camera, avoid to compute space area or too far
			vMask |= kRejectFar;
		else
		{
			const int x = matrix.TransformVectorx(&V->InitialPos);
			const int y = matrix.TransformVectory(&V->InitialPos);

#ifndef USE_OGL

			if (z > NEAR_CLIP)
				vMask |= kRejectNear;

			// optimized dot product, one component is always null
			int Dot = m_frustrumBTLR[0].y*y + m_frustrumBTLR[0].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectR;

			Dot = m_frustrumBTLR[1].y*y + m_frustrumBTLR[1].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectL;

			Dot = m_frustrumBTLR[2].x*x + m_frustrumBTLR[2].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectT;

			Dot = m_frustrumBTLR[3].x*x + m_frustrumBTLR[3].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectB;
#endif /* !USE_OGL */

			V->WorldTPos.x = x;
			V->WorldTPos.y = y;
		}

		orFlag |= vMask;

		V->SetVMask(vMask);
		V->WorldTPos.z = z;
		V->ScreenPos.z = 0; // flag that screen projection not yet done
		V++;
	}
	return orFlag;
}

unsigned long CRender::TransformVertex_Test(TVertex *V, TVertex* lastVertex)
{
	const CMatrix44& matrix = m_matrixStack.CurrentMatrix();

	unsigned long orFlag = 0x00;
	m_andFlag = 0xFF;

	while( V != lastVertex)
	{
		unsigned int vMask = 0x00;
		const int z = matrix.TransformVectorz(&V->InitialPos);

		if ((z > -FRUSTRUM_CLIP) || (z <= FAR_CLIP))      // too far behind camera, avoid to compute space area or too far
			vMask |= kRejectFar;
		else
		{
			const int x = matrix.TransformVectorx(&V->InitialPos);
			const int y = matrix.TransformVectory(&V->InitialPos);

#ifndef USE_OGL

			if (z > NEAR_CLIP)
				vMask |= kRejectNear;

			// optimized dot product, one component is always null
			int Dot = m_frustrumBTLR[0].y*y + m_frustrumBTLR[0].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectR;

			Dot = m_frustrumBTLR[1].y*y + m_frustrumBTLR[1].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectL;

			Dot = m_frustrumBTLR[2].x*x + m_frustrumBTLR[2].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectT;

			Dot = m_frustrumBTLR[3].x*x + m_frustrumBTLR[3].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectB;
#endif /* !USE_OGL */

			V->WorldTPos.x = x;
			V->WorldTPos.y = y;
		}

		orFlag |= vMask;
		m_andFlag &= vMask;

		V->SetVMask(vMask);
		V->WorldTPos.z = z;
		V->ScreenPos.z = 0; // flag that screen projection not yet done
		V++;
	}
	return orFlag;
}


unsigned long CRender::TransformVertexPointerArray(TVertex **vertexPointerArray,int count) const
{
	const CMatrix44& matrix = m_matrixStack.CurrentMatrix();

	unsigned long orFlag = 0x00;
	
	//for (int i=0;i<count;i++)
	for (int i=count-1; i>=0; --i)
	{
		TVertex * V = vertexPointerArray[i];
		unsigned int vMask = 0x00;
		const int z = matrix.TransformVectorz(&V->InitialPos);

		if ((z > -FRUSTRUM_CLIP) || (z <= FAR_CLIP))      // too far behind camera, avoid to compute space area or too far
			vMask |=kRejectFar;
		else
		{
			const int x = matrix.TransformVectorx(&V->InitialPos);
			const int y = matrix.TransformVectory(&V->InitialPos);

#ifndef USE_OGL

			if (z > NEAR_CLIP)
				vMask |= kRejectNear;

			// optimized dot product, one component is always null
			int Dot = m_frustrumBTLR[0].y*y + m_frustrumBTLR[0].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectR;

			Dot = m_frustrumBTLR[1].y*y + m_frustrumBTLR[1].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectL;

			Dot = m_frustrumBTLR[2].x*x + m_frustrumBTLR[2].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectT;

			Dot = m_frustrumBTLR[3].x*x + m_frustrumBTLR[3].z*z;
			if (Dot > CLIP_REJECT)
				vMask |= kRejectB;
#endif /* !USE_OGL */

			V->WorldTPos.x = x;
			V->WorldTPos.y = y;
		}

		orFlag |= vMask;

		V->SetVMask(vMask);
		V->WorldTPos.z = z;
		V->ScreenPos.z = 0; // flag that screen projection not yet done
		//V++;
	}

	return orFlag;
}

// ---------------------------------------------------------------------------
// perform following operations:
// transform WorldTPos position using current view matrix
// ---------------------------------------------------------------------------
void CRender::TransformVertexNoClip(TVertex *V,TVertex* lastVertex) const
{
	const CMatrix44& matrix = m_matrixStack.CurrentMatrix();
	while( V != lastVertex)
	{
		matrix.TransformVector(&V->InitialPos, &V->WorldTPos);
		V->ScreenPos.z = 0; // flag that screen projection not yet done
		V++;
	}
}


// ---------------------------------------------------------------------------
// draw a face in frustrum if subdiv not required. All Vertex must have been 
//	transformed before with TransformVertex() function
// ---------------------------------------------------------------------------
void CRender::FrustrumDraw(TFace *F,CBoard3D& board,unsigned int tface_flag)
{
	if (!F->ScrVectorA().z)
		board.DefProjection(&F->VectorA(VEC_WRD), &F->ScrVectorA(),m_fov);

	if (!F->ScrVectorB().z)
		board.DefProjection(&F->VectorB(VEC_WRD), &F->ScrVectorB(),m_fov);

	if (!F->ScrVectorC().z)
		board.DefProjection(&F->VectorC(VEC_WRD), &F->ScrVectorC(),m_fov);

	board.DrawFace(F,tface_flag);
}


// ---------------------------------------------------------------------------
// draw a face in frustrum if subdiv not required. All Vertex must have been 
//	transformed before with TransformVertex() function
// CULL and SORT (for dynamic meshes)
// ---------------------------------------------------------------------------
void CRender::FrustrumDrawCull(TFace *F,CBoard3D& board,unsigned int tface_flag)
{
	#if WIN_DEBUG
		static double _nbrTotal = 0;
		_nbrTotal++;
	#endif
	// projection
	if (!F->ScrVectorA().z)
		board.DefProjection(&F->VectorA(VEC_WRD), &F->ScrVectorA(),m_fov);

	if (!F->ScrVectorB().z)
		board.DefProjection(&F->VectorB(VEC_WRD), &F->ScrVectorB(),m_fov);

	if (!F->ScrVectorC().z)
		board.DefProjection(&F->VectorC(VEC_WRD), &F->ScrVectorC(),m_fov);


	// Culling Test
	register Vector4s*	VxA = (Vector4s*)F->VertexA();
	register Vector4s*	VxB = (Vector4s*)F->VertexB();
	register Vector4s*	VxC = (Vector4s*)F->VertexC();

 	if ((((VxA->x - VxB->x) * (VxA->y - VxC->y)) - ((VxA->y - VxB->y) * (VxA->x - VxC->x))) >= 0)
	{
#if defined WIN32
		if (F->m_flag & TFACE_FLAG_ENVMAP)
			otface_culled_2_nb++;
#endif
		return;
	}

	#if WIN_DEBUG
		static double _nbrNotCulled = 0;
		_nbrNotCulled++;

		double culledRatio = 1.0 - (_nbrNotCulled/_nbrTotal);
	#endif

	// draw
	board.DrawFace(F,tface_flag);
}
/*
void CRender::FrustrumDrawCullSort(TFace *F,CBoard3D& board)
{
	// projection
	if (!F->ScrVectorA().z)
		board.DefProjection(&F->VectorA(VEC_WRD), &F->ScrVectorA(),m_fov);

	if (!F->ScrVectorB().z)
		board.DefProjection(&F->VectorB(VEC_WRD), &F->ScrVectorB(),m_fov);

	if (!F->ScrVectorC().z)
		board.DefProjection(&F->VectorC(VEC_WRD), &F->ScrVectorC(),m_fov);


	// Culling Test
	register Vector4s*	VxA = (Vector4s*)F->VertexA();
	register Vector4s*	VxB = (Vector4s*)F->VertexB();
	register Vector4s*	VxC = (Vector4s*)F->VertexC();

	if ((((VxA->x - VxB->x) * (VxA->y - VxC->y)) - ((VxA->y - VxB->y) * (VxA->x - VxC->x))) >= 0)
		return;

	// draw
    m_sort->SortFace(F);
}*/



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CRender::RenderLine(const Vector4s& v1,const Vector4s& v2,CBoard3D& board)
{
	TVertex tv1,tv2;

	tv1.InitialPos = v1;
	tv2.InitialPos = v2;

	TransformVertex(&tv1);
	TransformVertex(&tv2);

	int OrFlag = tv1.Vmask() | tv2.Vmask();

	if (OrFlag & kRejectFar)
      return;

	
	
	int AndFlag = tv1.Vmask() & tv2.Vmask();
	if(AndFlag !=0)
		return;
	
	if(OrFlag & kRejectNear)
	{
		A_ASSERT((tv1.Vmask() & kRejectNear) !=0x00 ||(tv2.Vmask() & kRejectNear) !=0x00);


		Vector4s v = tv1.WorldTPos - tv2.WorldTPos;

		int n_dot_v = v.z;	// Since n = 0,0,1
		
		Vector4s Q = tv1.WorldTPos - Vector4s(0,0,-NEAR_CLIP);	// Segment origin, when translated to make d==0

		int n_dot_q = Q.z;	// Again, n is 0,0,1

		int t;
		{
            // All this code just for a division !!!
			if(n_dot_q < 0)
			{
				n_dot_q = -n_dot_q;
				n_dot_v = -n_dot_v;
			}
			
			if(n_dot_q &0x7F800000)
			{
				t = -n_dot_q / DownShift16(n_dot_v);
			}
			else if(n_dot_q & 0x007F8000)
				t = -(n_dot_q<<8) / DownShift8(n_dot_v);
			else
				t = -(n_dot_q<<16) / n_dot_v;

            CHK_PRECISION(-double(n_dot_q) * (1<<16) / n_dot_v, t, 5);

		}		
		const Vector4s rv = tv2.WorldTPos - tv1.WorldTPos;

		const Vector4s point(	DownShift16(rv.x * t) + tv1.WorldTPos.x,
								DownShift16(rv.y * t) + tv1.WorldTPos.y,
								DownShift16(rv.z * t) + tv1.WorldTPos.z);

		if(tv1.Vmask() & kRejectNear)							
			tv1.WorldTPos = point;
		else
			tv2.WorldTPos = point;
	}

	board.DefProjection(&tv1.WorldTPos, &tv1.ScreenPos,m_fov);  
	board.DefProjection(&tv2.WorldTPos, &tv2.ScreenPos,m_fov);

	board.DrawEdgeHV(	tv1.ScreenPos.x, tv1.ScreenPos.y, 
										tv2.ScreenPos.x, tv2.ScreenPos.y);
}


// ---------------------------------------------------------------------------
// reject faces not visible in frustrum, and flag face requiring subdivision
// CamPosition is used to test culling, NULL can be passed if object is mobile
// ---------------------------------------------------------------------------

#ifndef USE_OGL

void CRender::RenderRoad(RoadStruct *road, int from, int to, const Vector4s *CamPosition,CBoard3D& board, CLib3D& lib3d, int reverse)
{
	PERF_COUNTER(CRender_RenderRoad);

	TVertex*		vertOut = road->Transformed;
	int				nbSection;
	unsigned char	fogr,fogg,fogb;

#ifdef __565_RENDERING__
	fogr = road->FogColor[0] >> 3;
	fogg = road->FogColor[1] >> 2;
	fogb = road->FogColor[2] >> 3;
#else
	fogr = road->FogColor[0] >> 4;
	fogg = road->FogColor[1] >> 4;
	fogb = road->FogColor[2] >> 4;
#endif

	// copy / transform road vertice (TBD optimize this by using a cycling buffer)
	{
		int		nb;
		from = from - 1;
		//LAngelov: fixed a crash when driving reverse
		if(from < 0)
			from = road->SectionNb + from;

		if (to > from)
			nb = to - from;
		else
			nb = (road->SectionNb - from) + to;	// loop

		nbSection = nb;

		RoadSection		*sec = road->Section + from;
		RoadSection		*end = road->Section + road->SectionNb;

		while (nb--)
		{
			// each section
			{
				long*		vertIn = sec->Vertex;
				int			v = SECTION_NBVERT;

				while (v--)
				{
					// copy
					vertOut->InitialPos.Init( *vertIn, *(vertIn+1), *(vertIn+2) );
					// transform
					TransformVertex(vertOut);

					vertIn += 3;
					++vertOut;
				}
			}

			++sec;
			if (sec >= end)	// loop
				sec = road->Section;
		}
	}

	int			curr_section;

	//--------------------------------------
	// DRAW Faces
	if (reverse)
	{
		// we draw in reverse from [from] to [to]
		vertOut = road->Transformed;
		curr_section = from;
	}
	else
	{
		// we draw from [to] to [from]
		vertOut = road->Transformed + (nbSection - 2)*SECTION_NBVERT;
		curr_section = to - 1;
		if (curr_section < 0)
			curr_section = road->SectionNb - 1;
	}

	int OrFlag, AndFlag;

	TFace*	face1			= face_pool;
	TFace*	face2			= face_pool + (FACE_POOL_SIZE >> 1);	// so we can inc our pointers
	TBillboard* bb			= billboard_pool;

	TVertex*	vert = vert_pool;		// vert for generated vertical fence


	int		nbSec = nbSection - 1;
	bool	zcorrect = false;
	bool	subdiv4 = false;
	bool	flat = true;
	while (nbSec--)
	{
		int		nbQuads = (SECTION_NBVERT-1);
		
		// flat distant polys
		//if (nbSec == nbSection - 5)	// three fog polys
		if (nbSec == nbSection - 5)	// three fog polys
			flat = false;

		// zcorrect only 4 first section faces
		if (nbSec == 4)	//4
			zcorrect = true;

		// subdiv4 only 2 first section faces
		if (nbSec == 2)	// 2
			subdiv4 = true;

		// get current visual_id (for texture and billboards)
		int			visual_id;
		if (reverse)
		{
			visual_id = road->Section[curr_section].VisualId;
		}
		else
		{
			if (curr_section != 0)
				visual_id = road->Section[curr_section - 1].VisualId;
			else
				visual_id = road->Section[road->SectionNb - 1].VisualId;
		}

		while (nbQuads--)
		{
			// set textures
			TTexture*	text = road->VisualTextures[ visual_id ].Textures[nbQuads];

			if (text)
			{
				int			text_idx = road->VisualTextures[ visual_id ].MaterialIdx[nbQuads];
				int			flagX = road->VisualTextures[ visual_id ].flipX & (1 << nbQuads);
				int			flagY = road->VisualTextures[ visual_id ].flipY & (1 << nbQuads);
				int			tile = road->TextureTile[ text_idx ];

				int		texture_u = (text->DrawMaskX() << TTexture::TEX_UV_SHIFT) * tile;
				int		texture_v = (text->DrawMaskY() << TTexture::TEX_UV_SHIFT);

				if (flat)
				{
					int	col = text->GetTextureColor( 0, 0 );//1, 1 );
					int	r, g, b;

#ifdef __565_RENDERING__
					b = col&0x001F;
					g = (col&0x07E0)>>5;
					r = (col&0xF800)>>11;

					switch ( nbSection - nbSec - 1)
					{
					case 1:
						// 3/4 FOG
						col = ((b+(fogb*3))>>2) | (((g+(fogg*3))>>2)<<5) | (((r+(fogr*3))>>2)<<11);
					break;
					case 2:
						// MID FOG
						col = ((b+fogb)>>1) | (((g+fogg)>>1)<<5) | (((r+fogr)>>1)<<11);
					break;
					case 3:
						// 1/4 FOG
						col = ((b*3+fogb)>>2) | (((g*3+fogg)>>2)<<5) | (((r*3+fogr)>>2)<<11);
					break;
					}
#else
					r = col&0x00F;
					g = (col&0x0F0)>>4;
					b = (col&0xF00)>>8;

					switch ( nbSection - nbSec - 1)
					{
					case 1:
						// 3/4 FOG
						col = ((r+(fogb*3))>>2) | (((g+(fogg*3))>>2)<<4) | (((b+(fogr*3))>>2)<<8);
					break;
					case 2:
						// MID FOG
						col = ((r+fogb)>>1) | (((g+fogg)>>1)<<4) | (((b+fogr)>>1)<<8);
					break;
					case 3:
						// 1/4 FOG
						col = ((r*3+fogb)>>2) | (((g*3+fogg)>>2)<<4) | (((b*3+fogr)>>2)<<8);
					break;
					}
#endif

					face1->SetTexture( (const TTexture*)col );
					face2->SetTexture( (const TTexture*)col );
				}
				else
				{
					face1->SetTexture( text );
					face2->SetTexture( text );
				}

				// TBD optimize the following !!! (we can avoid to change eveything each time)

				// start flags
				int	startflag;
				if ((nbQuads==0) || (nbQuads==SECTION_NBVERT-2))	// do not zcorrect exteriors
				{
					if (flat)
						startflag = TFACE_FLAG_FLAT | TFACE_FLAG_OFFSET_BACK;
					else if (zcorrect && subdiv4)
						startflag = TFACE_FLAG_ZCORRECTED | TFACE_FLAG_SUBDIV4; 
					else if (zcorrect)
						startflag = TFACE_FLAG_ZCORRECTED; 
					else
						startflag = TFACE_FLAG_OFFSET_BACK;
				}
				else
				{
					if (zcorrect)
						startflag = TFACE_FLAG_ZCORRECTED | TFACE_FLAG_OFFSET_BACK | TFACE_FLAG_ROAD;
					else if (flat)
						startflag = TFACE_FLAG_FLAT | TFACE_FLAG_OFFSET_BACK;
					else
						startflag = TFACE_FLAG_OFFSET_BACK;
				}

				//if (!flat && text->m_globalAlpha != 0xFF)
				//{
				//	startflag |= TFACE_FLAG_TRANS_ADDITIVE_ALPHA/* | TFACE_FLAG_OFFSET_FRONT*/;
				//	startflag &= ~TFACE_FLAG_ZCORRECTED;
				//}

#if USE_TRACK_REVERSED
				if (road->Reverse == 0)
#endif // USE_TRACK_REVERSED
				{
					//int	tileFlag = road->TextureTile[ text_idx ];
					//if (nbQuads <= ((SECTION_NBVERT >> 1)-1))
					//	flags |= TEXTURE_FLAG_FLIP_X;

					//switch (flags & TEXTURE_FLAG_MASK)
					//case 0:
					if (!flagX && !flagY)
					{
						face1->SetuA( 0 );
						face1->SetvA( 0 );
						face1->SetuB( 0 );
						face1->SetvB( texture_v );
						face1->SetuC( texture_u );
						face1->SetvC( 0 );

						face2->SetuA( 0 );
						face2->SetvA( texture_v );
						face2->SetuB( texture_u );
						face2->SetvB( texture_v );
						face2->SetuC( texture_u );
						face2->SetvC( 0 );
					}
					else if (flagX && flagY)
					{
						face1->SetuA( texture_u );
						face1->SetvA( texture_v );
						face1->SetuB( texture_u );
						face1->SetvB( 0 );
						face1->SetuC( 0 );
						face1->SetvC( texture_v );

						face2->SetuA( texture_u );
						face2->SetvA( 0 );
						face2->SetuB( 0 );
						face2->SetvB( 0 );
						face2->SetuC( 0 );
						face2->SetvC( texture_v );
					}
					else if (flagX)
					{
						face1->SetuA( 0 );
						face1->SetvA( texture_v );
						face1->SetuB( 0 );
						face1->SetvB( 0 );
						face1->SetuC( texture_u );
						face1->SetvC( texture_v );

						face2->SetuA( 0 );
						face2->SetvA( 0 );
						face2->SetuB( texture_u );
						face2->SetvB( 0 );
						face2->SetuC( texture_u );
						face2->SetvC( texture_v );
					}
					else // if (flagY)
					{
						face1->SetuA( texture_u );
						face1->SetvA( 0 );
						face1->SetuB( texture_u );
						face1->SetvB( texture_v );
						face1->SetuC( 0 );
						face1->SetvC( 0 );

						face2->SetuA( texture_u );
						face2->SetvA( texture_v );
						face2->SetuB( 0 );
						face2->SetvB( texture_v );
						face2->SetuC( 0 );
						face2->SetvC( 0 );
					}					
				}
#if USE_TRACK_REVERSED
				else
				{
					// handle track reverse case

					// uv (& mirror)
					if (nbQuads > ((SECTION_NBVERT >> 1)-1))
					{
						face1->SetuA( texture_u );
						face1->SetvA( texture_v );
						face1->SetuB( texture_u );
						face1->SetvB( 0 );
						face1->SetuC( 0 );
						face1->SetvC( texture_v );
						
						
						face2->SetuB( 0 );
						face2->SetvB( 0 );
						face2->SetuA( texture_u );
						face2->SetvA( 0 );
						face2->SetuC( 0 );
						face2->SetvC( texture_v );
					}
					else
					{
						// flip texture coordinates (mirror) from the middle of the road
						face1->SetuA( texture_u );
						face1->SetvA( 0 );
						face1->SetuB( texture_u );
						face1->SetvB( texture_v );
						face1->SetuC( 0 );
						face1->SetvC( 0 );

						face2->SetuA( texture_u );
						face2->SetvA( texture_v );
						face2->SetuB( 0 );
						face2->SetvB( texture_v );
						face2->SetuC( 0 );
						face2->SetvC( 0 );
					}
				}
#endif // USE_TRACK_REVERSED
				//			|\
				// FACE1    |_\.

				TVertex *VxA = vertOut;
				TVertex *VxB = vertOut + 1;
				TVertex *VxC = vertOut + SECTION_NBVERT;

				face1->SetVectorA( VxA ) ;
				face1->SetVectorB( VxB );
				face1->SetVectorC( VxC );

				OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();				
				if (!(OrFlag & kRejectFar))		// one vertex too far behind NEAR_CLIP
				{
					// if f != 0 then face is not visible in frustrum
					AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();  
					if (!AndFlag)
					{
						if (OrFlag & kRejectNear)
						{
							if (!flat)	// we don't subdiv flat faces
							{
								face1->m_flag = startflag | TFACE_FLAG_SUBDIV;
								OTTableAdd( face1++ );
							}
						}
						else				
						{
							face1->m_flag = startflag;
							OTTableAdd( face1++ );
						}
					}
				}


				//			\-|
				// FACE2	 \|

				face2->SetVectorA( VxA = vertOut + 1);
				face2->SetVectorB( VxB = vertOut + SECTION_NBVERT + 1 );
				face2->SetVectorC( VxC = vertOut + SECTION_NBVERT );

				OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();
				if (!(OrFlag & kRejectFar))		// one vertex too far behind NEAR_CLIP
				{
					// if f != 0 then face is not visible in frustrum
					AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();  
					if (!AndFlag)
					{
						if (OrFlag & kRejectNear)
						{
							if (!flat)	// we don't subdiv flat faces
							{
								face2->m_flag = startflag | TFACE_FLAG_SUBDIV;
								OTTableAdd( face2++ );
							}
						}
						else				
						{
							face2->m_flag = startflag;
							OTTableAdd( face2++ );
						}
					}
				}
			}
			{
				//--------------------------------------
				// FENCE (first and last quad & not flat) 
				if ((nbQuads == 0) || (nbQuads == (SECTION_NBVERT-2))  && (!flat))
				{
					int	startflag;
					if (flat)
						startflag = TFACE_FLAG_OFFSET_FRONT;
					else if (zcorrect && subdiv4)
						startflag = TFACE_FLAG_ZCORRECTED | TFACE_FLAG_SUBDIV4 | TFACE_FLAG_OFFSET_FRONT; 
					else if (zcorrect)
						startflag = TFACE_FLAG_ZCORRECTED | TFACE_FLAG_OFFSET_FRONT; 
					else
						startflag = TFACE_FLAG_OFFSET_FRONT;					
					
					TVertex *VxA = vertOut;
					TVertex *VxB = vertOut + 1;
					TVertex *VxC = vertOut + SECTION_NBVERT;

					for (int two = 0; two < 2; two++)	// we have 2 fence on the quad
					{
						TVertex	*vertOut2;
						int		fence_idx;

						if (nbQuads == (SECTION_NBVERT-2))	// RIGHT
						{
							vertOut2 = vertOut + two;
							fence_idx = 3 - two;
						}
						else	// LEFT
						{
							vertOut2 = vertOut - two + 1;
							fence_idx = two;
						}

						TTexture*	text = road->VisualTextures[ visual_id ].Textures[ fence_idx + 8 ];

						if (text)
						{
							//if (!flat && text->m_globalAlpha != 0xFF)
							//{
							//	startflag |= TFACE_FLAG_TRANS_ADDITIVE_ALPHA/* | TFACE_FLAG_OFFSET_FRONT*/;
							//	startflag &= ~TFACE_FLAG_ZCORRECTED;
							//}

							int		text_idx = road->VisualTextures[ visual_id ].MaterialIdx[8 + fence_idx];
							A_ASSERT( text_idx != 255 );
							int		tile = road->TextureTile[ text_idx ];

							int		texture_u = (text->DrawMaskX() << TTexture::TEX_UV_SHIFT) * tile;
							int		texture_v = text->DrawMaskY() << TTexture::TEX_UV_SHIFT;

	/*						if (flat)
							{
								int	col = text->GetTextureColor( 1, 1 );
								int	r, g, b;

								r = col&0x00F;
								g = (col&0x0F0)>>4;
								b = (col&0xF00)>>8;

								switch ( nbSection - nbSec - 1)
								{
								case 1:
									// 3/4 FOG
									col = ((r+(fogb*3))>>2) | (((g+(fogg*3))>>2)<<4) | (((b+(fogr*3))>>2)<<8);
								break;
								case 2:
									// MID FOG
									col = ((r+fogb)>>1) | (((g+fogg)>>1)<<4) | (((b+fogr)>>1)<<8);
								break;
								case 3:
									// 1/4 FOG
									col = ((r*3+fogb)>>2) | (((g*3+fogg)>>2)<<4) | (((b*3+fogr)>>2)<<8);
								break;
								}

								face1->SetTexture( (const TTexture*)col );
								face2->SetTexture( (const TTexture*)col );
							}
							else*/
							{
								face1->SetuA( 0 );
								face1->SetvA( 0 );
								face1->SetuB( texture_u );
								face1->SetvB( 0 );
								face1->SetuC( texture_u );
								face1->SetvC( texture_v );

								face2->SetuA( 0 );
								face2->SetvA( 0 );
								face2->SetuB( texture_u );
								face2->SetvB( texture_v );
								face2->SetuC( 0 );
								face2->SetvC( texture_v );

								// set textures
								face1->SetTexture( text );
								face2->SetTexture( text );
							}

							int		height = road->VisualTextures[ visual_id ].FenceHeight[ fence_idx ];

							//			|\
							// FACE1    |_\.

							face1->SetVectorA( VxA = vertOut2) ;
							face1->SetVectorB( VxB = vertOut2 + SECTION_NBVERT);

							face1->SetVectorC( VxC = vert++ );
							VxC->InitialPos.Init( VxB->InitialPos.x, VxB->InitialPos.y + height, VxB->InitialPos.z );
							TransformVertex( VxC );

							OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();
							if (!(OrFlag & kRejectFar))		// one vertex too far behind NEAR_CLIP
							{

								// if f != 0 then face is not visible in frustrum
								AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();

								if (!AndFlag)
								{
									face1->m_flag = startflag;

									if (OrFlag & kRejectNear)
										face1->m_flag |= TFACE_FLAG_SUBDIV | TFACE_FLAG_SUBDIV4;

									OTTableAdd( face1++ );
								}
							}

							//			\-|
							// FACE2	 \|

							face2->SetVectorA( VxA );
							face2->SetVectorB( VxC );
							face2->SetVectorC( VxB = vert++ );

							VxB->InitialPos.Init( VxA->InitialPos.x, VxA->InitialPos.y + height, VxA->InitialPos.z );
							TransformVertex( VxB );

							OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();
							if (!(OrFlag & kRejectFar))		// one vertex too far behind NEAR_CLIP
							{
									// if f != 0 then face is not visible in frustrum
								AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();  
								if (!AndFlag)
								{
									face2->m_flag = startflag;

									if (OrFlag & kRejectNear)
										face2->m_flag |= TFACE_FLAG_SUBDIV | TFACE_FLAG_SUBDIV4;

									OTTableAdd( face2++ );
								}
							}
						}
					}
				}
			}


			// NEXT QUAD
			vertOut++;
		}

		//---------------------------------------------
		// TOP (tunnel)
		{
			// set textures
			TTexture*	text = road->VisualTextures[ visual_id ].Textures[ROAD_TEXTURE_IDX_TUNNEL];
			if (text)
			{
	//			int			text_idx = road->VisualTextures[ visual_id ].MaterialIdx[nbQuads];
	//			int			tile = road->TextureTile[ text_idx ];

				int		texture_u = (text->DrawMaskX() << TTexture::TEX_UV_SHIFT);
				int		texture_v = (text->DrawMaskY() << TTexture::TEX_UV_SHIFT);

				if (flat)
				{
					int	col = text->GetTextureColor( 1, 1 );
					int	r, g, b;

					r = col&0x00F;
					g = (col&0x0F0)>>4;
					b = (col&0xF00)>>8;

					switch ( nbSection - nbSec - 1)
					{
					case 1:
						// 3/4 FOG
						col = ((r+(fogb*3))>>2) | (((g+(fogg*3))>>2)<<4) | (((b+(fogr*3))>>2)<<8);
					break;
					case 2:
						// MID FOG
						col = ((r+fogb)>>1) | (((g+fogg)>>1)<<4) | (((b+fogr)>>1)<<8);
					break;
					case 3:
						// 1/4 FOG
						col = ((r*3+fogb)>>2) | (((g*3+fogg)>>2)<<4) | (((b*3+fogr)>>2)<<8);
					break;
					}

					face1->SetTexture( (const TTexture*)col );
					face2->SetTexture( (const TTexture*)col );
				}
				else
				{
					face1->SetTexture( text );
					face2->SetTexture( text );
				}

				// TBD optimize the following !!! (we can avoid to change eveything each time)

				// start flags
				int	startflag;
				if (zcorrect)
					startflag = TFACE_FLAG_ZCORRECTED | TFACE_FLAG_OFFSET_BACK;
				else if (flat)
					startflag = TFACE_FLAG_FLAT | TFACE_FLAG_OFFSET_BACK;
				else
					startflag = TFACE_FLAG_OFFSET_BACK;

				//if (!flat && text->m_globalAlpha != 0xFF)
				//{
				//	startflag |= TFACE_FLAG_TRANS_ADDITIVE_ALPHA/* | TFACE_FLAG_OFFSET_FRONT*/;
				//	startflag &= ~TFACE_FLAG_ZCORRECTED;
				//}

				face1->SetuA( 0 );
				face1->SetvA( texture_v );
				face1->SetuB( 0 );
				face1->SetvB( 0 );
				face1->SetuC( texture_u );
				face1->SetvC( texture_v );
					
				face2->SetuB( texture_u );
				face2->SetvB( 0 );
				face2->SetuA( 0 );
				face2->SetvA( 0 );
				face2->SetuC( texture_u );
				face2->SetvC( texture_v );

				//			|\
				// FACE1    |_\.

				TVertex *VxA;
				TVertex *VxB;
				TVertex *VxC;

				face1->SetVectorA( VxA = vertOut) ;
				face1->SetVectorB( VxB = vertOut - SECTION_NBVERT + 1);
				face1->SetVectorC( VxC = vertOut + SECTION_NBVERT );

				OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();
				if (!(OrFlag & kRejectFar))		// one vertex too far behind NEAR_CLIP
				{

					// if f != 0 then face is not visible in frustrum
					AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();  
					if (!AndFlag)
					{
						if (OrFlag & kRejectNear)
						{
							if (!flat)	// we don't subdiv flat faces
							{
								face1->m_flag = startflag | TFACE_FLAG_SUBDIV;
								OTTableAdd( face1++ );
							}
						}
						else				
						{
							face1->m_flag = startflag;
							OTTableAdd( face1++ );
						}
					}
				}


				//			\-|
				// FACE2	 \|

				face2->SetVectorA( VxA = vertOut - SECTION_NBVERT + 1 );
				face2->SetVectorB( VxB = vertOut + 1 );
				face2->SetVectorC( VxC = vertOut + SECTION_NBVERT );

				OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();
				if (!(OrFlag & kRejectFar))		// one vertex too far behind NEAR_CLIP
				{
					// if f != 0 then face is not visible in frustrum
					AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();  
					if (!AndFlag)
					{
						if (OrFlag & kRejectNear)
						{
							if (!flat)	// we don't subdiv flat faces
							{
								face2->m_flag = startflag | TFACE_FLAG_SUBDIV;
								OTTableAdd( face2++ );
							}
						}
						else				
						{
							{
								face2->m_flag = startflag;
								OTTableAdd( face2++ );
							}
						}
					}
				}
			}
		}

		//---------------------------------------------------
		// NEXT SECTION
		vertOut++; // we don't use the last

		//---------------------------------------------------
		// BILLBOARD

		if (nbSec < (nbSection - 4))
		{
			TTexture*	text;


			// 1st billboard

			text = road->VisualTextures[ visual_id ].Textures[13];

			if (text)
			{
				Vector4s	v;

				int	height = text->SizeY() + road->VisualTextures[ visual_id ].BillboardHeight[1];

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[1];
				if (pos & BILLBOARD_MID)
					v.Init( (vertOut[-8].InitialPos.x + vertOut[-9].InitialPos.x)/2, 
							(vertOut[-8].InitialPos.y + vertOut[-9].InitialPos.y)/2 + height, 
							(vertOut[-8].InitialPos.z + vertOut[-9].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vertOut[-9].InitialPos.x, vertOut[-9].InitialPos.y + height, vertOut[-9].InitialPos.z );
				else
					v.Init( vertOut[-8].InitialPos.x, vertOut[-8].InitialPos.y + height, vertOut[-8].InitialPos.z );

				const CMatrix44&	matrix = CurrentMatrix();
				Vector4s			cameraSpace;
				matrix.TransformVectorz( &v, &cameraSpace );
				if ((cameraSpace.z > FAR_CLIP_MAX) && (cameraSpace.z < NEAR_CLIP))
				{
					matrix.TransformVectorxy( &v, &cameraSpace );
					lib3d.DefProjection( &cameraSpace, &bb->pos );

					A_ASSERT(bb->pos.z > -NEAR_CLIP);
					bb->m_flag = TFACE_FLAG_BILLBOARD |  TFACE_FLAG_OFFSET_FRONT;
					if (pos & BILLBOARD_TRANS_MID)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
					else if (pos & BILLBOARD_TRANS_ADD)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
					else if (text->m_globalAlpha != 0xFF)
					{
						bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
						bb->alpha = text->m_globalAlpha >> 4;
					}
					bb->size = 256;
					bb->m_texture = text;
					OTTableAdd(bb);
					bb++;
				}
			}

			text = road->VisualTextures[ visual_id ].Textures[12];
			if (text)
			{				
				Vector4s	v;

				int	height = text->SizeY() + road->VisualTextures[ visual_id ].BillboardHeight[0];

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[0];
				if (pos & BILLBOARD_MID)
					v.Init(	(vertOut[-1].InitialPos.x + vertOut[-2].InitialPos.x)/2, 
							(vertOut[-1].InitialPos.y + vertOut[-2].InitialPos.y)/2 + height, 
							(vertOut[-1].InitialPos.z + vertOut[-2].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vertOut[-1].InitialPos.x, vertOut[-1].InitialPos.y + height, vertOut[-1].InitialPos.z );
				else
					v.Init( vertOut[-2].InitialPos.x, vertOut[-2].InitialPos.y + height, vertOut[-2].InitialPos.z );

				const CMatrix44&	matrix = CurrentMatrix();
				Vector4s			cameraSpace;
				matrix.TransformVectorz( &v, &cameraSpace );
				if ((cameraSpace.z > FAR_CLIP_MAX) && (cameraSpace.z < NEAR_CLIP))
				{
					matrix.TransformVectorxy( &v, &cameraSpace );
					lib3d.DefProjection( &cameraSpace, &bb->pos );

					A_ASSERT(bb->pos.z > -NEAR_CLIP);
					bb->m_flag = TFACE_FLAG_BILLBOARD |  TFACE_FLAG_OFFSET_FRONT;
					if (pos & BILLBOARD_TRANS_MID)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
					else if (pos & BILLBOARD_TRANS_ADD)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
					else if (text->m_globalAlpha != 0xFF)
					{
						bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
						bb->alpha = text->m_globalAlpha >> 4;
					}
					bb->size = 256;
					bb->m_texture = text;
					OTTableAdd(bb);
					bb++;
				}
			}
			// 2nd billboard
			text = road->VisualTextures[ visual_id ].Textures[16];

			if (text)
			{				
				Vector4s	v;

				int	height = ((text->SizeY()-2) * road->VisualTextures[ visual_id ].BillboardHeight[3]) / 128;

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[3];
				if (pos & BILLBOARD_MID)
					v.Init( (vertOut[-8].InitialPos.x + vertOut[-9].InitialPos.x)/2, 
							(vertOut[-8].InitialPos.y + vertOut[-9].InitialPos.y)/2 + height, 
							(vertOut[-8].InitialPos.z + vertOut[-9].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vertOut[-9].InitialPos.x, vertOut[-9].InitialPos.y + height, vertOut[-9].InitialPos.z );
				else
					v.Init( vertOut[-8].InitialPos.x, vertOut[-8].InitialPos.y + height, vertOut[-8].InitialPos.z );

				const CMatrix44&	matrix = CurrentMatrix();
				Vector4s			cameraSpace;
				matrix.TransformVectorz( &v, &cameraSpace );
				if ((cameraSpace.z > FAR_CLIP_MAX) && (cameraSpace.z < NEAR_CLIP))
				{
					matrix.TransformVectorxy( &v, &cameraSpace );
					lib3d.DefProjection( &cameraSpace, &bb->pos );

					A_ASSERT(bb->pos.z > -NEAR_CLIP);
					bb->start_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[2];
					bb->end_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[3];
					bb->m_flag = TFACE_FLAG_BILLBOARD/* |  TFACE_FLAG_OFFSET_FRONT*/ | TFACE_FLAG_BILBOARD_STARTEND;
					if (pos & BILLBOARD_TRANS_MID)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
					else if (pos & BILLBOARD_TRANS_ADD)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
					else if (text->m_globalAlpha != 0xFF)
					{
						bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
						bb->alpha = text->m_globalAlpha >> 4;
					}
					bb->size = road->VisualTextures[ visual_id ].BillboardHeight[3];
					bb->m_texture = text;
					OTTableAdd(bb);
					bb++;

				}
			}

			text = road->VisualTextures[ visual_id ].Textures[15];
			if (text)
			{

				Vector4s	v;

				int	height = ((text->SizeY()-2) * road->VisualTextures[ visual_id ].BillboardHeight[2]) / 128;

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[2];
				if (pos & BILLBOARD_MID)
					v.Init(	(vertOut[-1].InitialPos.x + vertOut[-2].InitialPos.x)/2, 
							(vertOut[-1].InitialPos.y + vertOut[-2].InitialPos.y)/2 + height, 
							(vertOut[-1].InitialPos.z + vertOut[-2].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vertOut[-1].InitialPos.x, vertOut[-1].InitialPos.y + height, vertOut[-1].InitialPos.z );
				else
					v.Init( vertOut[-2].InitialPos.x, vertOut[-2].InitialPos.y + height, vertOut[-2].InitialPos.z );

				const CMatrix44&	matrix = CurrentMatrix();
				Vector4s			cameraSpace;
				matrix.TransformVectorz( &v, &cameraSpace );
				if ((cameraSpace.z > FAR_CLIP_MAX) && (cameraSpace.z < NEAR_CLIP))
				{
					matrix.TransformVectorxy( &v, &cameraSpace );
					lib3d.DefProjection( &cameraSpace, &bb->pos );

					A_ASSERT(bb->pos.z > -NEAR_CLIP);
					bb->start_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[0];
					bb->end_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[1];
					bb->m_flag = TFACE_FLAG_BILLBOARD/* |  TFACE_FLAG_OFFSET_FRONT*/ | TFACE_FLAG_BILBOARD_STARTEND;
					if (pos & BILLBOARD_TRANS_MID)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
					else if (pos & BILLBOARD_TRANS_ADD)
						 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
					else if (text->m_globalAlpha != 0xFF)
					{
						bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
						bb->alpha = text->m_globalAlpha >> 4;
					}
					bb->size = road->VisualTextures[ visual_id ].BillboardHeight[2];
					bb->m_texture = text;
					OTTableAdd( bb);
					bb++;
				}
			}
		}

		// ---------------------------------------
		if (!reverse)
		{
			vertOut -= SECTION_NBVERT*2;
			curr_section--;
			if (curr_section < 0)
				curr_section = road->SectionNb - 1;
		}
		else
		{
			curr_section++;
			if (curr_section >= road->SectionNb)
				curr_section = 0;
		}
	}
}

#endif // ! USE_OGL

#ifdef USE_OGL

void CRender::RenderRoad(RoadStruct *road, int from, int to, int reverse, TTexture *texture, TTexture::sAtlasTextureTile *atlasTextureTiles, bool bRenderBillboards)
{
	PERF_COUNTER(CRender_RenderRoad);

	TVertex*		vertOut = road->Transformed;
	TBillboard* bb			= billboard_pool;
	int				nbSection;
	unsigned char	fogr,fogg,fogb;

	//TEST
	//util for billboards ... find the atlas texture where the material for this bilboard is included
//	TTexture *tempAtlasTexOpaque = CHighGear::GetInstance()->m_PlayingGame->m_Map->TrackTextureOpaque;
//	TTexture *tempAtlasTexAlpha = CHighGear::GetInstance()->m_PlayingGame->m_Map->TrackTextureAlpha;
//	TTexture::sAtlasTextureTile *tempAtlasTexOpaqueTile = CHighGear::GetInstance()->m_PlayingGame->m_Map->trackTextureOpaqueTiles;
//	TTexture::sAtlasTextureTile *tempAtlasTexAlphaTile = CHighGear::GetInstance()->m_PlayingGame->m_Map->trackTextureAlphaTiles;

	//reverse = true;

	fogr = road->FogColor[0] >> 3;
	fogg = road->FogColor[1] >> 2;
	fogb = road->FogColor[2] >> 3;

	// copy / transform road vertice (TBD optimize this by using a cycling buffer)
	//{
		int		nb;
		from = from - 1;
		//LAngelov: fixed a crash when driving reverse
		if(from < 0)
			from = road->SectionNb + from;

		if (to > from)
			nb = to - from;
		else
			nb = (road->SectionNb - from) + to;	// loop

		nbSection = nb;

		RoadSection		*sec = road->Section + from;
		RoadSection		*end = road->Section + road->SectionNb;

		//CMatrix44 matrix = m_matrixStack.CurrentMatrix();
		//matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

		//::glMatrixMode(GL_MODELVIEW);
		//::glPushMatrix();
		//::glLoadMatrixf(CMatrix44::s_matrixFloat);

		while (nb--)
		{
			// each section
			{
				long*		vertIn = sec->Vertex;
				int			v = SECTION_NBVERT;

				while (v--)
				{
					// copy
					vertOut->InitialPos.Init( *vertIn, *(vertIn+1), *(vertIn+2) );
					// transform
					//TransformVertex(vertOut);

					vertIn += 3;
					++vertOut;
				}
			}

			++sec;
			if (sec >= end)	// loop
				sec = road->Section;
		}
	//}

	int			curr_section;

	//--------------------------------------
	// DRAW Faces
	if (reverse)
	{
		// we draw in reverse from [from] to [to]
		vertOut = road->Transformed;
		curr_section = from;
	}
	else
	{
		// we draw from [to] to [from]
		vertOut = road->Transformed + (nbSection - 2) * SECTION_NBVERT;
		curr_section = to - 1;
		if (curr_section < 0)
			curr_section = road->SectionNb - 1;
	}

	int		nbSec = nbSection - 1;
	bool	zcorrect = false;
	bool	subdiv4 = false;
	bool	flat = true;

	int nRoadQuads = 0;
	int nFenceQuads = 0;

	f32* pVtx = s_pVtx;
	f32* pTex = s_pTex;

	f32* pFenceVtx = s_pVtx + (FACE_COMMON_FENCE_TRI_START_IDX * FACE_VTX_COUNT_PER_TRI * FACE_VTX_COMPONENT_COUNT);
	f32* pFenceTex = s_pTex + (FACE_COMMON_FENCE_TRI_START_IDX * FACE_VTX_COUNT_PER_TRI * FACE_TEX_COMPONENT_COUNT);

	f32** ppVtx = 0;
	f32** ppTex = 0;
	int*  pnRoadQuads = 0;

	//get the storage address for the variables
	if(texture->m_bHasAlphaChannel)
	{
		ppVtx = &pVtx;
		ppTex = &pTex;
		pnRoadQuads = &nRoadQuads;		
	}
	else
	{
		//separate opaque fences from opaque road
		ppVtx = &pFenceVtx;
		ppTex = &pFenceTex;
		pnRoadQuads = &nFenceQuads;		
	}


	f32*& pRefFenceVtx = *ppVtx;
	f32*& pRefFenceTex = *ppTex;
	int& nRefFenceQuads = *pnRoadQuads;	


//	f32	tile_u = 64.0f / texture->m_pow2Width;
//	f32	tile_v = 64.0f / texture->m_pow2Height;
	f32 textureInvW = 1.0f / texture->m_pow2Width;
	f32 textureInvH = 1.0f / texture->m_pow2Height;

	f32	texture_u;
	f32	texture_v;
	f32 err = textureInvH;

	f32 start_u;
	f32 start_v;

	f32 uv[8];
	f32 ftmp;

	while (nbSec--)
	{
		int		nbQuads = (SECTION_NBVERT-1);
		
		// flat distant polys
		if (nbSec == nbSection - 5)	// three fog polys
			flat = false;

		// zcorrect only 4 first section faces
		if (nbSec == 4)	//4
			zcorrect = true;

		// get current visual_id (for texture and billboards)
		int			visual_id;
		if (reverse)
		{
			visual_id = road->Section[curr_section].VisualId;
		}
		else
		{
			if (curr_section != 0)
				visual_id = road->Section[curr_section - 1].VisualId;
			else
				visual_id = road->Section[road->SectionNb - 1].VisualId;
		}

		TVertex* vOut = vertOut;
		TVertex* VxA;
		Vector3f vfA, vfB;

		while (nbQuads--)
		{
			int textId = road->VisualTextures[ visual_id ].MaterialIdx[ nbQuads ];
			int	flagX = road->VisualTextures[ visual_id ].flipX & (1 << nbQuads);
			int	flagY = road->VisualTextures[ visual_id ].flipY & (1 << nbQuads);
			int	tile = road->TextureTile[ textId ];

			if (textId != 0xFF && atlasTextureTiles[textId].id != 0xFF)
			{
				//textId = atlasTextureTiles[textId].id;
				//TTexture*	text = road->VisualTextures[ visual_id ].Textures[nbQuads];

				texture_u = atlasTextureTiles[textId].w * textureInvW;  //tile_u * text->SizeX() / 64.0f;
				texture_v = atlasTextureTiles[textId].h * textureInvH;  //tile_v * text->SizeY() / 64.0f;
				start_u = atlasTextureTiles[textId].x * textureInvW; // tile_u * (textId % 8);
				start_v = atlasTextureTiles[textId].y * textureInvH; // tile_v * (textId / 8);// + texture_v * t / tile;			

				f32 dx = ((vOut + SECTION_NBVERT)->InitialPos.x - vOut->InitialPos.x) / tile;
				f32 dy = ((vOut + SECTION_NBVERT)->InitialPos.y - vOut->InitialPos.y) / tile;
				f32 dz = ((vOut + SECTION_NBVERT)->InitialPos.z - vOut->InitialPos.z) / tile;

				f32 dx1 = ((vOut + SECTION_NBVERT + 1)->InitialPos.x - (vOut + 1)->InitialPos.x) / tile;
				f32 dy1 = ((vOut + SECTION_NBVERT + 1)->InitialPos.y - (vOut + 1)->InitialPos.y) / tile;
				f32 dz1 = ((vOut + SECTION_NBVERT + 1)->InitialPos.z - (vOut + 1)->InitialPos.z) / tile;

				vfA.x = (f32)vOut->InitialPos.x;
				vfA.y = (f32)vOut->InitialPos.y;
				vfA.z = (f32)vOut->InitialPos.z;
				vfB.x = (f32)(vOut + 1)->InitialPos.x;
				vfB.y = (f32)(vOut + 1)->InitialPos.y;
				vfB.z = (f32)(vOut + 1)->InitialPos.z;

				// The texture is first Y-flipped (is a bottom-top tga), then rotated right
				uv[0] = start_u + err;
				uv[1] = start_v + err;
				uv[2] = start_u + err;
				uv[3] = start_v + texture_v - err;
				uv[4] = start_u + texture_u - err;
				uv[5] = start_v + texture_v - err;
				uv[6] = start_u + texture_u - err;
				uv[7] = start_v + err;

				if (flagX)
				{
					ftmp = uv[1];
					uv[1] = uv[3];
					uv[3] = ftmp;

					ftmp = uv[5];
					uv[5] = uv[7];
					uv[7] = ftmp;
				}
				
				if (flagY)
				{
					ftmp = uv[0];
					uv[0] = uv[6];
					uv[6] = ftmp;

					ftmp = uv[2];
					uv[2] = uv[4];
					uv[4] = ftmp;
				}


				for (int t = 0; t<tile; ++t)
				{
					//VxA = vOut;

					*pVtx++ = vfA.x;
					*pVtx++ = vfA.y;
					*pVtx++ = vfA.z;

					*pTex++ = uv[0];
					*pTex++ = uv[1];

					//VxA = vOut + 1;

					*pVtx++ = vfB.x;
					*pVtx++ = vfB.y;
					*pVtx++ = vfB.z;

					*pTex++ = uv[2];
					*pTex++ = uv[3];

					//VxA = vOut + SECTION_NBVERT + 1;

					vfB.x += dx1;
					vfB.y += dy1;
					vfB.z += dz1;

					*pVtx++ = vfB.x;
					*pVtx++ = vfB.y;
					*pVtx++ = vfB.z;

					*pTex++ = uv[4];
					*pTex++ = uv[5];

					//VxA = vOut + SECTION_NBVERT;
					vfA.x += dx;
					vfA.y += dy;
					vfA.z += dz;

					*pVtx++ = vfA.x;
					*pVtx++ = vfA.y;
					*pVtx++ = vfA.z;

					*pTex++ = uv[6];
					*pTex++ = uv[7];

					++nRoadQuads;
				}
			}

			// Fence
			if ((nbQuads == 0) || (nbQuads == (SECTION_NBVERT-2)))
			{
				for (int two = 0; two < 2; two++)	// we have 2 fence on the quad
				{
					TVertex	*vertOut2;
					int		fence_idx;

					if (nbQuads == (SECTION_NBVERT-2))	// RIGHT
					{
						vertOut2 = vOut + two;
						fence_idx = 3 - two;
					}
					else	// LEFT
					{
						vertOut2 = vOut - two + 1;
						fence_idx = two;
					}

					int textId = road->VisualTextures[ visual_id ].MaterialIdx[8 + fence_idx];

					if (textId == 0xFF || atlasTextureTiles[textId].id == 0xFF)
						continue;

					int tile = road->TextureTile[ textId ];
					int	height = road->VisualTextures[ visual_id ].FenceHeight[ fence_idx ];

					//textId = atlasTextureTiles[textId].id;
					//TTexture*	text = road->VisualTextures[ visual_id ].Textures[fence_idx + 8];

					texture_u = atlasTextureTiles[textId].w * textureInvW; // tile_u * text->SizeX() / 64.0f;
					texture_v = atlasTextureTiles[textId].h * textureInvH; // tile_v * text->SizeY() / 64.0f;
					start_u = atlasTextureTiles[textId].x * textureInvW;; // tile_u * (textId % 8);
					start_v = atlasTextureTiles[textId].y * textureInvH;  // tile_v * (textId / 8);// + texture_v * t / tile;

					f32 dx = ((vertOut2 + SECTION_NBVERT)->InitialPos.x - vertOut2->InitialPos.x) / tile;
					f32 dy = ((vertOut2 + SECTION_NBVERT)->InitialPos.y - vertOut2->InitialPos.y) / tile;
					f32 dz = ((vertOut2 + SECTION_NBVERT)->InitialPos.z - vertOut2->InitialPos.z) / tile;

					vfA.x = (f32)vertOut2->InitialPos.x;
					vfA.y = (f32)vertOut2->InitialPos.y;
					vfA.z = (f32)vertOut2->InitialPos.z;

					for (int t = 0; t<tile; ++t)
					{
						uv[0] = start_u + texture_u - err;
						uv[1] = start_v + err;
						uv[2] = start_u + texture_u - err;
						uv[3] = start_v + texture_v - err;
						uv[4] = start_u + err;
						uv[5] = start_v + texture_v - err;
						uv[6] = start_u + err;
						uv[7] = start_v + err;

						*pRefFenceVtx++ = vfA.x;
						*pRefFenceVtx++ = vfA.y;
						*pRefFenceVtx++ = vfA.z;

						*pRefFenceTex++ = uv[0];
						*pRefFenceTex++ = uv[1];

						*pRefFenceVtx++ = vfA.x;
						*pRefFenceVtx++ = vfA.y + height;
						*pRefFenceVtx++ = vfA.z;

						*pRefFenceTex++ = uv[2];
						*pRefFenceTex++ = uv[3];

						vfA.x += dx;
						vfA.y += dy;
						vfA.z += dz;

						*pRefFenceVtx++ = vfA.x;
						*pRefFenceVtx++ = vfA.y + height;
						*pRefFenceVtx++ = vfA.z;

						*pRefFenceTex++ = uv[4];
						*pRefFenceTex++ = uv[5];

						*pRefFenceVtx++ = vfA.x;
						*pRefFenceVtx++ = vfA.y;
						*pRefFenceVtx++ = vfA.z;

						*pRefFenceTex++ = uv[6];
						*pRefFenceTex++ = uv[7];

						++nRefFenceQuads;
					}
				}
			}

			// NEXT QUAD
			++vOut;
		}

		// Top - tunnel
		int	textId = road->VisualTextures[ visual_id ].MaterialIdx[ROAD_MATERIAL_IDX_TUNNEL];	

		if (textId != 0xFF && atlasTextureTiles[textId].id != 0xFF)
		{
			//textId = atlasTextureTiles[textId].id;
			//TTexture*	text = road->VisualTextures[ visual_id ].Textures[ROAD_TEXTURE_IDX_TUNNEL];

			texture_u = atlasTextureTiles[textId].w * textureInvW; // tile_u * text->SizeX() / 64.0f;
			texture_v = atlasTextureTiles[textId].h * textureInvH; // tile_v * text->SizeY() / 64.0f;
			start_u = atlasTextureTiles[textId].x * textureInvW; // tile_u * (textId % 8);
			start_v = atlasTextureTiles[textId].y * textureInvH; // tile_v * (textId / 8);

			uv[0] = start_u + texture_u - err;
			uv[1] = start_v + texture_v - err;
			uv[2] = start_u + texture_u - err;
			uv[3] = start_v + err;
			uv[4] = start_u + err;
			uv[5] = start_v + err;
			uv[6] = start_u + err;
			uv[7] = start_v + texture_v - err;

			VxA = vOut;

			*pVtx++ = (f32)VxA->InitialPos.x;
			*pVtx++ = (f32)VxA->InitialPos.y;
			*pVtx++ = (f32)VxA->InitialPos.z;

			*pTex++ = uv[0];
			*pTex++ = uv[1];

			VxA = vOut - SECTION_NBVERT + 1;

			*pVtx++ = (f32)VxA->InitialPos.x;
			*pVtx++ = (f32)VxA->InitialPos.y;
			*pVtx++ = (f32)VxA->InitialPos.z;

			*pTex++ = uv[2];
			*pTex++ = uv[3];

			VxA = vOut + 1;

			*pVtx++ = (f32)VxA->InitialPos.x;
			*pVtx++ = (f32)VxA->InitialPos.y;
			*pVtx++ = (f32)VxA->InitialPos.z;

			*pTex++ = uv[4];
			*pTex++ = uv[5];

			VxA = vOut + SECTION_NBVERT;

			*pVtx++ = (f32)VxA->InitialPos.x;
			*pVtx++ = (f32)VxA->InitialPos.y;
			*pVtx++ = (f32)VxA->InitialPos.z;

			*pTex++ = uv[6];
			*pTex++ = uv[7];

			++nRoadQuads;
		}

		// NEXT QUAD
		++vOut;

		//////////////////////////////////////////////////////////////////////
		// Billboards - only with the transparent atlas texture... (maybe this needs to change)		
		if ( ( nbSec < (nbSection - 4) ) && bRenderBillboards )
		{
			//TTexture*	text;

			// 1st billboard

			//text = road->VisualTextures[ visual_id ].Textures[13];
			int textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 1];

			//if (text)
			//TEST
			//if (textureId != 0xFF && (tempAtlasTexAlphaTile[textureId].id != 0xFF || tempAtlasTexOpaqueTile[textureId].id != 0xFF))
			{
				int sizeY;
				int globalAlpha = 0xFF;
				//TEST
//				if (tempAtlasTexAlphaTile[textureId].id != 0xFF)
//				{
//					sizeY = tempAtlasTexAlphaTile[textureId].h;
//					globalAlpha = tempAtlasTexAlphaTile[textureId].globalAlpha;
//				}
//				else
//				{
//					sizeY = tempAtlasTexOpaqueTile[textureId].h;
//					globalAlpha = tempAtlasTexOpaqueTile[textureId].globalAlpha;
//				}

				Vector4s	v;

				int	height = sizeY + road->VisualTextures[ visual_id ].BillboardHeight[1];

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[1];
				if (pos & BILLBOARD_MID)
					v.Init( (vOut[-8].InitialPos.x + vOut[-9].InitialPos.x)/2, 
							(vOut[-8].InitialPos.y + vOut[-9].InitialPos.y)/2 + height, 
							(vOut[-8].InitialPos.z + vOut[-9].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vOut[-9].InitialPos.x, vOut[-9].InitialPos.y + height, vOut[-9].InitialPos.z );
				else
					v.Init( vOut[-8].InitialPos.x, vOut[-8].InitialPos.y + height, vOut[-8].InitialPos.z );

								
				bb->m_flag = TFACE_FLAG_BILLBOARD |  TFACE_FLAG_OFFSET_FRONT;
				if (pos & BILLBOARD_TRANS_MID)
					 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
				else if (pos & BILLBOARD_TRANS_ADD)
					 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
				else if (globalAlpha != 0xFF)
				{
					bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
					bb->alpha = globalAlpha >> 4;
				}
				bb->size = 256;

				//find atlas texture that contains this material id
				//int textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 1];
				
				bb->m_texture = NULL;

				//if( textureId != 0xFF )
				{
					//TEST
//					if( tempAtlasTexAlphaTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexAlphaTile[textureId].x;
//						bb->start_y = tempAtlasTexAlphaTile[textureId].y;
//						bb->end_x = tempAtlasTexAlphaTile[textureId].x + tempAtlasTexAlphaTile[textureId].w;
//						bb->end_y = tempAtlasTexAlphaTile[textureId].y + tempAtlasTexAlphaTile[textureId].h;
//						bb->m_texture = tempAtlasTexAlpha;					
//					}
//					else if( tempAtlasTexOpaqueTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexOpaqueTile[textureId].x;
//						bb->start_y = tempAtlasTexOpaqueTile[textureId].y;
//						bb->end_x = tempAtlasTexOpaqueTile[textureId].x + tempAtlasTexOpaqueTile[textureId].w;
//						bb->end_y = tempAtlasTexOpaqueTile[textureId].y + tempAtlasTexOpaqueTile[textureId].h;
//						bb->m_texture = tempAtlasTexOpaque;					
//					}
				}
				
				//TEST
				//add billboard to alpha groups
//				if( bb->m_texture != NULL && AddBillboardToAlphaAppGrps(*this, *bb, v) )
//				{
//					bb++;
//				}
			}

			//text = road->VisualTextures[ visual_id ].Textures[12];
			textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 0];
			//if (text)		
			//TEST
			//if (textureId != 0xFF && (tempAtlasTexAlphaTile[textureId].id != 0xFF || tempAtlasTexOpaqueTile[textureId].id != 0xFF))
			{
				int sizeY;
				int globalAlpha;
				//TEST
//				if (tempAtlasTexAlphaTile[textureId].id != 0xFF)
//				{
//					sizeY = tempAtlasTexAlphaTile[textureId].h;
//					globalAlpha = tempAtlasTexAlphaTile[textureId].globalAlpha;
//				}
//				else
//				{
//					sizeY = tempAtlasTexOpaqueTile[textureId].h;
//					globalAlpha = tempAtlasTexOpaqueTile[textureId].globalAlpha;
//				}

				Vector4s	v;

				int	height = sizeY + road->VisualTextures[ visual_id ].BillboardHeight[0];

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[0];
				if (pos & BILLBOARD_MID)
					v.Init(	(vOut[-1].InitialPos.x + vOut[-2].InitialPos.x)/2, 
							(vOut[-1].InitialPos.y + vOut[-2].InitialPos.y)/2 + height, 
							(vOut[-1].InitialPos.z + vOut[-2].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vOut[-1].InitialPos.x, vOut[-1].InitialPos.y + height, vOut[-1].InitialPos.z );
				else
					v.Init( vOut[-2].InitialPos.x, vOut[-2].InitialPos.y + height, vOut[-2].InitialPos.z );

				bb->m_flag = TFACE_FLAG_BILLBOARD |  TFACE_FLAG_OFFSET_FRONT;
				if (pos & BILLBOARD_TRANS_MID)
					bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
				else if (pos & BILLBOARD_TRANS_ADD)
					bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
				else if (globalAlpha != 0xFF)
				{
					bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
					bb->alpha = globalAlpha >> 4;
				}
				bb->size = 256;

				//find atlas texture that contains this material id
				//int textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 0];
				
				bb->m_texture = NULL;

				//if( textureId != 0xFF )
				{
					//TEST
//					if( tempAtlasTexAlphaTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexAlphaTile[textureId].x;
//						bb->start_y = tempAtlasTexAlphaTile[textureId].y;
//						bb->end_x = tempAtlasTexAlphaTile[textureId].x + tempAtlasTexAlphaTile[textureId].w;
//						bb->end_y = tempAtlasTexAlphaTile[textureId].y + tempAtlasTexAlphaTile[textureId].h;
//						bb->m_texture = tempAtlasTexAlpha;					
//					}
//					else if( tempAtlasTexOpaqueTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexOpaqueTile[textureId].x;
//						bb->start_y = tempAtlasTexOpaqueTile[textureId].y;
//						bb->end_x = tempAtlasTexOpaqueTile[textureId].x + tempAtlasTexOpaqueTile[textureId].w;
//						bb->end_y = tempAtlasTexOpaqueTile[textureId].y + tempAtlasTexOpaqueTile[textureId].h;
//						bb->m_texture = tempAtlasTexOpaque;					
//					}
				}
				
				//add billboard to alpha groups
				//TEST
//				if( bb->m_texture != NULL && AddBillboardToAlphaAppGrps(*this, *bb, v) )
//				{
//					bb++;
//				}
				
			}
			// 2nd billboard
			//text = road->VisualTextures[ visual_id ].Textures[16];
			textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 2 + 1 ];

			//if (text)
			//TEST
			//if (textureId != 0xFF && (tempAtlasTexAlphaTile[textureId].id != 0xFF || tempAtlasTexOpaqueTile[textureId].id != 0xFF))
			{
				int sizeY;
				int globalAlpha;
				//TEST
//				if (tempAtlasTexAlphaTile[textureId].id != 0xFF)
//				{
//					sizeY = tempAtlasTexAlphaTile[textureId].h;
//					globalAlpha = tempAtlasTexAlphaTile[textureId].globalAlpha;
//				}
//				else
//				{
//					sizeY = tempAtlasTexOpaqueTile[textureId].h;
//					globalAlpha = tempAtlasTexOpaqueTile[textureId].globalAlpha;
//				}

				Vector4s	v;

				int	height = ((sizeY - 2) * road->VisualTextures[ visual_id ].BillboardHeight[3]) / 128;

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[3];
				if (pos & BILLBOARD_MID)
					v.Init( (vOut[-8].InitialPos.x + vOut[-9].InitialPos.x)/2, 
							(vOut[-8].InitialPos.y + vOut[-9].InitialPos.y)/2 + height, 
							(vOut[-8].InitialPos.z + vOut[-9].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vOut[-9].InitialPos.x, vOut[-9].InitialPos.y + height, vOut[-9].InitialPos.z );
				else
					v.Init( vOut[-8].InitialPos.x, vOut[-8].InitialPos.y + height, vOut[-8].InitialPos.z );

				
				bb->start_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[2];
				bb->end_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[3];
				bb->m_flag = TFACE_FLAG_BILLBOARD/* |  TFACE_FLAG_OFFSET_FRONT*/ | TFACE_FLAG_BILBOARD_STARTEND;
				if (pos & BILLBOARD_TRANS_MID)
					 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
				else if (pos & BILLBOARD_TRANS_ADD)
					 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
				else if (globalAlpha != 0xFF)
				{
					bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
					bb->alpha = globalAlpha >> 4;
				}
				bb->size = road->VisualTextures[ visual_id ].BillboardHeight[3];
				
				//find atlas texture that contains this material id
				//int textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 2 + 1 ];
				
				bb->m_texture = NULL;

				//if( textureId != 0xFF )
				{
					//TEST
//					if( tempAtlasTexAlphaTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexAlphaTile[textureId].x;
//						bb->start_y = tempAtlasTexAlphaTile[textureId].y;
//						bb->end_x = tempAtlasTexAlphaTile[textureId].x + tempAtlasTexAlphaTile[textureId].w;
//						bb->end_y = tempAtlasTexAlphaTile[textureId].y + tempAtlasTexAlphaTile[textureId].h;
//						bb->m_texture = tempAtlasTexAlpha;					
//					}
//					else if( tempAtlasTexOpaqueTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexOpaqueTile[textureId].x;
//						bb->start_y = tempAtlasTexOpaqueTile[textureId].y;
//						bb->end_x = tempAtlasTexOpaqueTile[textureId].x + tempAtlasTexOpaqueTile[textureId].w;
//						bb->end_y = tempAtlasTexOpaqueTile[textureId].y + tempAtlasTexOpaqueTile[textureId].h;
//						bb->m_texture = tempAtlasTexOpaque;					
//					}
				}
				
				//TEST
				//add billboard to alpha groups
//				if( bb->m_texture != NULL && AddBillboardToAlphaAppGrps(*this, *bb, v) )
//				{
//					bb++;
//				}					
			}

			//text = road->VisualTextures[ visual_id ].Textures[15];
			textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 2 ];

			//if (text)
			//TEST
			//if (textureId != 0xFF && (tempAtlasTexAlphaTile[textureId].id != 0xFF || tempAtlasTexOpaqueTile[textureId].id != 0xFF))
			{
				int sizeY;
				int globalAlpha;
				//TEST
//				if (tempAtlasTexAlphaTile[textureId].id != 0xFF)
//				{
//					sizeY = tempAtlasTexAlphaTile[textureId].h;
//					globalAlpha = tempAtlasTexAlphaTile[textureId].globalAlpha;
//				}
//				else
//				{
//					sizeY = tempAtlasTexOpaqueTile[textureId].h;
//					globalAlpha = tempAtlasTexOpaqueTile[textureId].globalAlpha;
//				}

				Vector4s	v;

				int	height = ((sizeY - 2) * road->VisualTextures[ visual_id ].BillboardHeight[2]) / 128;

				int		pos = road->VisualTextures[ visual_id ].BillboardPos[2];
				if (pos & BILLBOARD_MID)
					v.Init(	(vOut[-1].InitialPos.x + vOut[-2].InitialPos.x)/2, 
							(vOut[-1].InitialPos.y + vOut[-2].InitialPos.y)/2 + height, 
							(vOut[-1].InitialPos.z + vOut[-2].InitialPos.z)/2 );
				else if (pos & BILLBOARD_EXT)
					v.Init( vOut[-1].InitialPos.x, vOut[-1].InitialPos.y + height, vOut[-1].InitialPos.z );
				else
					v.Init( vOut[-2].InitialPos.x, vOut[-2].InitialPos.y + height, vOut[-2].InitialPos.z );

				
				bb->start_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[0];
				bb->end_x = road->VisualTextures[ visual_id ].Billboard2StartEnd[1];
				bb->m_flag = TFACE_FLAG_BILLBOARD/* |  TFACE_FLAG_OFFSET_FRONT*/ | TFACE_FLAG_BILBOARD_STARTEND;
				if (pos & BILLBOARD_TRANS_MID)
					 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_MID;
				else if (pos & BILLBOARD_TRANS_ADD)
					 bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ADD;
				else if (globalAlpha != 0xFF)
				{
					bb->m_flag |= TFACE_FLAG_BILBOARD_TRANS_ALPHA;
					bb->alpha = globalAlpha >> 4;
				}
				bb->size = road->VisualTextures[ visual_id ].BillboardHeight[2];

				//find atlas texture that contains this material id
				//int textureId = road->VisualTextures[ visual_id ].MaterialIdx[ ROAD_MATERIAL_IDX_BILLBOARD + 2 ];
				
				bb->m_texture = NULL;

				//if( textureId != 0xFF )
				{
					//TEST
			//		if( tempAtlasTexAlphaTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexAlphaTile[textureId].x;
//						bb->start_y = tempAtlasTexAlphaTile[textureId].y;
//						bb->end_x = tempAtlasTexAlphaTile[textureId].x + tempAtlasTexAlphaTile[textureId].w;
//						bb->end_y = tempAtlasTexAlphaTile[textureId].y + tempAtlasTexAlphaTile[textureId].h;
//						bb->m_texture = tempAtlasTexAlpha;					
//					}
//					else if( tempAtlasTexOpaqueTile[textureId].id != 0xFF )
//					{
//						bb->m_flag |= TFACE_FLAG_BILBOARD_STARTEND;
//						bb->start_x = tempAtlasTexOpaqueTile[textureId].x;
//						bb->start_y = tempAtlasTexOpaqueTile[textureId].y;
//						bb->end_x = tempAtlasTexOpaqueTile[textureId].x + tempAtlasTexOpaqueTile[textureId].w;
//						bb->end_y = tempAtlasTexOpaqueTile[textureId].y + tempAtlasTexOpaqueTile[textureId].h;
//						bb->m_texture = tempAtlasTexOpaque;					
//					}
				}
				
				//TEST
				//add billboard to alpha groups
//				if( bb->m_texture != NULL && AddBillboardToAlphaAppGrps(*this, *bb, v) )
//				{
//					bb++;
//				}	
			}
		} //billboards

		

		// ---------------------------------------
		if (!reverse)
		{
			vertOut -= SECTION_NBVERT;
			curr_section--;
			if (curr_section < 0)
				curr_section = road->SectionNb - 1;
		}
		else
		{
			vertOut += SECTION_NBVERT;
			curr_section++;
			if (curr_section >= road->SectionNb)
				curr_section = 0;
		}
	}

	if (nRoadQuads == 0)
	{
		return;
	}
	
	if(!texture->m_bHasAlphaChannel)
	{
		//separate opaque fences from opaque road
		A_ASSERT( (nRoadQuads * TRI_COUNT_PER_QUAD) <= (FACE_COMMON_TRI_COUNT - FACE_COMMON_FENCE_TRI_COUNT));
		A_ASSERT( (nFenceQuads * TRI_COUNT_PER_QUAD) <= FACE_COMMON_FENCE_TRI_COUNT);
	}
	else
	{
		A_ASSERT( (nRoadQuads * TRI_COUNT_PER_QUAD) <= FACE_COMMON_TRI_COUNT );	
	}

	s_nOpaqueFenceQuads = nFenceQuads;

	//OpenGL Hardware Render
	//enable depth test and write to depth buffer
	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_LEQUAL);
	::glDepthMask(true);	
	::glDisable(GL_ALPHA_TEST);
	
	if (texture->m_bHasAlphaChannel)
	{
		::glEnable(GL_BLEND);
		::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		::glDisable(GL_BLEND);
	}

	::glEnableClientState(GL_VERTEX_ARRAY);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY); //ACTIVATE texture

#ifdef USE_VERTEX_COLOR_TEST
	::glEnableClientState(GL_COLOR_ARRAY);		   //ACTIVATE color
	::glColorPointer(4, GL_UNSIGNED_BYTE, 0, s_pColor);
#else
	::glDisableClientState(GL_COLOR_ARRAY);		   //DEACTIVATE color
#endif

	::glVertexPointer(3, GL_FLOAT, 0, s_pVtx);
	::glTexCoordPointer(2, GL_FLOAT, 0, s_pTex );		

	::glEnable( GL_TEXTURE_2D );	

	::glBindTexture(GL_TEXTURE_2D, texture->m_glTextureName);

#ifdef USE_VERTEX_COLOR_TEST
	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#else
	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif

	CMatrix44 matrix = m_matrixStack.CurrentMatrix();
	matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadMatrixf(CMatrix44::s_matrixFloat);

	::glDrawElements( GL_TRIANGLES, TRI_COUNT_PER_QUAD * VTX_COUNT_PER_TRI * nRoadQuads, GL_UNSIGNED_SHORT, s_pIndicesTris);

	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);

	::glPopMatrix();

#ifdef WIN32
	gNbFacesDrawn += nRoadQuads * TRI_COUNT_PER_QUAD;
#endif // WIN32
}

#endif /* USE_OGL */

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

void CRender::RenderMeshOTFace(TVertex* vertex,int nbVertex,TFace* faces,int i_nbFaces,CBoard3D& board, int type,bool doubleFaced )
{
	static const struct {int near_flag;int normal_flag;} kFlags[]=
	{
		{		//MESH_OT_TYPE_BG
			TFACE_FLAG_SUBDIV | TFACE_FLAG_SUBDIV4 | TFACE_FLAG_ZCORRECTED,
			TFACE_FLAG_CULL | TFACE_FLAG_ZCORRECTED
		},	
		{		//MESH_OT_TYPE_TRAFIC
			TFACE_FLAG_SUBDIV | TFACE_FLAG_ZCORRECTED,
			TFACE_FLAG_CULL
		},
		{		//MESH_OT_TYPE_PLAYER
			TFACE_FLAG_SUBDIV | TFACE_FLAG_ZCORRECTED | TFACE_FLAG_ENVMAP,
			TFACE_FLAG_CULL |/*| TFACE_FLAG_ZCORRECTED|*/ TFACE_FLAG_ENVMAP
		},
		{		//MESH_OT_TYPE_COCKPIT
			TFACE_FLAG_SUBDIV | TFACE_FLAG_ZCORRECTED | TFACE_FLAG_OFFSET_FRONT_MORE,
			TFACE_FLAG_CULL | TFACE_FLAG_OFFSET_FRONT_MORE,			
		},
		{		//MESH_OT_TYPE_GHOST
			TFACE_FLAG_SUBDIV | TFACE_FLAG_ZCORRECTED | TFACE_FLAG_GHOST,
			TFACE_FLAG_CULL | TFACE_FLAG_GHOST
		},
		{	   // LOD
			0,0
		},
		{		//MESH_OT_TYPE_PLAYER_GARAGE
			TFACE_FLAG_SUBDIV | TFACE_FLAG_ZCORRECTED, // | TFACE_FLAG_OLDENVMAP,
			TFACE_FLAG_CULL | TFACE_FLAG_SUBDIV, // | TFACE_FLAG_OLDENVMAP
		}
	};

	TransformVertex_Test(vertex,vertex+nbVertex);

	if (m_andFlag != 0)
	{
#if defined WIN32
		otface_reject_nb = i_nbFaces;
#endif
		return;
	}

	int		OrFlag, AndFlag;

	int		nbface = i_nbFaces;
	TFace*	F = faces;

	A_ASSERT(type >= 0 && type < MESH_OT_TYPE___COUNT);

	const int near_flag = kFlags[type].near_flag;
	const int normal_flag = kFlags[type].normal_flag;

	while (nbface--)
	{
		TVertex *VxA = F->VertexA();
		TVertex *VxB = F->VertexB();
		TVertex *VxC = F->VertexC();

		OrFlag = (VxA->Vmask() | VxB->Vmask()) | VxC->Vmask();

		if (OrFlag & kRejectFar)		// one vertex too far behind NEAR_CLIP
		{
 			F++;
			continue;                 // disable display
		}

		// if f != 0 then face is not visible in frustrum
		AndFlag = (VxA->Vmask() & VxB->Vmask()) & VxC->Vmask();  

		if (AndFlag)
		{
			F++;
			continue;
		}

		if(!doubleFaced && type != 0 && F->CullingTest())
		{
			F++;
#if defined WIN32
			if (type == 0)
				otface_culled_bg_nb++;
			otface_culled_nb++;
#endif
			continue;
		}

		if (OrFlag & kRejectNear)
		{
			//F->m_flag = near_flag;
			// temp method for envmap only on face initialized with enmap flag
			if (F->m_flag & (TFACE_FLAG_ENVMAP | TFACE_FLAG_OLDENVMAP))
				F->m_flag = near_flag;
			else
				F->m_flag = near_flag & (~(TFACE_FLAG_ENVMAP|TFACE_FLAG_OLDENVMAP));
		}
		else				
		{
			//F->m_flag = normal_flag;
			// temp method for envmap only on face initialized with enmap flag
			if (F->m_flag & (TFACE_FLAG_ENVMAP | TFACE_FLAG_OLDENVMAP))
				F->m_flag = normal_flag;
			else
				F->m_flag = normal_flag & (~(TFACE_FLAG_ENVMAP | TFACE_FLAG_OLDENVMAP));
		}

		if (type == MESH_OT_TYPE_BG || type == MESH_OT_TYPE_PLAYER_GARAGE)
		{
			if (F->m_texture->m_globalAlpha != 0xFF)
			{
				F->m_flag |= TFACE_FLAG_TRANS_ADDITIVE_ALPHA | TFACE_FLAG_OFFSET_FRONT;
				F->m_flag &= ~TFACE_FLAG_ZCORRECTED;
			}

			if (F->m_texture->m_mask)
			{
				F->m_flag |= TFACE_FLAG_ALPHA_MASK | TFACE_FLAG_OFFSET_FRONT;
				F->m_flag &= ~TFACE_FLAG_ZCORRECTED;
			}
		}

		A_ASSERT((F->m_flag & TFACE_FLAG_TRANS_SUBSTRATIVE)==0);

#ifdef USE_OGL
		OTTableAdd( F, ( F->m_flag & 0xFFFFFFFF & ( TFACE_FLAG_ALPHA_MASK | TFACE_FLAG_TRANS_ADDITIVE_ALPHA | TFACE_FLAG_TRANS | TFACE_FLAG_TRANS_ADDITIVE )) == 0 );
#else /* USE_OGL */
		OTTableAdd( F );
#endif /* USE_OGL */

		F++;
	}
}
// ---------------------------------------------------------------------------
// reject faces not visible in frustrum, and flag face requiring subdivision
// CamPosition is used to test culling, NULL can be passed if object is mobile
// ---------------------------------------------------------------------------

void CRender::RenderMeshOTFace(CMesh *Mesh,CBoard3D& board, int type )
{
	RenderMeshOTFace(Mesh->FirstVertex(),Mesh->NbVertex(),Mesh->FirstFace(),Mesh->NbFaces(),board,type,Mesh->IsDoubleFaced());
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
static bool CircleLineTest(	Vector4s::Type	planeNormalx,
														Vector4s::Type	planeNormaly,
														Vector4s::Type	centrex,
														Vector4s::Type	centrey,
														int							radius)
{
	int centre2x = centrex - ((planeNormalx * radius) >> NFRUST_SHIFT);
	int centre2y = centrey - ((planeNormaly * radius) >> NFRUST_SHIFT);

	int dot = planeNormalx * centre2x + planeNormaly * centre2y;
	if (dot > CLIP_REJECT)
		return false;
	else
		return true;
}
									

// ---------------------------------------------------------------------------
//	pos must be in CameraSpace
// ---------------------------------------------------------------------------
bool CRender::TestSphereInFrustum(const Vector4s& pos,int radius) const
{
	if ((pos.z + radius) < FAR_CLIP)
		return false;

	if ((pos.z - radius) > NEAR_CLIP)
		return false;

	if(!::CircleLineTest(m_frustrumBTLR[2].x,m_frustrumBTLR[2].z,pos.x,pos.z,radius))
		return false;

	if(!::CircleLineTest(m_frustrumBTLR[3].x,m_frustrumBTLR[3].z,pos.x,pos.z,radius))
		return false;

	if(!::CircleLineTest(m_frustrumBTLR[0].y,m_frustrumBTLR[2].z,pos.y,pos.z,radius))
		return false;

	if(!::CircleLineTest(m_frustrumBTLR[1].y,m_frustrumBTLR[3].z,pos.y,pos.z,radius))
		return false;

	return true;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
// Not used
unsigned int CRender::FrustrumRejectionTest(const Vector4s& v) const
{
	Vector4s	v2;
	CurrentMatrix().TransformVector(&v,&v2);

	unsigned int reject = 0x00;

	if (v2.z < FAR_CLIP)
		reject |= kRejectFar;

#ifndef USE_OGL

	if (v2.z > NEAR_CLIP)                        // too near
		reject |= kRejectNear;

	int dot;

	dot = m_frustrumBTLR[0].y * v2.y + m_frustrumBTLR[0].z * v2.z;
	if (dot > CLIP_REJECT)
		reject |= kRejectB;

	dot = m_frustrumBTLR[1].y * v2.y + m_frustrumBTLR[1].z * v2.z;
	if (dot > CLIP_REJECT)
		reject |= kRejectT;

	dot = m_frustrumBTLR[2].x * v2.x + m_frustrumBTLR[2].z * v2.z;
	if (dot > CLIP_REJECT)
		reject |= kRejectL;

	dot = m_frustrumBTLR[3].x * v2.x + m_frustrumBTLR[3].z * v2.z;
	if (dot > CLIP_REJECT)
		reject |= kRejectR;

#endif /* !USE_OGL*/

	return reject;
}

// ============================================================================
// OT TABLE STUFF
// ============================================================================

void CRender::OTTableClear()
{
#if defined WIN32
//	TRACE2("OT faces: %d %d %d %d", otface_nb, otface_reject_nb, otface_culled_nb, otface_culled_bg_nb);
	otface_nb = 0;
	otface_reject_nb = 0;
	otface_culled_nb = 0;
	otface_culled_2_nb = 0;
	otface_culled_bg_nb = 0;
#endif

	memset( OTTable, 0, sizeof(OTTable) );

#if USE_OTTABLE_GROUPS
	memset(OTTableUsage, 0, sizeof(OTTableUsage));
#endif
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifdef USE_OGL

void CRender::OTTableAdd(COTObject* obj, bool bIgnoreZRenderFirst )

#else /* USE_OGL */

void CRender::OTTableAdd(COTObject* obj)

#endif /* USE_OGL */

{
	//map Z values to OTTable index
	//idx OTTable?    ...    ABS(obj z far value)
	//ot_table_SIZE	...    z_buffer_SIZE
	//idx = z * ot_tableSize / z_bufferSIZE

	int	idx = obj->GetZForOrderingTable() >> (Z_BUFFER_BITS - OT_TABLE_BITS);

	if (obj->m_flag & TFACE_FLAG_OFFSET_FRONT)
		idx -= 16;
	else if (obj->m_flag & TFACE_FLAG_OFFSET_BACK)
		idx += 16;
//	else if (obj->m_flag & TFACE_FLAG_OFFSET_MIN)
//		idx = 0;

	if (idx < 0 )
		idx = 0;
	else if (idx >= OT_TABLE_SIZE)	// should not happen, but does on wrong scale roads
	{
//		A_ASSERT(false);
		return;
	}

	//faces to be first rendered
#ifdef USE_OGL
	if( bIgnoreZRenderFirst )
	{
		idx = OT_TABLE_SIZE - ( 1 << kOOtableShift ) - 1;
	}	
#endif /* USE_OGL */

	// test - sort
	//COTObject *oo = OTTable[ idx ];
	//COTObject *oo2 = oo;
	//while (oo != NULL && oo->GetZForOrderingTable() < obj->GetZForOrderingTable())
	//{
	//	oo2 = oo;
	//	oo = oo->m_next_object;
	//}
	//obj->m_next_object = oo;
	//if (oo2 == NULL || oo2 == oo)
	//	OTTable[ idx ] = obj;
	//else oo2->m_next_object = obj;	

	obj->m_next_object = OTTable[ idx ];
	OTTable[ idx ] = obj;

#if USE_OTTABLE_GROUPS
	//++OTTableUsage[idx >> kOOtableShift];
	OTTableUsage[idx >> kOOtableShift] = 1;
#endif

#if defined WIN32
	otface_nb++;
#endif
}



#if WIN_DEBUG
class OTTableStats
{
public:
	OTTableStats()
	{
		m_nbr=0;
		memset(m_val,0,sizeof(m_val));
	}
	~OTTableStats()
	{
		int i;

		int total = 0 ;

		for(i=0;i<OT_TABLE_SIZE;i++)
		{
			total += m_val[i];
			m_val2[i] = total;
		}

		if(m_nbr)
		{
			FILE* f = fopen("d:\\ottable.txt","wt");
			if(f)
			{
				for(i=0;i<OT_TABLE_SIZE;i++)
					fprintf(f,"%4d %4d %4f %4f %4f\n",	i,
													m_val[i],
													float(m_val[i])/float(m_nbr),
													float(m_val2[i])/float(total)	,
													1.0f - float(m_val2[i])/float(total)
													);
				fclose(f);
			}
		}
	}

	int m_nbr;
	int m_val[OT_TABLE_SIZE];
	int m_val2[OT_TABLE_SIZE];

};

static OTTableStats _gOttableStats; 


#endif

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifdef USE_OGL

void CRender::OTTableDraw(CLib3D& lib3d,  bool bRenderOnlyFarthest )
{

#if USE_OTTABLE_GROUPS
	
	//needed to parse the static allocated arrays
	int vtxIdx = 0;
	int nCurrVtxAdded = 0;
	f32* temp_pVtx = s_pVtx;
	f32* temp_pTex = s_pTex;
	u8*  temp_pColor = s_pColor;
	u16* temp_pIndices = s_pIndices;
	OTGroupInfo* temp_pOTGroupInfo = s_pOTGroupInfo;
	//initialize first element
	temp_pOTGroupInfo[0].m_startVtxIdx = 0;
	temp_pOTGroupInfo[0].m_vtxCount = 0;


	const int kNbrGroups		= OT_TABLE_SIZE >> kOOtableShift;
	const int kNbElemPerGroup	= 1 << kOOtableShift;
	

	if( bRenderOnlyFarthest )
	{
		//render only the object with zIdx = OT_TABLE_SIZE - ( 1 << kOOtableShift ) - 1
		int ottable_idx = OT_TABLE_SIZE - ( 1 << kOOtableShift ) - 1;
		COTObject**	t = OTTable + ottable_idx;

		COTObject*	object = *t;			
		
		//draw objects with same index in OTTABLE ... ( almost same z value )
		while(object)
		{
			A_ASSERT(object->m_flag != 0xFFFFFFFF);

			object->AddToOGLRenderingPipeline(nCurrVtxAdded, temp_pOTGroupInfo, temp_pVtx, temp_pTex, vtxIdx, temp_pIndices, temp_pColor);

			vtxIdx += nCurrVtxAdded;				

			object = object->m_next_object;
		}

		//mark as rendered
		*t = NULL;
	}
	else
	{
		int i = kNbrGroups - 2;

		while (i >= 0)
		{
			if (OTTableUsage[i] > 0)
			{
				COTObject**	t = OTTable + ( (i+1) << kOOtableShift) - 1;

				COTObject** endObject = t - kNbElemPerGroup;
				while(t != endObject)
				{
					COTObject*	object = *t;			

					//draw objects with same index in OTTABLE ... ( almost same z value )
					while(object)
					{
						A_ASSERT(object->m_flag != 0xFFFFFFFF);

						object->AddToOGLRenderingPipeline(nCurrVtxAdded, temp_pOTGroupInfo, temp_pVtx, temp_pTex, vtxIdx, temp_pIndices, temp_pColor);

						vtxIdx += nCurrVtxAdded;				

						object = object->m_next_object;
					}

					*t = NULL;
					--t;
				}
			}

			OTTableUsage[i] = 0;
			--i;
		}
	}
	
	int vtxIndicesCount = (temp_pIndices - s_pIndices);

	//OpenGL Hardware Render
	if( vtxIndicesCount > 0)
	{

		//if failed please increase FACE_COMMON_TRI_COUNT
		A_ASSERT( vtxIndicesCount <= (FACE_COMMON_TRI_COUNT * FACE_VTX_COUNT_PER_TRI) );
		
		//add also the last group ... +1
		int nbGroups = temp_pOTGroupInfo + 1 - s_pOTGroupInfo;
		
		temp_pOTGroupInfo = s_pOTGroupInfo;
		
		//enable depth test and write to depth buffer
		::glEnable(GL_DEPTH_TEST);
		::glDepthFunc(GL_LEQUAL);
		::glDepthMask(true);
	
		::glDisable(GL_ALPHA_TEST);

		::glEnableClientState(GL_VERTEX_ARRAY);
		::glVertexPointer(3, GL_FLOAT, 0, s_pVtx);

		
		while(nbGroups--)
		{	
			if( CHECK_GRP_INFO_DISABLE_DEPTH_TEST(temp_pOTGroupInfo->m_flags) )
			{
				::glDisable(GL_DEPTH_TEST);
			}
			else
			{
				::glEnable(GL_DEPTH_TEST);
			}
				 
			if( CHECK_GRP_INFO_IS_BILLBOARD(temp_pOTGroupInfo->m_flags) )
			{
				if( !CHECK_GRP_INFO_DISABLE_DEPTH_TEST(temp_pOTGroupInfo->m_flags) )
				{
					::glEnable(GL_POLYGON_OFFSET_FILL);
					::glPolygonOffset(0.0f, -100.0f);
					::glDepthMask(false);
				}
			}
			else
			{
				 ::glDisable(GL_POLYGON_OFFSET_FILL);
				 ::glDepthMask(true);
			}


			if( CHECK_GRP_INFO_HAS_COLOR(temp_pOTGroupInfo->m_flags) )
			{	
				::glEnableClientState(GL_COLOR_ARRAY);			//ACTIVATE color
				::glColorPointer(4, GL_UNSIGNED_BYTE, 0, s_pColor);

				::glDisable( GL_TEXTURE_2D );
				::glDisableClientState(GL_TEXTURE_COORD_ARRAY); //DEACTIVATE texture				
			}
			
			else if( CHECK_GRP_INFO_HAS_TEXTURE( temp_pOTGroupInfo->m_flags ) )			
			{
				if( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_TRANS )
				{
					::glEnable(GL_BLEND); //use blend

					if(  ( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_ADDITIVE ) && 
						 ( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_GLB_ALPHA )
					  )
					{
						::glBlendFunc( GL_SRC_ALPHA, GL_ONE );
					}
					else if( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_ADDITIVE )
					{
						// suppose that 0 is transparent color
						::glBlendFunc(GL_ONE, GL_ONE);
					}
					else if( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_GLB_ALPHA )
					{
						::glColor4ub(0xFF, 0xFF, 0xFF, temp_pOTGroupInfo->m_pTexOrColor->m_globalAlpha & 0xFF);
						::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					}
					else
					{
						//only trans
						::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					}
				}
				else
				{
					if(  temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_ADDITIVE )
					{
						::glEnable(GL_BLEND);
						::glBlendFunc(GL_ONE, GL_ONE);
					}
					else if( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_GLB_ALPHA )
					{
						::glEnable(GL_BLEND);						
						::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						::glColor4ub(0xFF, 0xFF, 0xFF, temp_pOTGroupInfo->m_pTexOrColor->m_globalAlpha & 0xFF);
					}
					else
					{
						::glDisable(GL_BLEND);
					}
				}


				::glEnable( GL_TEXTURE_2D );
				::glEnableClientState(GL_TEXTURE_COORD_ARRAY); //ACTIVATE texture
				::glTexCoordPointer(2, GL_FLOAT, 0, s_pTex );
				
				::glDisableClientState(GL_COLOR_ARRAY);		   //DEACTIVATE color				

				::glBindTexture( GL_TEXTURE_2D, temp_pOTGroupInfo->m_pTexOrColor->m_glTextureName );

				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				if( temp_pOTGroupInfo->m_flags & FLAG_GRP_INFO_USE_GLB_ALPHA )
				{
					::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				}
				else
				{
					::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				}

			}
			else
			{
				//no texture no color ... skip this ... to do
				temp_pOTGroupInfo++;
				continue;
			}

			
			::glDrawElements( GL_TRIANGLES, temp_pOTGroupInfo->m_vtxCount, GL_UNSIGNED_SHORT, s_pIndices + temp_pOTGroupInfo->m_startVtxIdx);
			
			::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			::glDisableClientState(GL_COLOR_ARRAY);

			//next group
			temp_pOTGroupInfo++;

#ifdef WIN32
			gNbFacesDrawn += temp_pOTGroupInfo->m_vtxCount / 3;
#endif // WIN32
		}

		::glDisable(GL_POLYGON_OFFSET_FILL);
		::glDisableClientState(GL_VERTEX_ARRAY);
		::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

	} //vtxIdx > 0
	

#endif

#if defined WIN32
	otface_nb = 0;
	otface_reject_nb = 0;
	otface_culled_nb = 0;
	otface_culled_2_nb = 0;
	otface_culled_bg_nb = 0;
#endif
}

#else /* USE_OGL */

void CRender::OTTableDraw(CLib3D& lib3d )
{
	PERF_COUNTER(OTTableDraw);

#if USE_OTTABLE_GROUPS
	const int kNbrGroups		= OT_TABLE_SIZE >> kOOtableShift;
	const int kNbElemPerGroup	= 1 << kOOtableShift;


#if WIN_DEBUG
	_gOttableStats.m_nbr++;
#endif

	//int i = kNbrGroups-1;
	int i = kNbrGroups - 2;
#ifdef	SHOW_FPS
	int countDrawnObjects=0;
#endif

	while (i >= 0)
	{
		if (OTTableUsage[i] > 0)
		{
			COTObject**	t = OTTable + ( (i+1) << kOOtableShift) - 1;

			COTObject** endObject = t - kNbElemPerGroup;
			while(t != endObject)// && OTTableUsage[i] > 0)
			{
				COTObject*	object = *t;
				#if WIN_DEBUG
					if(i==0)
						int set_breakPoint_here=0;

					if(object)
						_gOttableStats.m_val[i]++;
				#endif		

				while(object)
				{
					A_ASSERT(object->m_flag != 0xFFFFFFFF);
				#ifdef	SHOW_FPS
					countDrawnObjects++;
				#endif
					object->Draw(lib3d);
					object = object->m_next_object;

					//--OTTableUsage[i];
				}

				*t = NULL;
				--t;
			}			
		}

		OTTableUsage[i] = 0;
		--i;
	}
#ifdef	SHOW_FPS
//	CHighGear::GetInstance()->countDrawnObjects = countDrawnObjects; 

#endif
#else //PERF_COUNTER(OTTableDraw);
	COTObject**	t = OTTable + OT_TABLE_SIZE;

	int		i = OT_TABLE_SIZE;
	while (i--)
	{
		#if WIN_DEBUG
			if(i==0)
				int set_breakPoint_here=0;
		#endif

		COTObject* object = *(--t);
		while(object)
		{
			A_ASSERT(object->m_flag != 0xFFFFFFFF);

			object->Draw(lib3d);
			object = object->m_next_object;
		}
	}	
#endif

#if defined WIN32
//	TRACE2("OT faces: %d %d %d %d", otface_nb, otface_reject_nb, otface_culled_nb, otface_culled_2_nb);
	otface_nb = 0;
	otface_reject_nb = 0;
	otface_culled_nb = 0;
	otface_culled_2_nb = 0;
	otface_culled_bg_nb = 0;
#endif
}

#endif /* USE_OGL */

///////////////////////////////////////////////////////////////////////////////////////

void CRender::OTTableDrawReverse(CLib3D& lib3d )
{
	PERF_COUNTER(OTTableDraw);

#if USE_OTTABLE_GROUPS
	const int kNbrGroups		= OT_TABLE_SIZE >> kOOtableShift;
	const int kNbElemPerGroup	= 1 << kOOtableShift;

#if WIN_DEBUG
	_gOttableStats.m_nbr++;
#endif

	int i = 0;
#ifdef	SHOW_FPS
	int countDrawnObjects=0;
#endif

	while (i < kNbrGroups)
	{
		if (OTTableUsage[i] > 0)
		{
			COTObject**	t = OTTable + ( (i+1) << kOOtableShift) - 1;

			COTObject** endObject = t - kNbElemPerGroup;
			while(t != endObject)// && OTTableUsage[i] > 0)
			{
				COTObject*	object = *t;
				#if WIN_DEBUG
					if(i==0)
						int set_breakPoint_here=0;

					if(object)
						_gOttableStats.m_val[i]++;
				#endif		

				while(object)
				{
					A_ASSERT(object->m_flag != 0xFFFFFFFF);
				#ifdef	SHOW_FPS
					countDrawnObjects++;
				#endif
					object->Draw(lib3d);
					object = object->m_next_object;

					//--OTTableUsage[i];
				}
				--t;
			}
		}		
		++i;
	}
#endif
}










