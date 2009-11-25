#ifndef __CONVEXHULL_H
#define __CONVEXHULL_H

#include "config.h"
#include "Vector.h"

namespace Lib3D
{
	class CGLMesh;
}


//--------------------------------------------------------------
//--
//--	2D convex hull finding class
//--
//--------------------------------------------------------------
class ConvexHullAtom
{
public:
	Vector2s	m_Point;
	int			m_index;
	ConvexHullAtom(){}
	ConvexHullAtom(const Vector2s& Point_in,int index_in):m_Point(Point_in),m_index(index_in){}
	ConvexHullAtom& operator=(const ConvexHullAtom& in)
	{
		m_Point.x = in.m_Point.x;
		m_Point.y = in.m_Point.y;
		m_index = in.m_index;
		return *this;
	}

};


#define CONVEX_HULL_TRESHOLD 1
class CConvexHull
{
public:
	enum ProjectionPlane{PLANE_XY,PLANE_XZ,PLANE_YZ};
	CConvexHull(Lib3D::CGLMesh& mesh,ProjectionPlane ProjectionMode,int clipping_min,int clipping_max);
	void ApplyContraints(Lib3D::CGLMesh& mesh,ProjectionPlane ProjectionMode,int clipping_min,int clipping_max);
	int TestConstraint(Lib3D::CGLMesh& mesh,ProjectionPlane ProjectionMode,int clipping_min,int clipping_max);
	~CConvexHull();
	int GetNumVertices()const{return m_VerticeNum;}
	int GetVerticeIndex(int index_in)const{return m_VerticesIndex[index_in];}


private:
	void PrepareVertices(const Lib3D::CGLMesh& mesh,ProjectionPlane ProjectionMode,int clipping_min,int clipping_max);
	int ComputeHull();
	int isLeft(const ConvexHullAtom& P0,const ConvexHullAtom& P1,const ConvexHullAtom& P2);
	//int isLeft(const Vector4s& P0,const Vector4s& P1,const Vector4s& P2);
	bool isValid(const Vector4s& P0);
	bool TooClose(const Vector4s& P0,const Vector4s& P1);


	ConvexHullAtom* m_AtomArray;
	int				m_nAtom;

	ConvexHullAtom* H;

	int*	m_VerticesIndex;
	int		m_VerticeNum;

	int		m_clipping_min;
	int		m_clipping_max;
	ProjectionPlane m_ProjectionMode;
};
//}



#endif// __CONVEXHULL_H
