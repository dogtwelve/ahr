#include "shadow.h"

#if USE_CAR_SHADOW == 1

#include "devutil.h"
#include "texture.h"
#include "Lib3d.h"
#include "HighGear.h"

#include "ScreenBufferWrapper.h"

using namespace Lib3D;

#ifdef WIN32
	extern int gNbFacesDrawn;
#endif

// ----------------------------------------------
//	class CShadow
// ----------------------------------------------

f32 CShadow::s_texFBO_UV[4*2];
f32 CShadow::s_texFBO_XYZ[4*3];
//0 bottom left, 1 bottom right, 2 top right, 3 top left
u16 CShadow::s_texFBO_INDICES[6] = { 0, 1, 2, 0, 2, 3 };

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE
//texture frame buffer object
TextureFBO* CShadow::s_texFBO = NULL;

void CShadow::createShadowTexFBO()
{
	s_texFBO = NEW TextureFBO(SHADOW_TEXFBO_WIDTH, SHADOW_TEXFBO_HEIGHT, true);

	::glBindTexture(GL_TEXTURE_2D, s_texFBO->m_glTextureName);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	::glBindTexture(GL_TEXTURE_2D, 0);	
}

void CShadow::destroyShadowTexFBO()
{	
	SAFE_DELETE(s_texFBO);
}

//if animMatrix!=NULL do not apply shearing ...
void CShadow::projectShadowInTexture(int lightAngle, CMatrix44 *animMatrix)
{
	int top = m_carMesh.GetLengthFront() + SHADOW_TEXFBO_DELTA;
	int bottom = -m_carMesh.GetLengthRear() - SHADOW_TEXFBO_DELTA;
	int size = top - bottom;

	s_texFBO->saveFBOLinkage();
	s_texFBO->bindFBO();
	::glViewport(0, 0, SHADOW_TEXFBO_WIDTH, SHADOW_TEXFBO_HEIGHT);

	::glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //transparent and black
	::glClear(GL_COLOR_BUFFER_BIT);
	
	::glMatrixMode(GL_PROJECTION);
	::glPushMatrix();
	::glLoadIdentity();	
	::glOrthof( -(size >> 1), (size >> 1), bottom, top, -1000.0f, 1000.0f);
	::glRotatef(90, 1, 0, 0); //look from top ...
	
	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();	
	::glLoadIdentity();
	
	if(animMatrix != NULL)
	{
		animMatrix->GetMatrixFloat(CMatrix44::s_matrixFloat);
	}
	else
	{		
		//multiply with shearing matrix
		memset(CMatrix44::s_matrixFloat, 0, 16 * sizeof (float));
		//m[0:0]
		CMatrix44::s_matrixFloat[0] = 1.0f;
		//x offset from y
		CMatrix44::s_matrixFloat[4] = (-1.0f * Cosinus(lightAngle) ) / COS_SIN_MUL;
		//m[1:1]
		CMatrix44::s_matrixFloat[5] = 1.0f;
		//z offset from y
		CMatrix44::s_matrixFloat[6] = (-1.0f * Sinus(lightAngle) ) / COS_SIN_MUL;	
		//m[2:2]
		CMatrix44::s_matrixFloat[10] = 1.0f;
		//m[3:3]
		CMatrix44::s_matrixFloat[15] = 1.0f;	
	}

	::glMultMatrixf(CMatrix44::s_matrixFloat);

	//fill alpha channel	
	m_carMesh.DrawAsShadow(0, 0, 0, 0x48);	

	//restore matrix
	::glMatrixMode(GL_PROJECTION);
	::glPopMatrix();
	::glMatrixMode(GL_MODELVIEW);
	::glPopMatrix();
	
	s_texFBO->restoreFBOLinkage();
	
	::glViewport(0, 0, g_sceneViewportW, g_sceneViewportH);
}
#endif //USE_PROJECT_SHADOW_INTO_TEXTURE

void  CShadow::freeShadowMesh()
{
	//for (int i=0; i < m_nFaces; i++)
	//	MM_DELETE m_faces[i];

	//for (int i=0; i < m_nVertex; i++)
	//	MM_DELETE m_vertexes[i];

	//MM_DELETE m_vertexes;
	//MM_DELETE m_faces;

	MM_DELETE(m_shadowMesh);
}

void  CShadow::initializeShadowMesh()
{
//	int		texture_u = (m_pShadowTexture->SizeX()-1) << TTexture::TEX_UV_SHIFT;
//	int		texture_v = (m_pShadowTexture->SizeY()-1) << TTexture::TEX_UV_SHIFT;

//	m_nFaces = m_carMesh.NbFaces();// - WHEEL_FACE_SIZE;
//	m_nVertex = m_carMesh.NbVertex();// - WHEEL_ARRAY_SIZE;
	
	//m_vertexes = NEW  TVertex * [m_nVertex];
	//m_faces = NEW  TFace * [m_nFaces];
	//TVertex * firstVertex = m_carMesh.FirstVertex();
	//TFace * firstFace = m_carMesh.FirstFace();

	//int k = 0;

	//for (int i=0;i<m_carMesh.NbVertex();i++)
	//{
	//	TVertex * newVertex = NEW TVertex();
	//	newVertex->InitialPos = Vector4s(firstVertex[i].InitialPos);
	//	m_vertexes[k++] = newVertex;
	//}

	//k = 0;
	//for (int i=0; i<m_carMesh.NbFaces(); i++)
	//{
	//	TFace currentFace = firstFace[i];
	//	//if (currentFace.GetTexture() == m_car->wheeltextures[0])
	//	//	continue;

	//	int idxVertexA =  currentFace.VertexA() - firstVertex;
	//	int idxVertexB =  currentFace.VertexB() - firstVertex;
	//	int idxVertexC =  currentFace.VertexC() - firstVertex;
	//	
	//	TFace * newFace = NEW TFace(m_pShadowTexture,
	//								*m_vertexes[idxVertexA],
	//								0,
	//								0,
	//								*m_vertexes[idxVertexB],
	//								0,
	//								0,
	//								*m_vertexes[idxVertexC],
	//								0,
	//								0,
	//								0); // TFACE_FLAG_TRANS_ADDITIVE);

	//	newFace->m_flag = TFACE_FLAG_TRANS_SHADOW;

	//	m_faces[k++] = newFace;
	//}

	int* nIndices = new int[1];
	nIndices[0] = 0;
	for (int i=0; i<m_carMesh.NbMaterials(); ++i)
		nIndices[0] += m_carMesh.NbIndices(i);

	int nbVertex = m_carMesh.NbVertex();
	f32 *vertex = new f32[nbVertex];
	f32 *tex = new f32[nbVertex];
	u16 **indices = new u16*[1];
	indices[0] = new u16[nIndices[0]];

	nIndices[0] = 0;
	for (int i=0; i<m_carMesh.NbMaterials(); ++i)
	{
		memcpy(indices[0] + nIndices[0], m_carMesh.Indices(i), m_carMesh.NbIndices(i));
		nIndices[0] += m_carMesh.NbIndices(i);
	}

	m_shadowMesh = NEW CGLMesh(1, nbVertex, nIndices, vertex, tex, NULL, indices, NULL, 0, m_carMesh.GetLengthFront(), m_carMesh.GetLengthRear(), m_carMesh.GetHalfWidth());
}

CShadow::CShadow(const TTexture* texture, CGLMesh & carMesh/*int z,int x_left,int x_right,int y_front,int y_back*/)
:m_pShadowTexture(texture),
m_carMesh(carMesh),
m_car(NULL),
m_nFaces(0),
m_nVertex(0)
{
	initializeShadowMesh();
}

CShadow::~CShadow()
{
	freeShadowMesh();
}



void CShadow::Draw(	Lib3D::CLib3D& lib3d,const Vector4s * carPosition,Vector4s * sunPosition, 
				   bool bRealShadow, CMatrix44 *inverseShadowMatrix, CMatrix44 *animMatrix)
{
#ifdef USE_OGL
	if( bRealShadow )
	{
		int angle = 0;

		if( inverseShadowMatrix != NULL )
		{
			const Vector4s& cameraPosition	= lib3d.GetCameraPosition();

			const Vector4s	sunPositionToBeTransformed( cameraPosition.x + sunPosition->x,sunPosition->y,cameraPosition.z + sunPosition->z);
			Vector4s		sunPositionTransformed;
			//bring sunPos in shadow space
			inverseShadowMatrix->TransformVector( &sunPositionToBeTransformed, &sunPositionTransformed );
				
			angle = Atan2i( sunPositionTransformed.x , sunPositionTransformed.z );
		}

	#ifndef USE_PROJECT_SHADOW_INTO_TEXTURE
		
		CMatrix44 current_matrix = lib3d.m_renderer->CurrentMatrix();
		current_matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

		::glMatrixMode(GL_MODELVIEW);
		::glPushMatrix();
		::glLoadMatrixf(CMatrix44::s_matrixFloat);
		
		if( inverseShadowMatrix != NULL )
		{
			//multiply with shearing matrix
			memset(CMatrix44::s_matrixFloat, 0, 16 * sizeof (float));
			//m[0:0]
			CMatrix44::s_matrixFloat[0] = 1.0f;
			//x offset from y
			CMatrix44::s_matrixFloat[4] = (-1.0f * Cosinus(angle) ) / COS_SIN_MUL;
			//m[1:1]
			CMatrix44::s_matrixFloat[5] = 1.0f;
			//z offset from y
			CMatrix44::s_matrixFloat[6] = (-1.0f * Sinus(angle) ) / COS_SIN_MUL;	
			//m[2:2]
			CMatrix44::s_matrixFloat[10] = 1.0f;
			//m[3:3]
			CMatrix44::s_matrixFloat[15] = 1.0f;

			::glMultMatrixf(CMatrix44::s_matrixFloat);
		}

		m_carMesh.DrawAsShadow(0, 0, 0, 0x48, animMatrix);

		::glPopMatrix();	

	#else //!USE_PROJECT_SHADOW_INTO_TEXTURE

		if(animMatrix != NULL )
		{			
			projectShadowInTexture(0, animMatrix);			
		}
		else
		{				
			//glViewport in texture space start from the upper left corner (0, 0)			
			projectShadowInTexture(angle, NULL);
		}

		//DEBUG_SHADOW_PROJECTION
		s_texFBO_UV[0] = 0;
		s_texFBO_UV[1] = 1;
		s_texFBO_UV[2] = 1;
		s_texFBO_UV[3] = 0;
		g_lib3DGL->paint2DModule(0, 0, 256, 256, s_texFBO->m_glTextureName, s_texFBO_UV, 0);

		//look on negative Z
		int quadFront = -m_carMesh.m_lengthFront - SHADOW_TEXFBO_DELTA;
		int quadBottom = m_carMesh.m_lengthRear + SHADOW_TEXFBO_DELTA;

		int quadHalfSize = (quadBottom - quadFront) >>1;
		
		//GLVIEWPORT IN TEXTURE SPACE START FROM THE UPPER LEFT CORNER (0, 0)
		//draw a textured quad under car
		//create vertexes
		//bottom left
		s_texFBO_XYZ[0] = -quadHalfSize;
		s_texFBO_XYZ[1] = 0;
		s_texFBO_XYZ[2] = quadBottom;

		s_texFBO_UV[0]	= 0.0f;
		s_texFBO_UV[1]	= 0.0f;

		//bottom right
		s_texFBO_XYZ[3] = quadHalfSize;
		s_texFBO_XYZ[4] = 0;
		s_texFBO_XYZ[5] = quadBottom;

		s_texFBO_UV[2]	= 1.0f;
		s_texFBO_UV[3]	= 0.0f;

		//top right
		s_texFBO_XYZ[6] = quadHalfSize;
		s_texFBO_XYZ[7] = 0;
		s_texFBO_XYZ[8] = quadFront;

		s_texFBO_UV[4]	= 1.0f;
		s_texFBO_UV[5]	= 1.0f;

		//top left
		s_texFBO_XYZ[9] = -quadHalfSize;
		s_texFBO_XYZ[10] = 0;
		s_texFBO_XYZ[11] = quadFront;

		s_texFBO_UV[6]	= 0.0f;
		s_texFBO_UV[7]	= 1.0f;

		
		CMatrix44 current_matrix = lib3d.m_renderer->CurrentMatrix();
		current_matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

		::glMatrixMode(GL_MODELVIEW);
		::glPushMatrix();	
		::glLoadMatrixf(CMatrix44::s_matrixFloat);

		::glEnable(GL_TEXTURE_2D);	
		::glBindTexture(GL_TEXTURE_2D, s_texFBO->m_glTextureName);

		::glEnable(GL_CULL_FACE);
		::glCullFace(GL_BACK);

		::glDisable(GL_DEPTH_TEST);

		::glDisableClientState(GL_COLOR_ARRAY);

		::glEnableClientState(GL_VERTEX_ARRAY);
		::glVertexPointer(3, GL_FLOAT, 0, s_texFBO_XYZ);	
	
		::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		::glTexCoordPointer(2, GL_FLOAT, 0, s_texFBO_UV);
		
		::glEnable( GL_BLEND );
		::glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );	

#ifdef WIN32
		gNbFacesDrawn += 2;
#endif // WIN32

		::glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_texFBO_INDICES);
	
		//restore gl state
		::glDisableClientState(GL_VERTEX_ARRAY);
		::glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		::glDisable(GL_BLEND);	
		
		::glDisable(GL_CULL_FACE);
		
		::glPopMatrix();	
	#endif //!USE_PROJECT_SHADOW_INTO_TEXTURE
	}
	else //not real for opponents
	{
		//draw a textured quad under car
		//create vertexes
		//bottom left
		s_texFBO_XYZ[0] = -m_carMesh.m_halfWidth;
		s_texFBO_XYZ[1] = 0;
		s_texFBO_XYZ[2] = m_carMesh.m_lengthRear;

		s_texFBO_UV[0]	= 0.0f;
		s_texFBO_UV[1]	= 1.0f;

		//bottom right
		s_texFBO_XYZ[3] = m_carMesh.m_halfWidth;
		s_texFBO_XYZ[4] = 0;
		s_texFBO_XYZ[5] = m_carMesh.m_lengthRear;

		s_texFBO_UV[2]	= 1.0f;
		s_texFBO_UV[3]	= 1.0f;

		//top right
		s_texFBO_XYZ[6] = m_carMesh.m_halfWidth;
		s_texFBO_XYZ[7] = 0;
		s_texFBO_XYZ[8] = -m_carMesh.m_lengthFront;

		s_texFBO_UV[4]	= 1.0f;
		s_texFBO_UV[5]	= 0.0f;

		//top left
		s_texFBO_XYZ[9] = -m_carMesh.m_halfWidth;
		s_texFBO_XYZ[10] = 0;
		s_texFBO_XYZ[11] = -m_carMesh.m_lengthFront;

		s_texFBO_UV[6]	= 0.0f;
		s_texFBO_UV[7]	= 0.0f;

		
		CMatrix44 current_matrix = lib3d.m_renderer->CurrentMatrix();
		current_matrix.GetMatrixFloat(CMatrix44::s_matrixFloat);

		::glMatrixMode(GL_MODELVIEW);
		::glPushMatrix();	
		::glLoadMatrixf(CMatrix44::s_matrixFloat);

		::glDisable( GL_TEXTURE_2D );	

		::glEnable(GL_CULL_FACE);
		::glCullFace(GL_BACK);

		::glDisable(GL_DEPTH_TEST);
		
		::glDisableClientState(GL_COLOR_ARRAY);

		::glEnableClientState(GL_VERTEX_ARRAY);	

		::glVertexPointer(3, GL_FLOAT, 0, s_texFBO_XYZ);	
	
		::glColor4ub(0, 0, 0, 0x48);	
		::glEnable( GL_BLEND );
		::glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );	

#ifdef WIN32
		gNbFacesDrawn += 2;
#endif // WIN32

		::glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_texFBO_INDICES);
	
		//restore gl state
		::glDisableClientState(GL_VERTEX_ARRAY);

		::glDisable(GL_BLEND);	

		::glDisable(GL_CULL_FACE);

		::glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		::glPopMatrix();
	}

#else //USE_OGL

 	const unsigned int	OrFlag = lib3d.m_renderer->TransformVertexPointerArray(m_vertexes, m_nVertex);

	if (!(OrFlag & (Lib3D::kRejectFar|Lib3D::kRejectNear)))
	{
		const CMatrix44*	matrix		= lib3d.GetCameraMatrix();
		const Vector4s& cameraPosition	= lib3d.GetCameraPosition();

		const Vector4s	sunPositionToBeTransformed( cameraPosition.x + sunPosition->x,sunPosition->y,cameraPosition.z + sunPosition->z);
		Vector4s		sunPositionTransformed;
		matrix->TransformVector( &sunPositionToBeTransformed, &sunPositionTransformed );

		const Vector4s	carPositionToBeTransformed(carPosition->x,carPosition->y,carPosition->z);
		Vector4s		carPositionTransformed;
		matrix->TransformVector( &carPositionToBeTransformed, &carPositionTransformed );

		int ratioX = (sunPositionTransformed.x - carPositionTransformed.x);
		int ratioZ = (sunPositionTransformed.z - carPositionTransformed.z);
		int angle = Atan2i( ratioX, ratioZ );

		for (int i = m_nVertex - 1;i >= 0; --i)
		{
			int heightMultiplier = m_vertexes[i]->InitialPos.y >> 3;
			m_vertexes[i]->WorldTPos.x -= (heightMultiplier * (Cosinus(angle)>>9))>>2;
			m_vertexes[i]->WorldTPos.z -= (heightMultiplier * (Sinus(angle)>>9)) >>2;
		}

		for(int i = m_nFaces-1; i >= 0; --i)
		{
			 //m_faces[i]->m_flag = TFACE_FLAG_TRANS_SHADOW; // TFACE_FLAG_TRANS_SUBSTRATIVE;

			lib3d.m_renderer->OTTableAdd(m_faces[i]);
		}
	}
#endif /* USE_OGL */
}

void CShadow::DrawCopterShadow(Lib3D::CLib3D& lib3d,const Vector4s * carPosition,Vector4s * sunPosition)
{
#pragma REMINDER ("Implement DrawCopterShadow()")
	/*
	const unsigned int	OrFlag = lib3d.m_renderer->TransformVertexPointerArray(m_vertexes, m_nVertex);

	if (!(OrFlag & (Lib3D::kRejectFar|Lib3D::kRejectNear)))
	{
		const CMatrix44*	matrix		= lib3d.GetCameraMatrix();
		const Vector4s& cameraPosition	= lib3d.GetCameraPosition();

		const Vector4s	sunPositionToBeTransformed( cameraPosition.x + sunPosition->x,sunPosition->y,cameraPosition.z + sunPosition->z);
		Vector4s		sunPositionTransformed;
		matrix->TransformVector( &sunPositionToBeTransformed, &sunPositionTransformed );

		const Vector4s	carPositionToBeTransformed(carPosition->x,carPosition->y,carPosition->z);
		Vector4s		carPositionTransformed;
		matrix->TransformVector( &carPositionToBeTransformed, &carPositionTransformed );

		int ratioX = (sunPositionTransformed.x - carPositionTransformed.x);
		int ratioZ = (sunPositionTransformed.z - carPositionTransformed.z);
		int angle = Atan2i( ratioX, ratioZ );

		for (int i = m_nVertex - 1;i >= 0; --i)
		{
			int heightMultiplier = m_vertexes[i]->InitialPos.y >> 3;
			m_vertexes[i]->WorldTPos.x -= (heightMultiplier * (Cosinus(angle)>>9)) >> 1;
			m_vertexes[i]->WorldTPos.z -= (heightMultiplier * (Sinus(angle)>>9)) >> 1;
		}

		for(int i = m_nFaces-1; i >= 0; --i)
		{
			m_faces[i]->m_flag = TFACE_FLAG_TRANS_SHADOW; // TFACE_FLAG_TRANS_SUBSTRATIVE;

			lib3d.m_renderer->OTTableAdd(m_faces[i]);
		}
	}
	*/
}

#endif	// USE_CAR_SHADOW

