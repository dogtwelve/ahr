#ifndef _SOFTLINE_H
#define _SOFTLINE_H

#include "Lib3d2/Lib3D.h"
#include "Lib3d2/face.h"
#include "imath.h"


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CSoftLine3D
	: public COTObject
{
	enum
	{ 
		kStandardLength =128,
		kWidth = 8,
	};
public:

	virtual int			GetZForOrderingTable() const {return Min(m_startPosition.z, m_endPosition.z);}
	virtual void		Draw(CLib3D& lib3d);

#ifdef USE_OGL
	virtual void AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor);
#endif
//private: [FIXME] could clean this up using an Init function
	// Note:  The S part of the vectors store the width
	Vector4s	m_startPosition;
	Vector4s	m_endPosition;
	int			m_startWidth;
	int			m_endWidth;
	bool		m_drawEndsParallelToGround;
	int			m_Severity12;		// from 0 to 4095, for choosing the color gradient level

private:
	static void DrawScanline(	CLib3D& lib3d,
								unsigned short * const line, 
								int const xLeft, 
								int const xRight, 
								int const sLeft, 
								int const sRight, 
								int const * const texture);
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CFakeZAALine3D
	: public COTObject
{
public:
	virtual int			GetZForOrderingTable() const {return 1;}
	virtual void		Draw(CLib3D& lib3d);
	Vector2s			m_startPosition;
	Vector2s			m_endPosition;
	unsigned short		m_color;


};
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CFakeZAALine3DGradient
	: public COTObject
{
public:
	virtual int			GetZForOrderingTable() const {return -m_zdepth;}
	virtual void		Draw(CLib3D& lib3d);

#ifdef USE_OGL
	virtual void		AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor);
#endif

	Vector2s			m_startPosition;
	Vector2s			m_endPosition;
	int					m_zdepth;
	unsigned short*		m_Gradient; //just a reference, the object does not own the datas
	int					m_ColorStart;
	int					m_ColorEnd;
};



#endif
