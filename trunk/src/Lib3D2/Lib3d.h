#ifndef _LIB3D_H_
#define _LIB3D_H_

#pragma warning(disable:4786)

#include "config.h"
#include "devutil.h"

#include "render.h"
#include "constants.h"
#include "roadstruct.h"

#include  "Board3D.h"

#include "../Lib2D/Image.h"

#if WIN_DEBUG
	#include <string>	
#endif

class CHighGear;
class SunFX;

namespace Lib3D
{
	class CSkyBox;
	class CColor;
	class CBoard3D;
	class CRender;
	class CMatrix44;
	class TFace;
	class CBBox;
	class OrientedBBox;
	class CCameraAsphalt;
	class Room;
	class Scene;
	class FX;
	class TTexture;

	class NormalMask;

//	enum {k_nFovDefault = 250};
//	enum {k_nFovDefault = 140};//135};//125};
	enum {k_nFovDefault = 260};//135};//125};

	typedef unsigned int ColorType;

// 3D context, contain required object for 3d application
// this is required for Symbian because global object are
// not allowed
class CLib3D
{
	class Line;
public:

	enum PostProcessor
	{
		kNone,
		kDefaultFog,
		kNightVision,
		kThunderStorm,
	};

	CLib3D();
	~CLib3D();

	void							Init();
    

inline	const unsigned short*			GetImageBuffer() const {return m_board3d->GetImageBuffer();}
inline	const unsigned short*			GetImageBufferFull() const {return m_board3d->GetImageBufferFull();}


inline	unsigned short*					GetImageBuffer() {return m_board3d->GetImageBuffer();}
inline	unsigned short*					GetImageBufferFull() {return m_board3d->GetImageBufferFull();}
public:
inline	CMatrix44&						PushMatrix()			{return m_renderer->MatrixStack().PushMatrix();}
inline	void							PopMatrix()				{m_renderer->MatrixStack().PopMatrix();}

inline	const CMatrix44&				CurrentMatrix()const	{return m_renderer->MatrixStack().CurrentMatrix();}

const CMatrix44*				GetCameraMatrix();

inline	void							TransformVertex(TVertex& v) const {m_renderer->TransformVertex(&v);}
	void							DefProjection(const Vector4s *src, Vector4s *target) const;

	void							Draw(RoadStruct* road, int from, int to, int reverse);
#ifdef USE_OGL
	void							Draw(RoadStruct* road, int from, int to, int reverse, TTexture *texture, TTexture::sAtlasTextureTile *atlasTextureTiles, bool bRenderBillboards = true);
#endif /* USE_OGL */

	void							Draw(CSkyBox&, SunFX& sunFx, bool draw_bottom = false);
	void							Draw(const Vector4s&v1, const Vector4s& v2,int colour);
	
inline	void							DrawMeshOTFace(CMesh& mesh,int face_flags ){m_renderer->RenderMeshOTFace(&mesh,*m_board3d, face_flags);}
inline	void							DrawMeshOTFace(TVertex* vertex,int nbVertex,TFace* faces,int i_nbFaces, int face_flags,bool doubleFaced){m_renderer->RenderMeshOTFace(vertex,nbVertex,faces,i_nbFaces,*m_board3d, face_flags,doubleFaced );};

	//void							AddOTMesh( TFace* );

	void							DrawBillboard(const Vector4s& pos,bool screenSpace,const TTexture& ,int scale,const int transparency=0);
	void							DrawLine(const Vector4s& v1,const Vector4s& v2,int c1, int c2, bool yesZ=true);
	void							Draw(TFace* f,unsigned int tface_flag)		{m_board3d->DrawFace(f,tface_flag);}
	bool							TestScreenZoneVisibility(const Vector4s& position,const Vector4s& size) const;

	//void							Blit(unsigned short *in_ScreenPtr,int postProcessor);

	void							SetFov(int in_nFov);
	inline int						GetFoV() const {return m_renderer->GetFoV();}
	
	bool							TestSphereInFrustum(const Vector4s& pos,int radius) const	{return m_renderer->TestSphereInFrustum(pos,radius);}

	inline int						Width()		{return m_dispX;}
	inline int						Height()	{return m_dispY;}
//	inline int						Heigth()	{return Height();}
	
	void							GetVisionAngle(int& out_AngleX, int& out_AngleY) const;
	const Vector4s&					GetCameraPosition() const;
	const Vector4s&					GetCameraRotation() const;

	void							BeginRendering(const Vector4s& camposition,const Vector4s& camRotation, const CMatrix44& fxmat);
	void							EndRendering();
	
	

inline	void							SetFX( const TTexture*	texture, int p1, int p2 )		{ m_board3d->SetTextureFX( texture, p1, p2 ); }

inline	int								GetFrameCount() const {return m_frameCount;}


	TFace*							AllocateFaces(int nbr);
	TVertex*						AllocateVertex(int nbr);

inline	int								NbrFacesAllocated() const	{return m_faceBufferAllocated;}


	// MOVED from private, because I'm so sick of not being able to call renderer
	// and board functions from where I want !!! 
	CBoard3D*           m_board3d;
	CRender*            m_renderer;

    #ifdef BILINEAR_FILTERING_TEST
	static bool bEnableBilinearFiltering;
	#endif
	void enableRendering() {m_RenderingEnabled=true;};
	void disableRendering() {m_RenderingEnabled=false;};
	bool isRenderingEnabled() {return m_RenderingEnabled;};
private:

	enum{k_nFovMin = 100, k_nFovMax = 250};

	CColor*             m_colIndex;
	CCameraAsphalt*			m_camera;
	bool				m_FrameEven;
	bool				m_RenderingEnabled;
	short               m_FovToAngle[(k_nFovMax-k_nFovMin)*2];
	Vector4s			m_frustumPoints[5];	// in worldSpace
	int					m_skybox_previous_rot;
	int					m_skybox_rot;
	int					m_frameCount;

	TFace*				m_faceBuffer;
	int					m_faceBufferAllocated;

	TVertex*			m_vertexBuffer;
	int					m_vertexBufferAllocated;

public:
	CHighGear	*m_pHG;
	Image		*m_screenImage3D;
	int& m_dispX;
	int& m_dispY;

	unsigned char *m_ShadowPtr;
	int shadowConst;
};

}	// namespace

#endif // _LIB3D_H_
