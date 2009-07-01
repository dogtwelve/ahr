#include "ConvexHull.h"
#include <stdlib.h>

#include "DevUtil.h"
#include "Mesh.h"

using namespace Lib3D;

class ConvexHullAtomAccessor
{
public:
	inline ConvexHullAtomAccessor(ConvexHullAtom*v,int n):m_vertex(v),m_nbVertex(n){};
	inline int		Size() const {return m_nbVertex;}

	inline ConvexHullAtom& operator[](int i) const {A_ASSERT(i>=0 && i<Size());return m_vertex[i];}

private:
	const int		m_nbVertex;
	ConvexHullAtom*	m_vertex;	
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class IndexArray
{
public:
	IndexArray(int size):m_size(size)
	{
		m_array = NEW int[size];
#if WIN_DEBUG
		for(int i=0;i<size;i++)
			m_array[i]=-666;
#endif
	}
	~IndexArray(){DELETE_ARRAY m_array;}

	int		operator[](int i) const {A_ASSERT(i>=0 && i<m_size);return m_array[i];}
	int&	operator[](int i)		{A_ASSERT(i>=0 && i<m_size);return m_array[i];}

private:
	const int	m_size;
	int*	m_array;
};


// isLeft(): test if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles

//--------------------------------------------------------------
//--
//--	2D convex hull finding class
//--
//--------------------------------------------------------------
int CConvexHull::isLeft(const ConvexHullAtom& P0,const ConvexHullAtom& P1,const ConvexHullAtom& P2)
{
	return (P1.m_Point.x - P0.m_Point.x)*(P2.m_Point.y - P0.m_Point.y) - (P2.m_Point.x - P0.m_Point.x)*(P1.m_Point.y - P0.m_Point.y);
}

CConvexHull::~CConvexHull()
{
	if(m_VerticesIndex)
	{
		DELETE_ARRAY m_VerticesIndex;
		m_VerticesIndex = NULL;
	}
	m_VerticeNum = 0;
}

bool CConvexHull::TooClose(const Vector4s& P0,const Vector4s& P1)
{
	switch(m_ProjectionMode)
	{
	case PLANE_XY:
		if(Lib3D::Abs(P0.x - P1.x) < CONVEX_HULL_TRESHOLD && Lib3D::Abs(P0.y - P1.y) < CONVEX_HULL_TRESHOLD )
			return true;
		return false;
	case PLANE_XZ:
		if(Lib3D::Abs(P0.x - P1.x) < CONVEX_HULL_TRESHOLD && Lib3D::Abs(P0.z - P1.z) < CONVEX_HULL_TRESHOLD )
			return true;
		return false;
	case PLANE_YZ:
		if(Lib3D::Abs(P0.y - P1.y) < CONVEX_HULL_TRESHOLD && Lib3D::Abs(P0.z - P1.z) < CONVEX_HULL_TRESHOLD )
			return true;
		return false;
	}
	A_ASSERT(false); //we should NEVER get to this
	return 0;
}



bool CConvexHull::isValid(const Vector4s& P0)
{

	switch(m_ProjectionMode)
	{
	case PLANE_XY:
		if(P0.z < m_clipping_min || P0.z > m_clipping_max)
			return false;
		return true;
	case PLANE_XZ:
		if(P0.y < m_clipping_min || P0.y > m_clipping_max)
			return false;
		return true;
	case PLANE_YZ:
		if(P0.x < m_clipping_min || P0.x > m_clipping_max)
			return false;
		return true;
	}
	A_ASSERT(false); //we should NEVER get to this
	return 0;
}

int CompareConvexHullAtom( const void *arg1, const void *arg2 )
{
	/* Compare all of both strings: */
	ConvexHullAtom* h1 = (ConvexHullAtom*)arg1;
	ConvexHullAtom* h2 = (ConvexHullAtom*)arg2;

	if(h1->m_Point.x > h2->m_Point.x)
		return 1;

	if(h1->m_Point.x < h2->m_Point.x)
		return -1;

	if(h1->m_Point.y > h2->m_Point.y)
		return 1;

	if(h1->m_Point.y < h2->m_Point.y)
		return -1;

	return 0;
}


void CConvexHull::PrepareVertices(const Lib3D::CGLMesh& mesh, ProjectionPlane ProjectionMode, int clipping_min, int clipping_max)
{
	m_nAtom = 0;
	int nVertexSrc = mesh.NbVertex();
	ConvexHullAtom* temp_array = NEW ConvexHullAtom[nVertexSrc];
	ConvexHullAtomAccessor temp_array_safe(temp_array,nVertexSrc);
	//const TVertex* scan_vertex = mesh.FirstVertex();

	f32 *v = mesh.GetVertices();
	Vector4s	vertex;

	for (int counter = 0; counter < nVertexSrc; counter++)
	{
		vertex.x = (int)*v++;
		vertex.y = (int)*v++;
		vertex.z = (int)*v++;

//		if(isValid(scan_vertex[counter].InitialPos))
		if (isValid(vertex))
		{
			switch(m_ProjectionMode)
			{
				case PLANE_XY:
					temp_array_safe[m_nAtom++] = ConvexHullAtom(Vector2s(vertex.x, vertex.y), counter);
					break;

				case PLANE_XZ:
					temp_array_safe[m_nAtom++] = ConvexHullAtom(Vector2s(vertex.x, vertex.z), counter);
					break;

				case PLANE_YZ:
					temp_array_safe[m_nAtom++] = ConvexHullAtom(Vector2s(vertex.y, vertex.z), counter);
					break;
			}
		}
	}
	m_AtomArray = NEW ConvexHullAtom[m_nAtom];
	ConvexHullAtomAccessor m_AtomArray_safe(m_AtomArray,m_nAtom);

	for (int counter = 0; counter < m_nAtom;counter++)
		m_AtomArray_safe[counter] = temp_array_safe[counter];

	DELETE_ARRAY temp_array;	
	temp_array = NULL;

	//sort the final array
	qsort((void *)m_AtomArray,m_nAtom, sizeof( ConvexHullAtom), CompareConvexHullAtom );
}


CConvexHull::CConvexHull(CGLMesh& mesh,
						 ProjectionPlane ProjectionMode,
						 int clipping_min,
						 int clipping_max):
	m_VerticesIndex(NULL),
	m_VerticeNum(0),
	m_ProjectionMode(ProjectionMode),
	m_AtomArray(NULL),
	m_nAtom(0),
	H(NULL)
{
	m_nAtom = 0; 
	if(clipping_min > clipping_max)
	{
		m_clipping_min = clipping_max;
		m_clipping_max = clipping_min;
	}
	else
	{
		m_clipping_min = clipping_min;
		m_clipping_max = clipping_max;
	}

	PrepareVertices(mesh, ProjectionMode, clipping_min, clipping_max);

	m_VerticeNum = ComputeHull();
	A_ASSERT(m_VerticeNum < 5000 && m_VerticeNum > -5000);
	m_VerticesIndex = NEW int[m_VerticeNum];
//	const TVertex* scan_vertex = mesh.FirstVertex();

	int real_counter = 0;
	for(int counter = 0; counter < m_VerticeNum; counter++)
	{
		bool too_close = false;
		for(int counter2 = 0; counter2 < real_counter && !too_close; counter2++)
		{
			//too_close = TooClose(	scan_vertex[m_VerticesIndex[counter2]].InitialPos,
			//						scan_vertex[H[counter].m_index].InitialPos);

			Vector4s vec1 = mesh.GetVector(m_VerticesIndex[counter2]);
			Vector4s vec2 = mesh.GetVector(H[counter].m_index);
			too_close = TooClose(vec1, vec2);
									
		}
		if(!too_close)
			m_VerticesIndex[real_counter++] = H[counter].m_index;
	}
	m_VerticeNum = real_counter;
	DELETE_ARRAY H;
	H = NULL;
	DELETE_ARRAY m_AtomArray;
	m_AtomArray = NULL;
}



int CConvexHull::ComputeHull()
{
	H = NEW ConvexHullAtom[m_nAtom];

	ConvexHullAtomAccessor H_safe(H,m_nAtom);
	ConvexHullAtomAccessor m_AtomArray_safe(m_AtomArray,m_nAtom);
	// the output array H_safe[] will be used as the stack
	int    bot=0, top=(-1);  // indices for bottom and top of the stack
	int    i;                // array scan index

	// Get the indices of points with min x-coord and min|max y-coord
	int minmin = 0, minmax;
	int xmin = m_AtomArray_safe[0].m_Point.x;
	for (i=1; i<m_nAtom; i++)
		if (m_AtomArray_safe[i].m_Point.x != xmin) 
			break;

	minmax = i-1;
	if (minmax == m_nAtom-1) 
	{
		// degenerate case: all x-coords == xmin
		H_safe[++top] = m_AtomArray_safe[minmin];
		if (m_AtomArray_safe[minmax].m_Point.y != m_AtomArray_safe[minmin].m_Point.y) // a nontrivial segment
			H_safe[++top] = m_AtomArray_safe[minmax];

		H_safe[++top] = m_AtomArray_safe[minmin];           // add polygon endpoint
		return top+1;
	}

	// Get the indices of points with max x-coord and min|max y-coord
	int maxmin, maxmax = m_nAtom-1;
	int xmax = m_AtomArray_safe[m_nAtom-1].m_Point.x;

	for (i=m_nAtom-2; i>=0; i--)
		if (m_AtomArray_safe[i].m_Point.x != xmax) 
			break;

	maxmin = i+1;

	// Compute the lower hull on the stack H_safe
	H_safe[++top] = m_AtomArray_safe[minmin];      // push minmin point onto stack
	i = minmax;
	while (++i <= maxmin)
	{
		// the lower line joins m_AtomArray_safe[minmin] with m_AtomArray_safe[maxmin]
		if (isLeft( m_AtomArray_safe[minmin], 
			m_AtomArray_safe[maxmin], 
			m_AtomArray_safe[i]) >= 0 && 
			i < maxmin)

			continue;          // ignore m_AtomArray_safe[i] above or on the lower line

		while (top > 0)        // there are at least 2 points on the stack
		{
			// test if m_AtomArray_safe[i] is left of the line at the stack top
			if (isLeft( H_safe[top-1], 
				H_safe[top], 
				m_AtomArray_safe[i]) > 0)
				break;         // m_AtomArray_safe[i] is a new hull vertex
			else
				top--;         // pop top point off stack
		}
		H_safe[++top] = m_AtomArray_safe[i];       // push m_AtomArray_safe[i] onto stack
	}

	// Next, compute the upper hull on the stack H_safe above the bottom hull
	if (maxmax != maxmin)      // if distinct xmax points
		H_safe[++top] = m_AtomArray_safe[maxmax];  // push maxmax point onto stack

	bot = top;                 // the bottom point of the upper hull stack
	i = maxmin;
	while (--i >= minmax)
	{
		// the upper line joins m_AtomArray_safe[maxmax] with m_AtomArray_safe[minmax]
		if (isLeft( m_AtomArray_safe[maxmax], 
			m_AtomArray_safe[minmax], 
			m_AtomArray_safe[i]) >= 0 && i > minmax)
			continue;          // ignore m_AtomArray_safe[i] below or on the upper line

		while (top > bot)    // at least 2 points on the upper stack
		{
			// test if m_AtomArray_safe[i] is left of the line at the stack top
			if (isLeft( H_safe[top-1], 
				H_safe[top], 
				m_AtomArray_safe[i]) > 0)
				break;         // m_AtomArray_safe[i] is a new hull vertex
			else
				top--;         // pop top point off stack
		}
		H_safe[++top] = m_AtomArray_safe[i];       // push m_AtomArray_safe[i] onto stack
	}
	if (minmax != minmin)
		H_safe[++top] = m_AtomArray_safe[minmin];  // push joining endpoint onto stack

	return top+1;
}



void CConvexHull::ApplyContraints(Lib3D::CGLMesh& mesh,ProjectionPlane ProjectionMode,int clipping_min,int clipping_max)
{
	int counter;
	if(clipping_min > clipping_max)
	{
		m_clipping_min = clipping_max;
		m_clipping_max = clipping_min;
	}
	else
	{
		m_clipping_min = clipping_min;
		m_clipping_max = clipping_max;
	}
	m_ProjectionMode = ProjectionMode;

	IndexArray new_VerticesIndex(m_VerticeNum);
	int vert_count = 0;
	//const TVertex* scan_vertex = mesh.FirstVertex();

	for (counter = 0; counter < m_VerticeNum; counter++)
		//if (isValid(scan_vertex[m_VerticesIndex[counter]].InitialPos))
		if (isValid(mesh.GetVector(m_VerticesIndex[counter])))
			new_VerticesIndex[vert_count++] = m_VerticesIndex[counter];

	DELETE_ARRAY m_VerticesIndex;
	m_VerticesIndex = NULL;
	m_VerticeNum = vert_count;
	m_VerticesIndex = NEW int[m_VerticeNum];

	for(counter = 0; counter < m_VerticeNum; counter++)
		m_VerticesIndex[counter] = new_VerticesIndex[counter];

//	DELETE_ARRAY new_VerticesIndex;
//	new_VerticesIndex = NULL;
}

int CConvexHull::TestConstraint(Lib3D::CGLMesh& mesh,ProjectionPlane ProjectionMode,int clipping_min,int clipping_max)
{
	int counter;
	if(clipping_min > clipping_max)
	{
		m_clipping_min = clipping_max;
		m_clipping_max = clipping_min;
	}
	else
	{
		m_clipping_min = clipping_min;
		m_clipping_max = clipping_max;
	}
	m_ProjectionMode = ProjectionMode;

	int vert_count = 0;
//	const TVertex* scan_vertex = mesh.FirstVertex();

	for(counter = 0; counter < m_VerticeNum; counter++)
		//if (isValid(scan_vertex[m_VerticesIndex[counter]].InitialPos))
		if (isValid(mesh.GetVector(m_VerticesIndex[counter])))
			vert_count++;

	return vert_count;
}

