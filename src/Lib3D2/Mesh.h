#ifndef _MESH_H_
#define _MESH_H_

#include "GenDef.h"

#include "Matrix.h"
#include "Face.h"
#include "config.h"
#include "DevUtil.h"
#include "Texture.h"

#include "Render.h" // for OGL defines

class A_IFile;

namespace Lib3D
{
	class TVertex;
	class TFace;
	class OrientedBBox;
	class TTexture;

// -------------------------------------------------------------
// 3D mesh structure
// -------------------------------------------------------------
class CMesh
{
	enum eFlags
		{
			kDisplay		= 0x01,
			kIncluded		= 0x02,
			kUseAlpha		= 0x04,
			kPortalActive	= 0x08,
		};
public:
	CMesh(const char* name,int nbVertex,TVertex*,int nbFaces,TFace*, int lengthFront, int lengthRear, int halfWidth,bool doubleFaced);
	virtual ~CMesh();

inline	int						NbFaces() const {return m_nbFaces;}

inline	TFace*					FirstFace() {A_ASSERT(m_face);return m_face;}
inline	TFace*					LastFace() {A_ASSERT(m_face);return m_face + m_nbFaces;}
inline	const TFace&			GetFace(int i) const {A_ASSERT(i>=0 && i<NbFaces() && m_face!=NULL);return m_face[i];}

inline	const TFace*			FirstFace() const  {A_ASSERT(m_face);return m_face;}
inline	const TFace*			LastFace() const {A_ASSERT(m_face);return m_face + m_nbFaces;}

inline	int						NbVertex() const{return m_nbVertex;}

inline	TVertex*				FirstVertex() {A_ASSERT(m_vertex);return m_vertex;}
inline	TVertex*				LastVertex() {A_ASSERT(m_vertex);return m_vertex + m_nbVertex;}

inline	const TVertex*			FirstVertex() const  {A_ASSERT(m_vertex);return m_vertex;}
inline	const TVertex*			LastVertex() const {A_ASSERT(m_vertex);return m_vertex + m_nbVertex;}
	
	void					GetLimits(Vector4s *Min, Vector4s *Max) const;

	void					ClearFaces();

	void					AnimationAlloc( int nbKeyframes );
	void					AnimationLoad( int keyframeIndex, Vector4s* keyframeVert );
	void					AnimationPlay( int frame1, int frame2, int factor );	// factor between frame1 and frame 2 from 0 - 4096 
	void					AnimationPlay3( int frame1, int frame2, int frame3, int alpha1, int alpha2, int alpha3 );

	int						GetLengthFront() const {return m_lengthFront;}
	int						GetLengthRear() const{return m_lengthRear;}
	int						GetHalfWidth() const{return m_halfWidth;}

	bool					IsDoubleFaced() const {return m_doubleFaced;}

	int						GetNumberKeyFrames() const { return m_nbKeyframes; }

#ifdef USE_OGL
	//#define MESH_FACE_MAX_TRI		1000

	//static f32 m_pVtx[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_VTX_COMPONENT_COUNT ];
	//static f32 m_pTex[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_TEX_COMPONENT_COUNT ];
	////static u8  m_pColor[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI * FACE_COLOR_COMPONENT_COUNT ];
	//static u16 m_pIndices[ MESH_FACE_MAX_TRI * FACE_VTX_COUNT_PER_TRI ];

	//void					DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
	//void					Draw();

#endif /* USE_OGL */


protected:
	TVertex*				m_vertex;
	int						m_nbVertex;

	TFace*					m_face;
	int						m_nbFaces;
private:
	// morph animation data
	Vector4s*				m_vertexKeyframes;
	int						m_nbKeyframes;
	int						m_lastKeyframe1;
	int						m_lastKeyframe2;
	int						m_lastFactor;

public:
	int						m_lengthFront;
	int						m_lengthRear;
	int						m_halfWidth;
	const bool				m_doubleFaced;

	//char			m_name[24];
	
};


#ifdef USE_OGL

class CGLMesh
{
public:
	// Drawing flags for CGLMesh
	enum
	{
		GLMESH_FLAG_GLOBAL_COLOR = (1 << 0),
		GLMESH_FLAG_VERTEX_COLOR = (1 << 1),
		GLMESH_FLAG_BLENDING = (1 << 2),
		GLMESH_FLAG_NORMALS = (1 << 3),
		GLMESH_FLAG_CULLING = (1 << 4),
		GLMESH_FLAG_DEPTH_FUNC_ALWAYS = (1 << 5),
	} eGLMeshFlags;

	CGLMesh(int nbMaterials, int nbVertex, int *nbIndices, f32 *vertex, f32 *tex, f32* normals, u16 **indices, const TTexture **textures, int textures_nb, int lengthFront, int lengthRear, int halfWidth);
	virtual ~CGLMesh();

	inline int				GetLengthFront() const {return m_lengthFront;}
	inline int				GetLengthRear() const {return m_lengthRear;}
	inline int				GetHalfWidth() const {return m_halfWidth;}

	inline int				NbVertex() const {return m_nbVertex;}
	inline int				NbMaterials() const {return m_nbMaterials;}
	inline int				NbIndices(int i) const { return m_nbIndices[i]; }
	inline u16*				Indices(int i) const { A_ASSERT(i < m_nbMaterials); return m_pIndices[i]; }
	inline f32*				GetVertices() const { return m_pVertex; }

	inline void				SetSkipRenderIndex(int index, bool skipRender) { A_ASSERT(index < m_nbMaterials); m_skipRenderIndices[index] = (char) skipRender; }

	Vector4s				GetVector(int index);
//	Vector4s&				GetVector(int index) const;

	void					Draw(const CMatrix44 &matrix);
	void					DrawEnvMap(const CMatrix44 &matrix);

#ifdef USE_PROJECT_SHADOW_INTO_TEXTURE
	void					DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
#else //USE_PROJECT_SHADOW_INTO_TEXTURE
	void					DrawAsShadow(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, const CMatrix44* animMat);
#endif //USE_PROJECT_SHADOW_INTO_TEXTURE

	inline void				SetTexture(int id, TTexture *tex) { m_pTextures[id] = tex; }
	
	//reflectIntensity [0.0f, 1.0f]
	inline void 			SetEnvTexture(TTexture *tex, f32 envReflectIntensity = 1.0f, f32 envRotAngle = 0.0f){ m_pEnvTexture = tex; m_envReflectIntensity = envReflectIntensity; m_envRotAngle = envRotAngle; }
	inline TTexture*		GetEnvTexture(){ return m_pEnvTexture; }
	inline void				PrepareForEnvMapping() { m_pEnvTexCoord = NEW f32[m_nbVertex * 2]; }
	void					ComputeSphereTexCoord(int materialID, const CMatrix44 &matrix);

	void					AnimationAlloc( int nbKeyframes );
	void					AnimationLoad( int keyframeIndex, f32* keyframeVert );
	void					AnimationPlay( int frame1, int frame2, int factor );	// factor between frame1 and frame 2 from 0 - 4096 
	void					AnimationPlay3( int frame1, int frame2, int frame3, int alpha1, int alpha2, int alpha3 );

//	void					EnableVertexColor(bool bVertexColor) { m_bVertexColorEnabled = bVertexColor; }
//	bool					IsVertexColorEnabled() { return m_bVertexColorEnabled; }
//	void					SetVertexColors(int color);

	void					SetFlag(int flag) { m_flags = flag; }
	void					AddFlag(int flag) { m_flags |= flag; }	
	void					RemoveFlag(int flag) { m_flags &= ~flag; }
	int						GetFlag() { return m_flags; }

	void					SetGlobalColor(int color);

protected:
	int						m_nbVertex;
	int*					m_nbIndices;	// for each material
	int						m_nbMaterials;
	int						m_nbTextures;	// should be equal to m_nbMaterials; used for drawing only
	f32*					m_pVertex;
	f32*					m_pTex;
	f32*					m_pNormals;
	u8*						m_pColors;
	u16**					m_pIndices;		// for each material

	// Added for cars in garage: we don't want the pilot to be displayed, skip rendering material # 1
	char*					m_skipRenderIndices;

	TTexture**				m_pTextures;
	
	TTexture*				m_pEnvTexture;
	f32*					m_pEnvTexCoord;	
	f32						m_envReflectIntensity;
	f32						m_envRotAngle;

	
	int						m_globalColor;
	int						m_flags;

// Animation
	// morph animation data
	f32*					m_vertexKeyframes;
	int						m_nbKeyframes;
	int						m_lastKeyframe1;
	int						m_lastKeyframe2;
	int						m_lastFactor;

public:
	int						m_lengthFront;
	int						m_lengthRear;
	int						m_halfWidth;
};

#endif // USE_OGL


class CBasicMesh
	:public Lib3D::CMesh
{
public:
	CBasicMesh(const char* fileName,const char* textureName,int scaleShift=1);
	CBasicMesh(const char* fileName, const char** textureNames, int nTextures, int scaleShift=1);
	virtual ~CBasicMesh();

	void SetUV(int textureId = 0);
	void Draw(CLib3D& lib3d);

	void ComputeBB(Vector4s& bb_min, Vector4s& bb_max);

private:
	// ---------------------------------------------------------------------------
	//
	// ---------------------------------------------------------------------------
	void ShiftUV(Lib3D::TFace& face);
	// ---------------------------------------------------------------------------
	//
	// ---------------------------------------------------------------------------
	void CreateFace(A_IFile * file,Lib3D::TFace& face,Lib3D::TVertex* vertex);

private:
	Lib3D::TTexture**	m_textures;
	int m_nTextures;
};

}//namespace
#endif // _MESH_H_
