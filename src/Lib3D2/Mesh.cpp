#pragma warning(disable:4786)

#include "Lib3D.h"
#include "Mesh.h"
#include "File.h"
#include "FSqrt.h"
//#include "Texture.h"
#include "DevUtil.h"

#include <string.h>
#include <stdlib.h>

#ifdef WIN32
	extern int gNbFacesDrawn;
#endif

using namespace Lib3D;

const int	kWorldSize		= 1 << 14;

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CMesh::CMesh(	const char* name,
				int nbVertex,TVertex* vertex,
				int nbFaces,TFace* face,
				int lengthFront, int lengthRear, int halfWidth,bool doubleFaced)
	:	m_nbVertex(nbVertex),
		m_vertex(vertex),
		m_nbFaces(nbFaces),
		m_face(face),
		m_lengthFront(lengthFront),
		m_lengthRear(lengthRear),
		m_halfWidth(halfWidth),
		m_doubleFaced(doubleFaced)
{
	m_vertexKeyframes = NULL;
	m_nbKeyframes = 0;
	m_lastKeyframe1 = -1;
	m_lastKeyframe2 = -1;
	m_lastFactor = -1;
}


CMesh::~CMesh()
{
	DELETE_ARRAY m_vertex;
	DELETE_ARRAY m_face;  
	if (m_vertexKeyframes)
		DELETE_ARRAY m_vertexKeyframes;
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CMesh::ClearFaces()
{
	DELETE_ARRAY m_face;
	m_face = NULL;
	m_nbFaces = 0;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CMesh::GetLimits(Vector4s *Min, Vector4s *Max) const
{
	if(NbVertex() >0)
	{
		Min->Init(&m_vertex[0].InitialPos);
		Max->Init(Min);

		for (int i=1; i<NbVertex(); i++)
		{
			Min->GetMin(&m_vertex[i].InitialPos);
			Max->GetMax(&m_vertex[i].InitialPos);
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CMesh::AnimationAlloc( int nbKeyframes )
{
	A_ASSERT(m_vertexKeyframes==0);

	m_nbKeyframes = nbKeyframes;
	m_vertexKeyframes = NEW Vector4s[m_nbKeyframes * m_nbVertex];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CMesh::AnimationLoad( int keyframeIndex, Vector4s* keyframeVert )
{
	A_ASSERT( keyframeIndex < m_nbKeyframes );

	// copy the vertex to the keyframe
	memcpy( m_vertexKeyframes + keyframeIndex * m_nbVertex, keyframeVert, sizeof( Vector4s ) * m_nbVertex );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CMesh::AnimationPlay( int frame1, int frame2, int factor )	// factor between frame1 and frame 2 from 0 - 4096 
{
	A_ASSERT( frame1 < m_nbKeyframes );
	A_ASSERT( frame2 < m_nbKeyframes );
	
	if ( factor < 0 )
		factor = 0;
	if ( factor > 4096 )
		factor = 4096;

	// prevent copy if not needed
	if ((frame1 == m_lastKeyframe1) && (frame1 == m_lastKeyframe1) && (factor == m_lastFactor))
	{
		// nothing to do :)
		return;
	}

	// save parameters
	m_lastKeyframe1 = frame1;
	m_lastKeyframe2 = frame2;
	m_lastFactor = factor;


	int	nbv = m_nbVertex;

	// check no interpolation cases first
	if (factor == 0)
	{
		// frame 1
		Vector4s*	in = m_vertexKeyframes + frame1 * m_nbVertex;
		TVertex*	out = m_vertex;

		while (nbv--)
		{
			out->InitialPos.x = in->x;
			out->InitialPos.y = in->y;
			out->InitialPos.z = in->z;	// we don't need s
			out++;
			in++;
		}
	}
	else
	if (factor == 4096)
	{
		// frame 2
		Vector4s*	in = m_vertexKeyframes + frame2 * m_nbVertex;
		TVertex*	out = m_vertex;

		while (nbv--)
		{
			out->InitialPos.x = in->x;
			out->InitialPos.y = in->y;
			out->InitialPos.z = in->z;	// we don't need s
			out++;
			in++;
		}
	}
	else
	{
		// interpolation between frame1 and frame2
		Vector4s*	in1 = m_vertexKeyframes + frame1 * m_nbVertex;
		Vector4s*	in2 = m_vertexKeyframes + frame2 * m_nbVertex;
		TVertex*	out = m_vertex;
		int			factor2 = 4096 - factor;

		while (nbv--)
		{
			out->InitialPos.x = (in1->x * factor2 + in2->x * factor) / 4096;
			out->InitialPos.y = (in1->y * factor2 + in2->y * factor) / 4096;
			out->InitialPos.z = (in1->z * factor2 + in2->z * factor) / 4096;

			in1++;
			in2++;
			out++;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CMesh::AnimationPlay3( int frame1, int frame2, int frame3, int alpha1, int alpha2, int alpha3 )
{
	A_ASSERT( frame1 < m_nbKeyframes );
	A_ASSERT( frame2 < m_nbKeyframes );
	A_ASSERT( frame3 < m_nbKeyframes );

	Vector4s const * in1 = m_vertexKeyframes + frame1 * m_nbVertex;
	Vector4s const * in2 = m_vertexKeyframes + frame2 * m_nbVertex;
	Vector4s const * in3 = m_vertexKeyframes + frame3 * m_nbVertex;
	TVertex * out = m_vertex;
	Vector4s const * end1 = in1 + m_nbVertex;
	while (in1 != end1)
	{
		out->InitialPos = ((alpha1 * *in1) + (alpha2 * *in2) + (alpha3 * *in3)) >> 12;
		++ in1;
		++ in2;
		++ in3;
		++ out;
	}
}

#ifdef USE_OGL

//f32 CMesh::m_pVtx[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_VTX_COMPONENT_COUNT ] = {0};
//f32 CMesh::m_pTex[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_TEX_COMPONENT_COUNT ] = {0};
////u8  CMesh::m_pColor[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_COLOR_COMPONENT_COUNT ] = {0};
//u16 CMesh::m_pIndices[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI ] = {0};
//
////file scope
////car box on ZX
static f32 s_quadVtx[4 * 3] = {0};
const  u16 s_quadIndices[6] = { 0, 1, 2, 2, 1, 3};

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE

//void CMesh::DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
//{
//	TVertex * firstVertex = FirstVertex();
//
//	f32* pVtx = m_pVtx;
//	u16* pIndices = m_pIndices;	
//	
//	u16 index = 0;
//
//	for (int i=0; i<NbFaces(); i++)
//	{
//		TFace currentFace = m_face[i];
//
//		int idxVertexA =  currentFace.VertexA() - firstVertex;
//		int idxVertexB =  currentFace.VertexB() - firstVertex;
//		int idxVertexC =  currentFace.VertexC() - firstVertex;
//
//		//vertexA
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.z;		
//
//		*pIndices++ = index++;
//		
//		//vertexB
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.z;		
//
//		*pIndices++ = index++;
//
//		//vertexC
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.z;		
//
//		*pIndices++ = index++;
//	}
//	
//	::glDisable( GL_TEXTURE_2D );	
//	::glDisable(GL_DEPTH_TEST);
//	::glDisableClientState(GL_COLOR_ARRAY);
//	::glEnableClientState(GL_VERTEX_ARRAY);	
//
//	::glVertexPointer(3, GL_FLOAT, 0, m_pVtx);	
//	
//	::glColor4ub(red, green, blue, alpha);	
//	::glEnable(GL_BLEND);
//	::glBlendFunc(GL_ONE, GL_ZERO);	
//	::glDrawElements( GL_TRIANGLES, (pIndices - m_pIndices), GL_UNSIGNED_SHORT, m_pIndices);
//	
//	::glDisableClientState(GL_VERTEX_ARRAY);
//	::glDisable(GL_BLEND);	
//	
//	::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
//}

#else //USE_PROJECT_SHADOW_INTO_TEXTURE

//void CMesh::DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
//{
//	TVertex * firstVertex = FirstVertex();
//
//	f32* pVtx = m_pVtx;
//	//f32* pTex = m_pTex;
//	//u8*  pColor = m_pColor;
//	u16* pIndices = m_pIndices;	
//
//	//f32 aspectRatioW = 1.0f / (FirstFace()->m_texture->m_pow2Width * (1<<4));
//	//f32 aspectRatioH = 1.0f / (FirstFace()->m_texture->m_pow2Height * (1<<4));	
//
//	u16 index = 0;
//
//	for (int i=0; i<NbFaces(); i++)
//	{
//		TFace currentFace = m_face[i];
//
//		int idxVertexA =  currentFace.VertexA() - firstVertex;
//		int idxVertexB =  currentFace.VertexB() - firstVertex;
//		int idxVertexC =  currentFace.VertexC() - firstVertex;
//
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.z;		
//
//		//*pTex++ = currentFace.GetuA() * aspectRatioW;
//		//*pTex++ = currentFace.GetvA() * aspectRatioW;
//
//		////col for vertex A
//		//*pColor++ = 0xFF;	//R
//		//*pColor++ = 0x0;	//G
//		//*pColor++ = 0x0;	//B
//		//*pColor++ = 0xFF;	//A
//
//		*pIndices++ = index++;
//		
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.z;		
//
//		//*pTex++ = currentFace.GetuB() * aspectRatioW;
//		//*pTex++ = currentFace.GetvB() * aspectRatioW;
//
//		////col for vertex A
//		//*pColor++ = 0xFF;	//R
//		//*pColor++ = 0x0;	//G
//		//*pColor++ = 0x0;	//B
//		//*pColor++ = 0xFF;	//A
//
//		*pIndices++ = index++;
//
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.z;
//
//		//*pTex++ = currentFace.GetuC() * aspectRatioW;
//		//*pTex++ = currentFace.GetvC() * aspectRatioW;
//
//		////col for vertex A
//		//*pColor++ = 0xFF;	//R
//		//*pColor++ = 0x0;	//G
//		//*pColor++ = 0x0;	//B
//		//*pColor++ = 0xFF;	//A
//
//		*pIndices++ = index++;
//	}
//	
//	::glDisable(GL_ALPHA_TEST);
//	
//	::glDisable( GL_TEXTURE_2D );	
//	::glDisableClientState(GL_COLOR_ARRAY);
//
//	::glEnableClientState(GL_VERTEX_ARRAY);
//	
//
//	s_quadVtx[0]= -m_halfWidth - 50;	s_quadVtx[1] = 0; s_quadVtx[2] = -m_lengthFront - 50;
//	s_quadVtx[3]= m_halfWidth + 50;	s_quadVtx[4] = 0; s_quadVtx[5] = -m_lengthFront - 50;
//	s_quadVtx[6]= -m_halfWidth - 50;		s_quadVtx[7] = 0; s_quadVtx[8]   = m_lengthRear + 50;
//	s_quadVtx[9]= m_halfWidth + 50;		s_quadVtx[10] = 0; s_quadVtx[11] = m_lengthRear + 50;
//
//	::glVertexPointer(3, GL_FLOAT, 0, s_quadVtx);
//	
//	//not implemented on windows pvr lib
//	::glColor4ub(255, 0, 0, 0);
//	
//	//draw a deep quad ... so shadow will pass allways the test over the road
//	::glEnable(GL_DEPTH_TEST);
//	::glDepthFunc(GL_ALWAYS);
//	::glDepthMask(true);
//	::glDisable(GL_BLEND);
//	::glEnable(GL_POLYGON_OFFSET_FILL);
//	::glPolygonOffset(0.0f, +100.0f);
//	::glColorMask( false, false, false, false );
//	::glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_quadIndices);	
//	::glDisable(GL_POLYGON_OFFSET_FILL);
//	
//	::glVertexPointer(3, GL_FLOAT, 0, m_pVtx);
//	
//	::glEnable(GL_CULL_FACE);
//	::glCullFace(GL_BACK);
//	
//	
//	::glEnable(GL_DEPTH_TEST);
//	::glDepthFunc(GL_LEQUAL);
//	::glDepthMask(true);
//	::glColor4ub(0, 0, 0, alpha);
//	::glDisable(GL_BLEND);
//	::glColorMask( false, false, false, true );
//	::glDrawElements( GL_TRIANGLES, (pIndices - m_pIndices), GL_UNSIGNED_SHORT, m_pIndices);
//	
//	::glDepthFunc(GL_EQUAL);
//	::glDepthMask(false);	
//	::glColor4ub(red, green, blue, 0);	
//	::glEnable(GL_BLEND);	
//	::glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);	
//	::glColorMask(true, true, true, true);
//	::glDrawElements( GL_TRIANGLES, (pIndices - m_pIndices), GL_UNSIGNED_SHORT, m_pIndices);
//	
//	::glDisableClientState(GL_VERTEX_ARRAY);
//	::glDisable(GL_CULL_FACE);
//	::glDisable(GL_BLEND);
//	
//	::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
//}
#endif //USE_PROJECT_SHADOW_INTO_TEXTURE


//void CMesh::Draw()
//{
//	TVertex * firstVertex = FirstVertex();
//
//	f32* pVtx = m_pVtx;
//	f32* pTex = m_pTex;
//	//u8*  pColor = m_pColor;
//	u16* pIndices = m_pIndices;	
//
//	u16 index = 0;
//
//	::glEnable(GL_DEPTH_TEST);
//	::glDisable(GL_ALPHA_TEST);
//	::glEnable(GL_CULL_FACE);
//
//	::glEnableClientState(GL_VERTEX_ARRAY);
//	::glVertexPointer(3, GL_FLOAT, 0, m_pVtx);
//
//	::glEnable(GL_BLEND);
//	::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	::glEnable( GL_TEXTURE_2D );
//	::glEnableClientState( GL_TEXTURE_COORD_ARRAY ); //ACTIVATE texture
//	::glTexCoordPointer( 2, GL_FLOAT, 0, m_pTex );
//	
//	for (int i=0; i<NbFaces(); i++)
//	{
//		TFace currentFace = m_face[i];
//
//		f32 aspectRatioW = 1.0f / (currentFace.m_texture->m_pow2Width * (1<<4));
//		f32 aspectRatioH = 1.0f / (currentFace.m_texture->m_pow2Height * (1<<4));	
//
//
//		int idxVertexA =  currentFace.VertexA() - firstVertex;
//		int idxVertexB =  currentFace.VertexB() - firstVertex;
//		int idxVertexC =  currentFace.VertexC() - firstVertex;
//
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexA].InitialPos.z;		
//
//		*pTex++ = currentFace.GetuA() * aspectRatioW;
//		*pTex++ = currentFace.GetvA() * aspectRatioW;
//
//		*pIndices++ = index++;
//		
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexB].InitialPos.z;		
//
//		*pTex++ = currentFace.GetuB() * aspectRatioW;
//		*pTex++ = currentFace.GetvB() * aspectRatioW;
//
//		*pIndices++ = index++;
//
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.x;
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.y;
//		*pVtx++ = (f32)m_vertex[idxVertexC].InitialPos.z;
//
//		*pTex++ = currentFace.GetuC() * aspectRatioW;
//		*pTex++ = currentFace.GetvC() * aspectRatioW;
//
//		*pIndices++ = index++;
//
//		::glBindTexture(GL_TEXTURE_2D, currentFace.m_texture->m_glTextureName);
//		::glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, pIndices);
//	}
//
//	::glDisableClientState(GL_VERTEX_ARRAY);
//	::glDisable(GL_CULL_FACE);
//	::glDisable(GL_BLEND);
//}

#endif /* USE_GL */


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

#ifdef USE_OGL

CGLMesh::CGLMesh(int nbMaterials, int nbVertex, int *nbIndices, f32 *vertex, f32 *tex, f32* normals, u16 **indices, const TTexture **textures, int textures_nb, int lengthFront, int lengthRear, int halfWidth)
	: m_nbMaterials(nbMaterials),
	m_nbVertex(nbVertex),
	m_nbIndices(nbIndices),
	m_pVertex(vertex),
	m_pTex(tex),
	m_pEnvTexCoord(NULL),
	m_pNormals(normals),
	m_pIndices(indices),
	m_lengthFront(lengthFront),
	m_lengthRear(lengthRear),
	m_halfWidth(halfWidth),
	m_pTextures(NULL),
	m_pEnvTexture(NULL),
	m_nbTextures(textures_nb),
	m_pColors(NULL),
	m_flags(0),
	m_globalColor(0)
{
	if (textures != NULL)
	{
		//m_nbTextures = min(m_nbTextures, m_nbMaterials);
		m_nbTextures = m_nbTextures < m_nbMaterials ? m_nbTextures : m_nbMaterials; 

		m_pTextures = new TTexture*[m_nbTextures];
		for (int i=0; i<m_nbTextures; ++i)
			m_pTextures[i] = (TTexture *) textures[i];
	}
	else
	{
		m_nbTextures = 0;
	}

	m_flags |= GLMESH_FLAG_CULLING;

	m_vertexKeyframes = NULL;
	m_nbKeyframes = 0;
	m_lastKeyframe1 = -1;
	m_lastKeyframe2 = -1;
	m_lastFactor = -1;

	m_skipRenderIndices = new char[m_nbMaterials];
	memset(m_skipRenderIndices, 0, m_nbMaterials);
}

CGLMesh::~CGLMesh()
{
	SAFE_DELETE_ARRAY(m_pVertex);
	SAFE_DELETE_ARRAY(m_pTex);
	SAFE_DELETE_ARRAY(m_pEnvTexCoord);
	SAFE_DELETE_ARRAY(m_pNormals);
	SAFE_DELETE_ARRAY(m_nbIndices);	
	SAFE_DELETE_ARRAY(m_pColors);

	if (m_pIndices)
	{
		for (int i=0; i<m_nbMaterials; ++i)
			SAFE_DELETE_ARRAY(m_pIndices[i]);
		SAFE_DELETE_ARRAY(m_pIndices);
	}

	SAFE_DELETE_ARRAY(m_pTextures);
	// the textures are references

	SAFE_DELETE_ARRAY(m_vertexKeyframes);
	SAFE_DELETE_ARRAY(m_skipRenderIndices);
}

// ---------------------------------------------------------------------------
/*
void CGLMesh::SetVertexColors(int color)
{
	if (!m_pColors)
		m_pColors = new u8[m_nbVertex * 4];

	u8 a = (color >> 24) & 0xFF;
	u8 r = (color >> 16) & 0xFF;
	u8 g = (color >>  8) & 0xFF;
	u8 b = (color >>  0) & 0xFF;

	u8 *colors = m_pColors;
	for (int i=0; i < m_nbVertex; i++)
	{
		*colors++ = r;
		*colors++ = g;
		*colors++ = b;
		*colors++ = a;
	}
}
*/

void CGLMesh::SetGlobalColor(int color)
{
	m_globalColor = color;
	m_flags |= GLMESH_FLAG_GLOBAL_COLOR;
	::glColor4ub(0, 0, 0, 0x48);
}

// ---------------------------------------------------------------------------
static const int k_MAX_VTX_NB = 4000; // rax: 2000 not enough, increased to 4000
static unsigned char s_mapVtxID_To_NeedComputing[k_MAX_VTX_NB] = { 1 };

void CGLMesh::ComputeSphereTexCoord(int materialID, const CMatrix44 &matrix)
{

	const int nIndices = m_nbIndices[materialID];
	if (nIndices <= 0)
		return;
	
	if(m_pEnvTexCoord == NULL || m_pNormals == NULL)
		return;

	A_ASSERT(m_nbVertex <= k_MAX_VTX_NB);

	memset(s_mapVtxID_To_NeedComputing, 1, k_MAX_VTX_NB);

	u16* pIndices		= m_pIndices[materialID];
	f32* pVtx			= m_pVertex;
	f32* pNormals		= m_pNormals;
	f32* pEnvTexCoord	= m_pEnvTexCoord;
	Vector4s r, normal, normalRot, view, vtx;

	//int count = 0;

	for(int idx = nIndices - 1; idx >= 0; idx--)
	{
		int vtxIdx = pIndices[idx];

		if(s_mapVtxID_To_NeedComputing[vtxIdx] == 0)
			continue;
		s_mapVtxID_To_NeedComputing[vtxIdx] = 0; //mark this vtx as computed

		vtx.x = pVtx[vtxIdx * 3 + 0]; 
		vtx.y = pVtx[vtxIdx * 3 + 1]; 
		vtx.z = pVtx[vtxIdx * 3 + 2];
		normal.x = pNormals[vtxIdx * 3 + 0] * COS_SIN_MUL; 
		normal.y = pNormals[vtxIdx * 3 + 1] * COS_SIN_MUL; 
		normal.z = pNormals[vtxIdx * 3 + 2] * COS_SIN_MUL;

		//int a = normal.x * normal.x + normal.y * normal.y  + normal.z * normal.z;
		//int b = FSqrt(a);

		matrix.RotateVector(&normal, &normalRot);
		matrix.TransformVector(&vtx, &view);

		view.Normalize();
		normalRot.Normalize();

		//compute reflection vector ... save in view
		// R = V - 2(V'.N')N'

		int dot = Vector4s::Dot(view, normalRot);
		// dot = prjAonB * B' ... prjAonB = dot / B' ... but B' is unit
		dot >>= COS_SIN_SHIFT;

	    normalRot.Scale(2*dot);
			
		view.Sub(&normalRot);
		
		//compute spherical coords
		// p = sqrt(Rx^2+Ry^2+(Rz+1)^2) = Rx^2 + Ry^2 + Rz^2 + 2*Rz*1 + 1^2 .... 1<=> COS_SIN_MUL
		// u = 1/2 + Rx/2p
		// v = 1/2 + Ry/2p

		f32 len2 = view.Length2();
		f32 inv2p = 0.5f / ( Lib3D::FSqrt(len2 + (view.z<<(1 + COS_SIN_SHIFT)) + (COS_SIN_MUL * COS_SIN_MUL) ) );	
		
		f32 vx = view.x * inv2p;
		f32 vy = view.y * inv2p;

		// check if it's outside of the 0.5 circle
		f32 radius2 = vx * vx + vy * vy;
		if (radius2 >= 0.25f)
		{
			//count++;
			f32 alpha = atan2(vy, vx);
			vx = 0.5f * cos(alpha);
			vy = 0.5f * sin(alpha);
		}

		//save uv
		pEnvTexCoord[vtxIdx * 2 + 0] = 0.5f + vx;
		pEnvTexCoord[vtxIdx * 2 + 1] = 0.5f + vy;
	}

	//dpf ("count: %d\n", count);
}

void CGLMesh::Draw(const CMatrix44 &matrix)
{
	::glEnable(GL_DEPTH_TEST);
	if (m_flags & GLMESH_FLAG_DEPTH_FUNC_ALWAYS)
		::glDepthFunc(GL_ALWAYS);
	else
		::glDepthFunc(GL_LEQUAL);
	::glDepthMask(true);	
	::glDisable(GL_ALPHA_TEST);	
	
	bool bBlend;

	if (m_flags & GLMESH_FLAG_BLENDING)
	{
		::glEnable(GL_BLEND);
		::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		bBlend = true;
	}
	else
	{
		::glDisable(GL_BLEND);
		bBlend = false;
	}
	
	if (m_flags & GLMESH_FLAG_CULLING)
	{
		::glEnable(GL_CULL_FACE);
		::glCullFace(GL_BACK);
	}

	::glEnable( GL_TEXTURE_2D );

	::glEnableClientState(GL_VERTEX_ARRAY);
	::glVertexPointer(3, GL_FLOAT, 0, m_pVertex);

	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);		//ACTIVATE texture	
	::glTexCoordPointer(2, GL_FLOAT, 0, m_pTex );

	if (m_flags & GLMESH_FLAG_VERTEX_COLOR)
	{
		::glEnableClientState(GL_COLOR_ARRAY);		   //ACTIVATE color
		::glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_pColors);
	}
	else
		::glDisableClientState(GL_COLOR_ARRAY);		   //DEACTIVATE color	

	if (m_pNormals && (m_flags & CGLMesh::GLMESH_FLAG_NORMALS))
	{
		::glEnableClientState(GL_NORMAL_ARRAY);
		::glNormalPointer(GL_FLOAT, 0, m_pNormals );
	}

	if (m_flags & GLMESH_FLAG_GLOBAL_COLOR)
	{
		// TODO: doesn't seem to work
		const unsigned char a = (m_globalColor >> 24) & 0xFF;
		const unsigned char r = (m_globalColor >> 16) & 0xFF;
		const unsigned char g = (m_globalColor >>  8) & 0xFF;
		const unsigned char b = (m_globalColor >>  0) & 0xFF;
		::glColor4ub(r, g, b, a);
	}

	matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadMatrixf(CMatrix44::s_matrixFloat);

	for (int i=0; i<m_nbTextures; ++i)
	{
		if (m_skipRenderIndices[i])
			continue;

		const int nIndices = m_nbIndices[i];

		if (nIndices <= 0)
			continue;

#ifdef WIN32
		gNbFacesDrawn += nIndices / 3;
#endif

		// If blending not forced, check texture for transparency
		if ((m_flags & GLMESH_FLAG_BLENDING) == 0)
		{
			if (m_pTextures[i]->m_bHasAlphaChannel)
			{
				if (!bBlend)
				{
					::glEnable(GL_BLEND);
					::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					bBlend = true;
				}			
			}
			else
			{
				if (!bBlend)
				{
					::glDisable(GL_BLEND);

					bBlend = false;
				}
			}
		}

		::glBindTexture(GL_TEXTURE_2D, m_pTextures[i]->m_glTextureName);
		if (bBlend)
			::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		else
			::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		::glDrawElements( GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, m_pIndices[i]);

	}

	::glDisable(GL_CULL_FACE);

	::glDisableClientState(GL_NORMAL_ARRAY);
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);
	::glDisableClientState(GL_COLOR_ARRAY);

	::glPopMatrix();
}

void CGLMesh::DrawEnvMap(const CMatrix44 &matrix)
{
	bool bUseEnvTexture = false;
	
	if(m_pNormals != NULL && m_pEnvTexture != NULL)
		bUseEnvTexture = true;

	if(bUseEnvTexture)
		ComputeSphereTexCoord(0, matrix);

	::glEnable(GL_DEPTH_TEST);
	if (m_flags & GLMESH_FLAG_DEPTH_FUNC_ALWAYS)
		::glDepthFunc(GL_ALWAYS);
	else
		::glDepthFunc(GL_LEQUAL);
	::glDepthMask(true);	
	::glDisable(GL_ALPHA_TEST);	
	
	bool bBlend;

	if (m_flags & GLMESH_FLAG_BLENDING)
	{
		::glEnable(GL_BLEND);
		::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		bBlend = true;
	}
	else
	{
		::glDisable(GL_BLEND);
		bBlend = false;
	}
	
	if (m_flags & GLMESH_FLAG_CULLING)
	{
		::glEnable(GL_CULL_FACE);
		::glCullFace(GL_BACK);
	}
	
	::glEnableClientState(GL_VERTEX_ARRAY);	
	::glVertexPointer(3, GL_FLOAT, 0, m_pVertex);

	::glDisableClientState(GL_COLOR_ARRAY);		   //DEACTIVATE color

	//first stage
	::glActiveTexture(GL_TEXTURE0);	
	::glEnable( GL_TEXTURE_2D );

	::glClientActiveTexture(GL_TEXTURE0);
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY); //ACTIVATE texture		
	::glTexCoordPointer(2, GL_FLOAT, 0, m_pTex );
	
	if (m_pNormals && (m_flags & CGLMesh::GLMESH_FLAG_NORMALS))
	{
		::glEnableClientState(GL_NORMAL_ARRAY);
		::glNormalPointer(GL_FLOAT, 0, m_pNormals );
	}

	matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadMatrixf(CMatrix44::s_matrixFloat);

	for (int i=0; i<m_nbTextures; ++i)
	{
		if (m_skipRenderIndices[i])
			continue;

		const int nIndices = m_nbIndices[i];

		if (nIndices <= 0)
			continue;

#ifdef WIN32
		gNbFacesDrawn += nIndices / 3;
#endif

		// If blending not forced, check texture for transparency
		if ((m_flags & GLMESH_FLAG_BLENDING) == 0)
		{
			if (m_pTextures[i]->m_bHasAlphaChannel)
			{
				if (!bBlend)
				{
					::glEnable(GL_BLEND);
					::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					bBlend = true;
				}			
			}
			else
			{
				if (!bBlend)
				{
					::glDisable(GL_BLEND);

					bBlend = false;
				}
			}
		}
			
		//to GL_TEXTURE0
		::glBindTexture(GL_TEXTURE_2D, m_pTextures[i]->m_glTextureName);
		if (bBlend)
			::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		else
			::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		
		if(bUseEnvTexture && i == 0)
		{
			if(m_envReflectIntensity == 1.0f)
			{
				//consinder light color or vertex color
				::glActiveTexture( GL_TEXTURE1 );
				::glEnable( GL_TEXTURE_2D );

				::glClientActiveTexture( GL_TEXTURE1 );
				::glEnableClientState(GL_TEXTURE_COORD_ARRAY); //ACTIVATE texture		
				::glTexCoordPointer(2, GL_FLOAT, 0, m_pEnvTexCoord );
						
				::glBindTexture(GL_TEXTURE_2D, m_pEnvTexture->m_glTextureName);			

				::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD); //arg

				::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			
				::glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
				::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			
				//GLfloat constColor[4] = {0.0f, 0.0f, 0.0f, 0.3f};
				//glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);			
				//
				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				//glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE); //arg
				//
				//glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				//glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				//
				//glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
				//glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				//
				//glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
				//glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
			}
			else
			{
				A_ASSERT(m_envReflectIntensity >= 0.0f && m_envReflectIntensity < 1.0f);
				
				::glTexCoordPointer(2, GL_FLOAT, 0, m_pEnvTexCoord );

				::glBindTexture(GL_TEXTURE_2D, m_pEnvTexture->m_glTextureName);					

				//set the intesity for the env texture ... TO DO ... consider also the light and vertex color 
				//u8 intensity = ((int)(m_envReflectIntensity * 255)) & 0xFF;
				//::glColor4ub(intensity, intensity, intensity, 0xFF); //ignored when lighting ... vertexColor igonored

				//::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				GLfloat constColor[4] = {m_envReflectIntensity, m_envReflectIntensity, m_envReflectIntensity, m_envReflectIntensity};
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);

				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE); //arg
	
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				
				if(m_envRotAngle != 0.0f)
				{
					::glMatrixMode(GL_TEXTURE);
					::glPushMatrix();
					::glLoadIdentity();
					::glTranslatef(0.5f, 0.5f, 0.0f);
					::glRotatef(m_envRotAngle, 0.0f, 0.0f, -1.0f);
					::glTranslatef(-0.5f, -0.5f, 0.0f);
				}				

				::glActiveTexture( GL_TEXTURE1 );
				::glEnable( GL_TEXTURE_2D );

				::glClientActiveTexture( GL_TEXTURE1 );
				::glEnableClientState(GL_TEXTURE_COORD_ARRAY); //ACTIVATE texture		
				::glTexCoordPointer(2, GL_FLOAT, 0, m_pTex );
						
				::glBindTexture(GL_TEXTURE_2D, m_pTextures[i]->m_glTextureName);		

				::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD); //arg

				::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			
				::glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
				::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			}
			
		}

		::glDrawElements( GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, m_pIndices[i]);

		if(bUseEnvTexture && i == 0)
		{
			//RESTORE TO ONE TEX UNIT
			::glDisable( GL_TEXTURE_2D );
			::glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			
			::glActiveTexture(GL_TEXTURE0);
			::glClientActiveTexture(GL_TEXTURE0);

			if(m_envReflectIntensity != 1.0f)
			{
				::glTexCoordPointer(2, GL_FLOAT, 0, m_pTex );
				
				if(m_envRotAngle != 0.0f)
				{
					::glMatrixMode(GL_TEXTURE);
					::glPopMatrix();
					::glMatrixMode(GL_MODELVIEW);
				}
			}			
		}//if(bUseEnvTexture && i == 0)
	}

	::glDisable(GL_CULL_FACE);

	::glDisableClientState(GL_NORMAL_ARRAY);
	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	::glDisableClientState(GL_VERTEX_ARRAY);

	::glPopMatrix();
}

Vector4s CGLMesh::GetVector(int index)
{
	A_ASSERT(index >= 0 && index < m_nbVertex); 

	index *= 3;
	Vector4s vec(
		(long)*(m_pVertex + index),
		(long)*(m_pVertex + index + 1),
		(long)*(m_pVertex + index + 2));

	return vec;
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE

void CGLMesh::DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	::glDisable( GL_TEXTURE_2D );	
	::glDisable(GL_DEPTH_TEST);
	::glDisableClientState(GL_COLOR_ARRAY);
	::glEnableClientState(GL_VERTEX_ARRAY);	

	::glVertexPointer(3, GL_FLOAT, 0, m_pVertex);
	
	::glColor4ub(red, green, blue, alpha);	
	::glDisable(GL_BLEND);
	
	for (int i=0; i<m_nbMaterials; ++i)
	{
		const int nIndices = m_nbIndices[i];

		if (nIndices <= 0)
			continue;

#ifdef WIN32
		gNbFacesDrawn += nIndices / 3;
#endif

		::glDrawElements( GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, m_pIndices[i]);
	}
	
	::glDisableClientState(GL_VERTEX_ARRAY);	
	::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
}

#else //USE_PROJECT_SHADOW_INTO_TEXTURE

void CGLMesh::DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, const CMatrix44* animMat)
{	
	::glDisable(GL_ALPHA_TEST);
	
	::glDisable( GL_TEXTURE_2D );
	::glDisableClientState(GL_COLOR_ARRAY);

	::glEnableClientState(GL_VERTEX_ARRAY);


	int half_side = m_halfWidth;
	::glColor4ub(255, 0, 0, 0);	
	
	if(animMat != NULL)
	{
		::glColor4ub(0, 0, 255, 0);	
		half_side = (m_lengthFront + m_lengthRear) >>1; //in animation the car could exceed quad lateral limits
	}

	s_quadVtx[0]= -half_side - 50;	s_quadVtx[1] = 0; s_quadVtx[2] = -m_lengthFront - 50; //fl
	s_quadVtx[3]= half_side + 50;	s_quadVtx[4] = 0; s_quadVtx[5] = -m_lengthFront - 50; //fr
	s_quadVtx[6]= -half_side - 50;		s_quadVtx[7] = 0; s_quadVtx[8]   = m_lengthRear + 50; //rl
	s_quadVtx[9]= half_side + 50;		s_quadVtx[10] = 0; s_quadVtx[11] = m_lengthRear + 50;   //rr

	::glVertexPointer(3, GL_FLOAT, 0, s_quadVtx);	
	
	
	//draw a deep quad ... so shadow will pass allways the test over the road
	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_ALWAYS);
	::glDepthMask(true);
	::glDisable(GL_BLEND);
	::glEnable(GL_POLYGON_OFFSET_FILL);
	::glPolygonOffset(0.0f, +100000.0f);
	::glColorMask( false, false, false, false);
	::glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_quadIndices);		
	::glDisable(GL_POLYGON_OFFSET_FILL);

#ifdef WIN32
	gNbFacesDrawn += 2;
#endif // WIN32


	//consider the flatten animation 
	if(animMat != NULL)
	{
		animMat->GetMatrixFloat(CMatrix44::s_matrixFloat);
		::glMultMatrixf(CMatrix44::s_matrixFloat);
	}

	::glVertexPointer(3, GL_FLOAT, 0, m_pVertex);

	::glEnable(GL_CULL_FACE);
	::glCullFace(GL_BACK);	
	
	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_LEQUAL);
	::glDepthMask(true);

	::glColor4ub(0, 0, 0, alpha);

	::glDisable(GL_BLEND);

	::glColorMask( false, false, false, true );
	
	for (int i=0; i<m_nbMaterials; ++i)
	{
		const int nIndices = m_nbIndices[i];

		if (nIndices <= 0)
			continue;
		
		::glDrawElements( GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, m_pIndices[i]);
	}
	
	::glDepthFunc(GL_EQUAL);
	::glDepthMask(false);
	::glColor4ub(red, green, blue, 0);	
	::glEnable(GL_BLEND);	
	::glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);	
	::glColorMask(true, true, true, true);
	
	for (int i=0; i<m_nbMaterials; ++i)
	{
		const int nIndices = m_nbIndices[i];

		if (nIndices <= 0)
			continue;
		
#ifdef WIN32
		gNbFacesDrawn += nIndices / 3;
#endif

		::glDrawElements( GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, m_pIndices[i]);
	}	

	// bug fix #1833907 - restore the depth
	::glVertexPointer(3, GL_FLOAT, 0, s_quadVtx);	
	::glDisable(GL_CULL_FACE);
	::glEnable(GL_DEPTH_TEST);
	::glDepthFunc(GL_ALWAYS);
	::glDepthMask(true);
	::glDisable(GL_BLEND);
	::glColorMask( false, false, false, false);
	::glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_quadIndices);

	::glColorMask( true, true, true, true);
	//~bug fix

	::glDisableClientState(GL_VERTEX_ARRAY);
	::glDisable(GL_CULL_FACE);
	::glDisable(GL_BLEND);
	
	::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
}
#endif //USE_PROJECT_SHADOW_INTO_TEXTURE

//-----------------------------------------------------------------------------
// ANIMATION
//-----------------------------------------------------------------------------

void CGLMesh::AnimationAlloc( int nbKeyframes )
{
	A_ASSERT(m_vertexKeyframes == 0);

	m_nbKeyframes = nbKeyframes;
	m_vertexKeyframes = NEW f32[m_nbKeyframes * m_nbVertex * 3];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CGLMesh::AnimationLoad( int keyframeIndex, f32* keyframeVert)
{
	A_ASSERT( keyframeIndex < m_nbKeyframes );

	// copy the vertex to the keyframe
	memcpy( m_vertexKeyframes + keyframeIndex * m_nbVertex * 3, keyframeVert, sizeof( f32 ) * m_nbVertex * 3);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CGLMesh::AnimationPlay( int frame1, int frame2, int factor )	// factor between frame1 and frame 2 from 0 - 4096 
{
	A_ASSERT( frame1 < m_nbKeyframes );
	A_ASSERT( frame2 < m_nbKeyframes );
	
	if ( factor < 0 )
		factor = 0;
	if ( factor > 4096 )
		factor = 4096;

	// prevent copy if not needed
	if ((frame1 == m_lastKeyframe1) && (frame1 == m_lastKeyframe1) && (factor == m_lastFactor))
	{
		// nothing to do :)
		return;
	}

	// save parameters
	m_lastKeyframe1 = frame1;
	m_lastKeyframe2 = frame2;
	m_lastFactor = factor;

	int	nbv = m_nbVertex * 3;

	// check no interpolation cases first
	if (factor == 0)
	{
		// frame 1
		f32*	in = m_vertexKeyframes + frame1 * m_nbVertex * 3;
		f32*	out = m_pVertex;

		while (nbv--)
		{
			*out++ = *in++;
		}
	}
	else if (factor == 4096)
	{
		// frame 2
		f32*	in = m_vertexKeyframes + frame2 * m_nbVertex * 3;
		f32*	out = m_pVertex;

		while (nbv--)
		{
			*out++ = *in++;
		}
	}
	else
	{
		// interpolation between frame1 and frame2
		f32*	in1 = m_vertexKeyframes + frame1 * m_nbVertex * 3;
		f32*	in2 = m_vertexKeyframes + frame2 * m_nbVertex * 3;
		f32*	out = m_pVertex;
		int		factor2 = 4096 - factor;

		while (nbv--)
		{
			*out++ = ((*in1) * factor2 + (*in2) * factor) / 4096.0f;
			++in1; ++in2;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CGLMesh::AnimationPlay3( int frame1, int frame2, int frame3, int alpha1, int alpha2, int alpha3 )
{
	A_ASSERT( frame1 < m_nbKeyframes );
	A_ASSERT( frame2 < m_nbKeyframes );
	A_ASSERT( frame3 < m_nbKeyframes );

	f32 const * in1 = m_vertexKeyframes + frame1 * m_nbVertex * 3;
	f32 const * in2 = m_vertexKeyframes + frame2 * m_nbVertex * 3;
	f32 const * in3 = m_vertexKeyframes + frame3 * m_nbVertex * 3;
	f32 * out = m_pVertex;	

	f32 const a1 = (f32)alpha1;
	f32 const a2 = (f32)alpha2;
	f32 const a3 = (f32)alpha3;

	int nbv = m_nbVertex * 3;
	while (nbv--)
	{
		*out = ((a1 * (*in1)) + (a2 * (*in2)) + (a3 * (*in3))) / 4096.0f;
		++ out; ++ in1; ++ in2; ++ in3;
	}
}

#endif // USE_OGL


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

CBasicMesh::CBasicMesh(const char* fileName, const char* textureName, int scaleShift)
	:CMesh("",0,0,0,0,0,0,0,true),
	m_textures(0)
{
	m_nTextures = 1;
	m_textures = new TTexture*[m_nTextures];
	m_textures[0] = NEW Lib3D::TTexture(textureName, MASK_TEXTURE_NEAREST_FREE_BUFFER);

	int i;
	A_IFile * file = A_IFile::Open( fileName, A_IFile::OPEN_READ | A_IFile::OPEN_BINARY | A_IFile::OPEN_CACHE);


	i = file->GetLong();A_ASSERT(i==1);
	file->GetLong();	// unused blockId

	// Vertexes
	m_nbVertex	= file->GetLong();
	m_vertex	= NEW TVertex[m_nbVertex];

	if (scaleShift >= 0)
	{
		for(i=0; i < m_nbVertex; i++)
		{
			Vector4s& v = m_vertex[i].InitialPos;
			//Scales the mesh 
			v.x = (file->GetLong()) >> scaleShift;
			v.y = (file->GetLong()) >> scaleShift;
			v.z = (file->GetLong()) >> scaleShift;
		}
	}
	else
	{
		scaleShift = -scaleShift;
		for(i=0; i < m_nbVertex; i++)
		{
			Vector4s& v = m_vertex[i].InitialPos;
			//Scales the mesh 
			v.x = (file->GetLong()) << scaleShift;
			v.y = (file->GetLong()) << scaleShift;
			v.z = (file->GetLong()) << scaleShift;
		}
	}

	i = file->GetLong();A_ASSERT(i==1);// submeshes

	const int meshId	= file->GetLong();
	const int doubleFace= file->GetLong();		
	m_nbFaces			= file->GetLong();
	m_face = NEW Lib3D::TFace[m_nbFaces];

	for(i=0;i<m_nbFaces;i++)
		CreateFace(file,m_face[i],m_vertex);

	SetUV();

	A_IFile::Close(file);
}

CBasicMesh::CBasicMesh(const char* fileName, const char** textureNames, int nTextures, int scaleShift)
	:CMesh("",0,0,0,0,0,0,0,true),
	m_textures(0)
{
	m_nTextures = nTextures;
	m_textures = new TTexture*[m_nTextures];
	for (int i=0; i<m_nTextures; i++)
		m_textures[i] = NEW Lib3D::TTexture(textureNames[i], MASK_TEXTURE_NEAREST_FREE_BUFFER);

	int i;
	A_IFile * file = A_IFile::Open( fileName, A_IFile::OPEN_READ | A_IFile::OPEN_BINARY | A_IFile::OPEN_CACHE);


	i = file->GetLong();A_ASSERT(i==1);
	file->GetLong();	// unused blockId

	// Vertexes
	m_nbVertex	= file->GetLong();
	m_vertex	= NEW TVertex[m_nbVertex];

	for(i=0;i<m_nbVertex;i++)
	{
		Vector4s& v = m_vertex[i].InitialPos;
		//Scales the mesh 
		v.x = (file->GetLong()) >> scaleShift;
		v.y = (file->GetLong()) >> scaleShift;
		v.z = (file->GetLong()) >> scaleShift;
	}

	i = file->GetLong();A_ASSERT(i==1);// submeshes

	const int meshId	= file->GetLong();
	const int doubleFace= file->GetLong();		
	m_nbFaces			= file->GetLong();
	m_face = NEW Lib3D::TFace[m_nbFaces];

	for(i=0;i<m_nbFaces;i++)
		CreateFace(file,m_face[i],m_vertex);

	SetUV();

	A_IFile::Close(file);
}

CBasicMesh::~CBasicMesh()
{
	for (int i=0; i<m_nTextures; i++)
		MM_DELETE m_textures[i];

	DELETE_ARRAY m_textures;
}

void CBasicMesh::SetUV(int textureId)
{
	//int res = m_textures[textureId]->SizeX();

	//if(res==256)
	//	return;

	//A_ASSERT(res<256);
	//int shift = 0;
	//A_ASSERT(res);
	//while(res != 256)
	//{
	//	res <<=1;
	//	shift++;
	//}

	for(int i = 0;i<m_nbFaces;i++)
	{
		ShiftUV(m_face[i]);
	}
}

void CBasicMesh::Draw(CLib3D& lib3d)	
{
	lib3d.DrawMeshOTFace(*this,0);
}

void CBasicMesh::ShiftUV(Lib3D::TFace& face)
{
	int shiftU = 8 - face.m_texture->XShift();
	int shiftV = 8 - face.m_texture->YShift();

	if (shiftU > 0)
	{
		face.uA >>= shiftU;
		face.uB >>= shiftU;
		face.uC >>= shiftU;
	}
	
	if (shiftV > 0)	
	{
		face.vA >>= shiftV;	
		face.vB >>= shiftV;	
		face.vC >>= shiftV;
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CBasicMesh::CreateFace(A_IFile * file,Lib3D::TFace& face,Lib3D::TVertex* vertex)
{
	int iA = file->GetLong();

	face.VxA = (Vector4s*)(vertex + iA);
	face.uA = file->GetLong();
	face.vA = file->GetLong();


	int iB = file->GetLong();
	face.VxB = (Vector4s*)(vertex + iB);
	face.uB = file->GetLong();
	face.vB = file->GetLong();

	int iC = file->GetLong();
	face.VxC = (Vector4s*)(vertex + iC);
	face.uC  = file->GetLong();
	face.vC  = file->GetLong();

	// the normal is ignored
	file->GetLong();
	file->GetLong();
	file->GetLong();

	// get material ID
	int material = file->GetChar() & 0xFF;
	if (material >= m_nTextures)
		material = 0;

	face.m_texture = m_textures[material];
}

void CBasicMesh::ComputeBB(Vector4s& bb_min, Vector4s& bb_max)
{
	int xmin = 100000;
	int xmax = -100000;
	int ymin = 100000;
	int ymax = -100000;
	int zmin = 100000;
	int zmax = -100000;

	for(int i = 0; i < m_nbVertex; i++)
	{
		Vector4s& v = m_vertex[i].InitialPos;

		if (v.x < xmin) xmin = v.x;
		if (v.x > xmax) xmax = v.x;

		if (v.y < ymin) ymin = v.y;
		if (v.y > ymax) ymax = v.y;

		if (v.z < zmin) zmin = v.z;
		if (v.z > zmax) zmax = v.z;
	}

	bb_min.x = xmin;						
	bb_min.y = ymin;						
	bb_min.z = zmin;					
	bb_max.x = xmax;						
	bb_max.y = ymax;						
	bb_max.z = zmax;	
}