#ifndef _FACE_H_
#define _FACE_H_

#include "vector.h"
#include "Vertex.h"
#include "matrix.h"
#include "config.h"

#include "GenDef.h"

namespace Lib3D
{

class TTexture;
class CLib3D;
class NormalMask;

typedef int		UVType;


#ifdef USE_OGL
//exclusive
	#define FLAG_GRP_INFO_HAS_BIT_MASK				( 0x3 )
	#define FLAG_GRP_INFO_HAS_TEXTURE					1 
	#define FLAG_GRP_INFO_HAS_COLOR					( 1<<1 )
	
	#define FLAG_GRP_INFO_IS_BILLBOARD				( 1<<2 )

	#define FLAG_GRP_INFO_USE_TRANS					( 1<<5 )
	#define FLAG_GRP_INFO_USE_ADDITIVE				( 1<<6 )
	#define FLAG_GRP_INFO_USE_GLB_ALPHA				( 1<<7 )
	#define FLAG_GRP_INFO_USE_POLYGON_OFFSET		( 1<<8 )
	#define FLAG_GRP_INFO_DISABLE_DEPTH_TEST		( 1<<9 )

	#define FLAG_GRP_INFO_ALL_BIT_MASK					( 0xFFFF )


	#define CHECK_GRP_INFO_HAS_TEXTURE(flag)				( ( flag & FLAG_GRP_INFO_ALL_BIT_MASK ) & FLAG_GRP_INFO_HAS_TEXTURE )
	#define CHECK_GRP_INFO_HAS_COLOR(flag)					( ( flag & FLAG_GRP_INFO_ALL_BIT_MASK ) & FLAG_GRP_INFO_HAS_COLOR )
	#define CHECK_GRP_INFO_IS_BILLBOARD(flag)				( ( flag & FLAG_GRP_INFO_ALL_BIT_MASK ) & FLAG_GRP_INFO_IS_BILLBOARD )
	#define CHECK_GRP_INFO_HAS_POLYGON_OFFSET(flag)			( ( flag & FLAG_GRP_INFO_ALL_BIT_MASK ) & FLAG_GRP_INFO_USE_POLYGON_OFFSET )
	#define CHECK_GRP_INFO_DISABLE_DEPTH_TEST(flag)			( ( flag & FLAG_GRP_INFO_ALL_BIT_MASK ) & FLAG_GRP_INFO_DISABLE_DEPTH_TEST )


		
//describe couple of objects with same appearance
	typedef struct tagOTGroupInfo
	{
		u16 m_flags;		
		const TTexture* m_pTexOrColor;


		u32 m_startVtxIdx; //index to the start vertex (offset in vertex arrays)
		u32 m_vtxCount;    //number of vertex for this group
		
		tagOTGroupInfo():  
						   m_flags(0),
						   m_pTexOrColor(0),
						   m_startVtxIdx(0), m_vtxCount(0)
		{}

	} OTGroupInfo;
#endif /* USE_OGL*/

// ----------------------------------------------
//
// ----------------------------------------------
class TVxuv
{
public:
	TVxuv()		{};
	Vector4s	*Vx;

	UVType		u;
	UVType		v;

private:
	TVxuv(const TVxuv&);
	void operator=(const TVxuv&);
};




class COTObject
{
protected:
	COTObject():m_flag(0x00){}

	#if WIN_DEBUG
	virtual ~COTObject(){m_flag = 0xFFFFFFFF;}
	#endif

public:

	virtual int			GetZForOrderingTable() const =0;
	virtual void		Draw(CLib3D&)=0;

#ifdef USE_OGL
	//add a face or billboard to vtx, collor, and texture arrays ( )
	//!!! OPTIMIZATION ( avoid recompute the postion from where to add the vtx )
	//
	//this function will modify pVtx, pTex, pColor ...  
	//so these arrays will point to a vtx position after the last added vtx( references to pointers)
	//
	virtual void		AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor)=0;

	virtual unsigned int	GetGLTextureName(){ return 0;}
#endif /* USE_OGL */

	unsigned int		m_flag;	
	COTObject*			m_next_object;
};


// ----------------------------------------------
//
// ----------------------------------------------
class TFace
	:public COTObject
{
public:
	//enum
	//	{
	//		kNormalShift = 12
	//	};

	TFace():VxA(0),VxB(0),VxC(0),m_texture(0)		{};

	TFace(TVertex*);

	TFace(	const TTexture* ,
							TVertex& vA,UVType u1,UVType v1,
							TVertex& vB,UVType u2,UVType v2,
							TVertex& vC,UVType u3,UVType v3,char shading);
	TFace(const TTexture*,TVertex&,TVertex& vB,TVertex& vC);

inline	const Vector4s&		ScrVectorA() const {return  *VxA;}
inline	const Vector4s&		ScrVectorB() const {return  *VxB;}
inline	const Vector4s&		ScrVectorC() const {return  *VxC;}
	
inline	Vector4s&			ScrVectorA() {return  *VxA;}
inline	Vector4s&			ScrVectorB() {return  *VxB;}
inline	Vector4s&			ScrVectorC() {return  *VxC;}

inline	TVertex*			VertexA() {return reinterpret_cast<TVertex*>(VxA);}
inline	TVertex*			VertexB() {return reinterpret_cast<TVertex*>(VxB);}
inline	TVertex*			VertexC() {return reinterpret_cast<TVertex*>(VxC);}

inline	const TVertex*		VertexA()const  {return reinterpret_cast<TVertex*>(VxA);}
inline	const TVertex*		VertexB()const  {return reinterpret_cast<TVertex*>(VxB);}
inline	const TVertex*		VertexC()const  {return reinterpret_cast<TVertex*>(VxC);}


	inline const Vector4s&	VectorA(eVextexVxIndex index) const {return  VxA[index];}
	inline const Vector4s&	VectorB(eVextexVxIndex index) const {return  VxB[index];}
	inline const Vector4s&	VectorC(eVextexVxIndex index) const {return  VxC[index];}

	inline void			SetVectorA(TVertex* v) { VxA = reinterpret_cast<Vector4s*>(v); }
	inline void			SetVectorB(TVertex* v) { VxB = reinterpret_cast<Vector4s*>(v); }
	inline void			SetVectorC(TVertex* v) { VxC = reinterpret_cast<Vector4s*>(v); }

//	inline void			SetVectorA(Vector4s* v) { VxA = v; }
//	inline void			SetVectorB(Vector4s* v) { VxB = v; }
//	inline void			SetVectorC(Vector4s* v) { VxC = v; }


inline	const TTexture*	GetTexture() const					{return m_texture;}
inline	void			SetTexture(const TTexture* t)		{m_texture = t;}

inline	void			InitSubDivFace(const TFace& face)	{	m_texture = face.m_texture; }


	bool			CullingTest() const;
	bool			CullingTestInv() const;
	


inline	const	TVxuv*		GetVxuvA() const{return reinterpret_cast<const	TVxuv*>(&VxA);}	
inline	const	TVxuv*		GetVxuvB() const{return (const	TVxuv*)&VxB;}
inline	const	TVxuv*		GetVxuvC() const{return (const	TVxuv*)&VxC;}

inline	UVType				GetuA() const {return uA;}
inline	UVType				GetvA() const {return vA;}

inline	UVType				GetuB() const {return uB;}
inline	UVType				GetvB() const {return vB;}

inline	UVType				GetuC() const {return uC;}
inline	UVType				GetvC() const {return vC;}

inline	void				SetuA(UVType u) {uA = u;}
inline	void				SetvA(UVType v) {vA = v;}
inline	void				SetuB(UVType u) {uB = u;}
inline	void				SetvB(UVType v) {vB = v;}
inline	void				SetuC(UVType u) {uC = u;}
inline	void				SetvC(UVType v) {vC = v;}


	void				Set(int textureIndex,TVertex&,TVertex&,TVertex&);

inline	void				SetFlag(unsigned char f)				{m_flag |= f;}
inline	void				ClearFlag(unsigned char f)				{m_flag &= ~f;}
inline	bool				TestFlag(unsigned char f)				{return (m_flag & f)!=0x00;}
inline	bool				TestAnyFlag() const						{return m_flag !=0;}
inline	void				ClearAllFlags()							{m_flag =0;}

	// from COTObject interface
	virtual int			GetZForOrderingTable() const;
	virtual void		Draw(CLib3D&);

#ifdef USE_OGL
	virtual void		AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor);
	unsigned int	GetGLTextureName();
#endif /* USE_OGL */


public:

// place first the flags used for drawing
#define TFACE_FLAG_ZCORRECTED			0x0000001
#define	TFACE_FLAG_FLAT					0x0000002	// drawn flat
#define	TFACE_FLAG_TRANS				0x0000004	// drawn transparent
#define	TFACE_FLAG_TRANS_ADDITIVE		0x0000008
#define	TFACE_FLAG_TRANS_SUBSTRATIVE	0x0000010
#define	TFACE_FLAG_TRANS_SHADOW			0x0000020
#define	TFACE_FLAG_TRANS_ADDITIVE_ALPHA	0x0000040
#define	TFACE_FLAG_ZCORRECTED_NO_KEYCOL	0x0000080	// for road
#define	TFACE_MASK_DRAWING				0x00000FF
#define	TFACE_FLAG_ALPHA_MASK			0x0000100

//#define TFACE_FLAG_ZCORRECTED			0x0000100
//#define TFACE_FLAG_FLAT					0x0000200	// drawn flat
#define TFACE_FLAG_FOG					0x0000400	// display fogged (go with FLAT)
//#define TFACE_FLAG_TRANS				0x0000800	// drawn transparent
#define TFACE_FLAG_BILBOARD_TRANS_MID	0x0000800	// drawn transparent 1pt/2
#define TFACE_FLAG_SUBDIV				0x0001000	// must be subdivided (front clip) before draw
#define TFACE_FLAG_BILBOARD_TRANS_ADD	0x0002000	// drawn transparent additive
#define TFACE_FLAG_CULL					0x0004000	// must be backcull tested before draw
#define TFACE_FLAG_BILLBOARD			0x0008000	// it's not a face, but a BILLBOARD Man !
#define TFACE_FLAG_OFFSET_FRONT			0x0010000	// offset in ot table to be in front
#define TFACE_FLAG_OFFSET_BACK			0x0020000	// offset in ot table to be on the back
#define TFACE_FLAG_OFFSET_MIN			0x0040000
#define TFACE_FLAG_OFFSET_MAX			0x0080000
#define TFACE_FLAG_ENVMAP				0x0100000
#define TFACE_FLAG_BILBOARD_STARTEND	0x0200000	// billboard with start x & end x
#define TFACE_FLAG_SUBDIV4				0x0400000	// must be subdivided by 4 before draw (only used with TFACE_FLAG_SUBDIV)
#define TFACE_FLAG_OFFSET_FRONT_MORE	0x0800000	// offset in ot table to be in front (but more than standard FRONT) to be USED with TFACE_FLAG_OFFSET_FRONT !!!
//#define TFACE_FLAG_TRANS_ADDITIVE		0x0800000	
#define TFACE_FLAG_GHOST				0x1000000
//#define TFACE_FLAG_TRANS_SUBSTRATIVE    0x2000000	
#define TFACE_FLAG_OLDENVMAP			0x4000000	// old envmap for menus
#define TFACE_FLAG_ROAD					0x8000000	// road FX
#define TFACE_FLAG_BILBOARD_TRANS_ALPHA	0x10000000	// billboard with alpha
#define TFACE_FLAG_BILBOARD_STENCIL		0x20000000	// additional buffer for nitro
//#define TFACE_FLAG_TRANS_SHADOW		    0x40000000
//#define TFACE_FLAG_TRANS_ADDITIVE_ALPHA 0x80000000
										  
										  
//private:								  
	Vector4s *VxA;                               // transformed position of vertex + light intensity (s)
	UVType uA, vA;                                // uv vertex a

	Vector4s *VxB;
	UVType uB, vB;                                // uv vertex b

	Vector4s *VxC;
	UVType uC, vC;                                // uv vertex c

	const TTexture*		m_texture;
};

// ----------------------------------------------
//
// ----------------------------------------------
class TFakeZFace
	:public TFace
{
public:
	// from COTObject interface
	virtual int			GetZForOrderingTable()const{return 1;}
};


class TBillboard
	:public COTObject
{
public:	
	virtual void		Draw(CLib3D&);
	virtual int			GetZForOrderingTable() const;

#ifdef USE_OGL
	virtual void		AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor);
//	virtual void		AddToOGLRenderingPipeline(Vector4s& pos, int& nQuadsAdded, f32*& pVtx, f32*& pTex);
#endif

	int					size;
	Vector4s			pos;
	TTexture*			m_texture;
	int					start_x, end_x;
#ifdef  USE_OGL
	int					start_y, end_y;
#endif
	short				alpha;
	// WARNING : MUST NOT BE BIGGER THAN A TFACE STRUCT !!!!!

} ;


}//namespace

#endif // _FACE_H_
