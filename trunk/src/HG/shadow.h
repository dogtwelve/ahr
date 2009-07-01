#ifndef __SHADOW_H
#define __SHADOW_H

#include "vertex.h"
#include "face.h"
#include "mesh.h"
#include "TextureFBO.h"

#include "config.h"
// ----------------------------------------------
//	class CShadow
//
//	This class handle the 4 neons needed to illuminate a car
//	
// ----------------------------------------------

class CCar;

#define SHADOW_TEXFBO_WIDTH	256
#define SHADOW_TEXFBO_HEIGHT 256

#define SHADOW_TEXFBO_DELTA 70		//should be equal to the max height of all cars

#ifdef IPHONE
	#define POLYGON_OFFSET_FOR_SHADOW	(-100.0f)
#else
	#define POLYGON_OFFSET_FOR_SHADOW	(-100000.0f)
#endif


namespace Lib3D
{
	class TTexture;
	class CLib3D;	
//	class TVertex;


#if USE_CAR_SHADOW == 1
class CShadow
{
public:

	CShadow(const TTexture*, CGLMesh & carMesh);
	~CShadow();
	//separate shadowMatrix, from animMatrix 
	//inverseShadowMatrix needed to obtain the light direction in shadow space
	void Draw(	Lib3D::CLib3D& lib3d, const Vector4s * carPosition,Vector4s * sunPosition, 
				bool bRealShadow, CMatrix44 *inverseShadowMatrix, CMatrix44 *animMatrix);
	void DrawCopterShadow(	Lib3D::CLib3D& lib3d, const Vector4s * carPosition,Vector4s * sunPosition);


public:
	//texture frame buffer object
	static f32 s_texFBO_UV[];
	static f32 s_texFBO_XYZ[];
	static u16 s_texFBO_INDICES[];

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE
	static TextureFBO* s_texFBO;
	static void createShadowTexFBO();
	static void destroyShadowTexFBO();
	void projectShadowInTexture(int lightAngle, CMatrix44* animMatrix);
#endif //USE_PROJECT_SHADOW_INTO_TEXTURE

private:

//	TVertex	** m_vertexes;
//	TFace	** m_faces;

//	CMesh & m_carMesh;

// TODO: remove m_carMesh, keep only m_shadowMesh
	CGLMesh&	m_carMesh;
	CGLMesh*	m_shadowMesh;


	CCar *m_car;
	void  initializeShadowMesh();
	void  freeShadowMesh();
// TODO: remove
	const TTexture* const m_pShadowTexture;

	int m_nFaces;
	int m_nVertex;
};
#endif	// USE_CAR_SHADOW
}

#endif //__SHADOW_H
