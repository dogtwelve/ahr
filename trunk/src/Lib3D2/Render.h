#ifndef _RENDER_H_
#define _RENDER_H_


#include "Matrix.h"
//#include "Constants.h"

#include "RoadStruct.h"

#ifdef USE_OGL
#include "GenDef.h"
#include "Lib3DGL/Lib3DGL.h"
#endif 

#include "face.h"

class CHighGear;

//specific constants for face's VTX STORAGE FORMAT
#ifdef USE_OGL

	#define FACE_MAX_TRI				10000 //!!!ATTENTION!!! (FACE_MAX_TRI * 3 < 2^16 - 1)

	#define FACE_COMMON_FENCE_TRI_START_IDX 2000
	#define FACE_COMMON_FENCE_TRI_COUNT		2000

	#define FACE_COMMON_TRI_COUNT			4000	

	#define FACE_BILBOARD_TRI_START_IDX		4000
	#define FACE_BILBOARD_TRI_COUNT			0	//FACE_BILBOARD_TRI_START_IDX + FACE_BILBOARD_TRI_COUNT = FACE_BGOBJ_TRI_START_IDX
	
	#define FACE_BGOBJ_TRI_START_IDX		4000
	#define FACE_BGOBJ_TRI_COUNT			6000 //FACE_BGOBJ_START_IDX + FACE_BGOBJ_TRI_COUNT = FACE_MAX_TRI

	#define FACE_TRI_COUNT				2
	#define FACE_VTX_COUNT_PER_TRI		3   //Va, Vb, Vc

	#define FACE_VTX_COMPONENT_COUNT	3	//x, y, z
	#define FACE_VTX_COMPONENT_SIZE		4   //GL_FLOAT

	#define FACE_TEX_COMPONENT_COUNT		2	//u, v
	#define FACE_TEX_COMPONENT_SIZE			4	//GL_FLOAT

	#define FACE_COLOR_COMPONENT_COUNT		4	// R, G, B, A
	#define FACE_COLOR_COMPONENT_SIZE		1	// GL_UNSIGNED_BYTE

#endif /* USE_OGL */

namespace Lib3D
{

enum // Frustrum rejection
{
	kRejectFar	= 0x20,
	kRejectNear = 0x10,
	kRejectB		= 0x08,
	kRejectT		= 0x04,
	kRejectL		= 0x02,
	kRejectR		= 0x01,
};

#define OT_TABLE_BITS	12
#define OT_TABLE_SIZE	(1 << OT_TABLE_BITS)

class TVertex;
class TFace;
class CRender;
class CBoard3D;
class CLib3D;
class CFSort;
class CMesh;
class Portal;
class TBillboard;
class COTObject;
class NormalMask;

// --------------------------------------------------------------------------
// ------------------         RENDERER METHODS          ---------------------
// --------------------------------------------------------------------------
class CRender
{
public:
	CRender(int fov);
	~CRender();

	long m_andFlag; // test


	inline unsigned long				TransformVertex(TVertex *V)const {return TransformVertex(V,V+1);}; // transform vertex using current view matrix, set vertex VMask flag depending of its position relatively to view frustrum  
	unsigned long						TransformVertexPointerArray(TVertex **vertexPointerArray,int count) const;
	unsigned long						TransformVertex(TVertex *V, TVertex*)const;					// transform vertex using current view matrix, set vertex VMask flag depending of its position relatively to view frustrum  
	unsigned long						TransformVertex_Test(TVertex *V, TVertex*);					// transform vertex using current view matrix, set vertex VMask flag depending of its position relatively to view frustrum  
	inline unsigned long				TransformVertexFakeZ(TVertex *V)const {return TransformVertexFakeZ(V,V+1);}; // transform vertex using current view matrix, set vertex VMask flag depending of its position relatively to view frustrum  
	unsigned long						TransformVertexFakeZ(TVertex *V,TVertex* lastVertex)const;
	void								TransformVertexNoClip(TVertex *V,TVertex* lastVertex) const;

  
	void								FrustrumDraw(TFace *F,CBoard3D&,unsigned int tface_flag ); //bool zcorrected=false,bool flat=false,bool trans=false);                   // draw a face in frustrum if subdiv not required. All Vertex must have been transformed before with TransformVertex() function
	void								FrustrumDrawCull(TFace *F,CBoard3D& board,unsigned int tface_flag );
//	void								FrustrumDrawCullSort(TFace *F,CBoard3D& board);

	void								RenderLine(const Vector4s&,const Vector4s& v2,CBoard3D&);                   

#define MESH_OT_TYPE_BG						0
#define MESH_OT_TYPE_TRAFIC					1
#define MESH_OT_TYPE_PLAYER					2
#define MESH_OT_TYPE_COCKPIT				3
#define MESH_OT_TYPE_GHOST					4
#define MESH_OT_TYPE_LOD					5
#define MESH_OT_TYPE_PLAYER_GARAGE			6
#define	MESH_OT_TYPE___COUNT				7 

	void								RenderMeshOTFace(CMesh *Mesh,CBoard3D&, int type);   // add a mesh in OT
	void								RenderMeshOTFace(TVertex* vertex,int nbVertex,TFace* faces,int i_nbFaces,CBoard3D& board, int type,bool doubleFaced );
//	void								RenderMesh(CMesh *Mesh,CBoard3D&,bool envmap);   // draw a mesh

	void								RenderRoad(RoadStruct *road, int from, int to, const Vector4s *CamPosition, CBoard3D& , CLib3D& lib3d, int reverse);   // draw the road. CamPosition allow to perform culling test if fixed objects
#ifdef USE_OGL
	void								RenderRoad(RoadStruct *road, int from, int to, int reverse, TTexture *texture, TTexture::sAtlasTextureTile *atlasTextureTiles, bool bRenderBillboards = true );
#endif /* USE_OGL */

inline	const CMatrix44&					CurrentMatrix() const {return m_matrixStack.CurrentMatrix();}

	CMatrixStack&						MatrixStack() {return m_matrixStack;}
	const CMatrixStack&					MatrixStack() const {return m_matrixStack;}

	void								SetFoV(int fov);
	inline int							GetFoV() const{return m_fov;}
	

	unsigned int						FrustrumRejectionTest(const Vector4s&) const;
	bool								TestSphereInFrustum(const Vector4s&,int radius) const;

	void								OTTableClear();
#ifdef USE_OGL
	void								OTTableAdd( COTObject* otobj, bool bIgnoreZRenderFirst=false );
#else /* USE_OGL */
	void								OTTableAdd( COTObject* );
#endif /* USE_OGL */

#ifdef USE_OGL
	//use bRenderOnlyFarthest to render only the opaque faces ... those with zIdx = OT_TABLE_SIZE - ( 1 << kOOtableShift ) - 1;
	//for more info view  OTTableAdd( COTObject* otobj, bool bIgnoreZRenderFirst=false );
	void								OTTableDraw(CLib3D& lib3d, bool bRenderOnlyFarthest = false );	
#else /* USE_OGL */
	void								OTTableDraw(CLib3D& lib3d );
#endif /* USE_OGL */
	void								OTTableDrawReverse(CLib3D& lib3d );


	void								FrustrumSubdiv(const TFace *Face, CBoard3D&,unsigned int tface_flag, bool cull = false); // draw a face in frustrum if subdiv required. Orflag is subdiv flag. All Vertex must have been transformed before with TransformVertex() function
	void								FaceSubdivide(TFace *Face,const CBoard3D&);  // subdivide a face with a frustrum plane (except far clip)
// rax - not used
//	void								FrustrumSubdiv4(TFace *Face,CBoard3D& board,unsigned int tface_flag );
//	void								FrustrumSubdiv4Front(TFace *Face,CBoard3D& board,unsigned int tface_flag );
//~rax - not used
private:	
	int					m_fov;

	
	
	Vector3i			m_frustrumBTLR[4];		// normal of view frustrun planes (bottom, top, left, right), used to subdiv near faces before projection

	CMatrixStack		m_matrixStack;
	
	TVertex*			m_subVertexList;		// vertex for subdivided opaque face list
	TFace**				m_subFaceList;      // subdivided opaque face list
	int					m_subFaceListSize;  // subdivided face list size

	TFace*				face_pool;			// face pool for generated faces & billboards
	TVertex*			vert_pool;			// vertex pool for generated face (fence)
	TBillboard*			billboard_pool;		


	enum	{kOOtableShift=6,};

	COTObject*			OTTable[ OT_TABLE_SIZE ];
	char					OTTableUsage[ OT_TABLE_SIZE >> kOOtableShift ];
	

#if defined WIN32
public:
	int					otface_nb;
	int					otface_reject_nb;
	int					otface_culled_nb;
	int					otface_culled_2_nb;
	int					otface_culled_bg_nb;
private:
#endif

public:
	CHighGear *m_pHG;
	NormalMask *normalMask;

	//static storage arrays for vtx
#ifdef USE_OGL
public:	
	static f32 s_pVtx[];
	static f32 s_pTex[];
	static u8  s_pColor[];
	static u16 s_pIndices[];
	static u16 s_pIndicesTris[];

	static OTGroupInfo	s_pOTGroupInfo[];

	static void InitVtxStorageArrays();	
	static int s_nOpaqueFenceQuads;
	
	void RenderOpaqueFenceQuads();	
#endif /* USE_OGL*/

};


}//namespace

#endif // _RENDER_H_
