#include "config.h"

#include "devutil.h"
#include "Vector.h"
#include <math.h>

#include "fsqrt.h"

#include "imath.h"
#include "Matrix.h"
#include <limits.h>

const Vector4s::Type	kNullvector[4] = {0,0,0,0};

using namespace Lib3D;

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector2s::Type Vector2s::Length() const
{
	return FSqrt(Length2());
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector2s::Type Vector2s::SafeLength() const
{
	int const l1Norm = L1Norm();
	if (l1Norm >= 32768)
	{
		return Product12(FSqrt(Square(Quotient12(x, l1Norm)) + Square(Quotient12(y, l1Norm))), l1Norm);
	}
	return FSqrt(Length2());
}


// ---------------------------------------------------------------------------
//	check if a point is on the right side of a vector (a vector from the 
//	origin to vec and behond)
//	Right side while looking in the direction of the vector
// ---------------------------------------------------------------------------
bool	Vector2s::RightSide(const Vector2s& vec,const Vector2s& point)
{	
	const Type d = Dot(vec.Normal(),point);
	return d <= 0;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const Vector4s& Vector4s::NullVector()
{
	return *((const Vector4s*)kNullvector);
}


#ifdef WIN_DEBUG
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void _CheckLimit(double val)
{
	A_ASSERT(val <= double(INT_MAX) && val >= double(INT_MIN));
};
#endif



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Vector2s::Normalize()
{
	CHK_MULT(x,x);
	CHK_MULT(y,y);

	CHECK_LIMIT( double(x) * double(1<<COS_SIN_SHIFT));
	CHECK_LIMIT( double(y) * double(1<<COS_SIN_SHIFT));

	const int Dist2 = (x*x) + (y*y);   // 2*COS_SIN_SHIFT bits average result for trigo vectors
	const int Dist  = FSqrt(Dist2);

	//// rax - prevent Division by 0
	//if (Dist == 0)
	//	return;

	x = (x << COS_SIN_SHIFT) / Dist;
	y = (y << COS_SIN_SHIFT) / Dist;  
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool Vector2s::SafeNormalize()
{
	int const Dist = SafeLength();
	if (Dist == 0)
	{
		return false;
	}
	x = QuotientCOS_SIN_SHIFT(x, Dist);
	y = QuotientCOS_SIN_SHIFT(y, Dist);
	return true;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Vector2s::Resize(int in_nNewLength)
{
	CHK_MULT(x,x);
	CHK_MULT(y,y);
	
	int Dist2 = (x*x) + (y*y);   // 2*COS_SIN_SHIFT bits average result for rot vectors
	int Dist  = FSqrt(Dist2);
	
	x = ((int)x * in_nNewLength / Dist);
	y = ((int)y * in_nNewLength / Dist);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool Vector2s::SafeResize(int in_nNewLength)
{
	int const Dist = SafeLength();
	if (Dist == 0)
	{
		return false;
	}
	x = ProductQuotient(x, in_nNewLength, Dist);
	y = ProductQuotient(y, in_nNewLength, Dist);
	return true;
}


// vector used as 3D array index
int Vector4s::LinIndex(int Ix, int Iy, int Iz)
{ 
	int LinearIndex = Ix*(y*z) + Iy*z + Iz;
	return LinearIndex;
}

void Vector4s::Scale(int s)
{ 
	CHK_MULT(s,x);CHK_MULT(s,y);CHK_MULT(s,z);
	
	x = ((s * x) >> COS_SIN_SHIFT); 
	y = ((s * y) >> COS_SIN_SHIFT); 
	z = ((s * z) >> COS_SIN_SHIFT); 
}

// get min values
void Vector4s::GetMin(const Vector4s *v2)
{
	if (v2->x < x) x = v2->x;
	if (v2->y < y) y = v2->y;
	if (v2->z < z) z = v2->z;
}

// get max values
void Vector4s::GetMax(const Vector4s *v2)
{
	if (v2->x > x) x = v2->x;
	if (v2->y > y) y = v2->y;
	if (v2->z > z) z = v2->z;
}

// cross product with v2, result -> v3
// at least one of the 2 vectors must be normalized, result is normalized
void Vector4s::CrossShift(const Vector4s *V2, Vector4s *Res)const
{
	CHK_MULT(y , V2->z);CHK_MULT(z , V2->y);
	CHK_MULT(z , V2->x);CHK_MULT(x , V2->z);
	CHK_MULT(x , V2->y);CHK_MULT(y , V2->x);
	
	
	Res->x = ((int)y * V2->z - (int)z * V2->y) >> COS_SIN_SHIFT;
	Res->y = ((int)z * V2->x - (int)x * V2->z) >> COS_SIN_SHIFT;
	Res->z = ((int)x * V2->y - (int)y * V2->x) >> COS_SIN_SHIFT;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int Vector4s::Length() const
{
	return FSqrt(Length2());
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector4s::Type Vector4s::SafeLength() const
{
	int const l1Norm = L1Norm();
	if (l1Norm > 26754)
	{
		return Product12(FSqrt(Square(Quotient12(x, l1Norm)) + Square(Quotient12(y, l1Norm)) + Square(Quotient12(z, l1Norm))), l1Norm);
	}
	return FSqrt(Length2());
}

// integer normalize, can be used for rotation vectors
void Vector4s::Normalize()
{
	CHK_MULT(x,x);
	CHK_MULT(y,y);
	CHK_MULT(z,z);
	
	// normalize x
	int Dist2 = (x*x) + (y*y) + (z*z);   // 2*COS_SIN_SHIFT bits average result for rot vectors
	int Dist  = FSqrt(Dist2);

//#ifdef DEBUG_WIN32
	if (Dist == 0)
		return;
//#endif
	
	x = ((int)x << COS_SIN_SHIFT) / Dist;
	y = ((int)y << COS_SIN_SHIFT) / Dist;
	z = ((int)z << COS_SIN_SHIFT) / Dist;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool Vector4s::SafeNormalize()
{
	int const Dist = SafeLength();
	if (Dist == 0)
	{
		return false;
	}
	x = QuotientCOS_SIN_SHIFT(x, Dist);
	y = QuotientCOS_SIN_SHIFT(y, Dist);
	z = QuotientCOS_SIN_SHIFT(z, Dist);
	return true;
}


void Vector4s::Resize(int in_nNewLength)
{
	CHK_MULT(x,x);
	CHK_MULT(y,y);
	CHK_MULT(z,z);
	
	int Dist2 = (x*x) + (y*y) + (z*z);   // 2*COS_SIN_SHIFT bits average result for rot vectors
	int Dist  = FSqrt(Dist2);
	
	x = ((int)x * in_nNewLength / Dist);
	y = ((int)y * in_nNewLength / Dist);
	z = ((int)z * in_nNewLength / Dist);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool Vector4s::SafeResize(int in_nNewLength)
{
	int const Dist = SafeLength();
	if (Dist == 0)
	{
		return false;
	}
	x = ProductQuotient(x, in_nNewLength, Dist);
	y = ProductQuotient(y, in_nNewLength, Dist);
	z = ProductQuotient(z, in_nNewLength, Dist);
	return true;
}

// ---------------------------------------------------------------------------
// static
// ---------------------------------------------------------------------------
const int*	Vector4s::GetReciprocalAxis(int axis)
{
	const static int kPlanes[3][2] = {{1,2},{0,2},{0,1}};
	A_ASSERT(axis>=0 && axis < 3);
	return kPlanes[axis];
}



int Vector4s::GetMainAxis() const
{
	const Type x1 = Abs(x);
	const Type y1 = Abs(y);
	const Type z1 = Abs(z);

	if(x1 >= y1)
	{
		if( x1 >= z1)
			return 0;
		else
			return 2;
	}
	else
	{
		if(y1 >= z1)
			return 1;
		else
			return 2;
	}
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Vector4s::SelfRotateY(int a)
{  
	const Type s = Sinus(a);
	const Type c = Cosinus(a);
	
	const int x1 =	(x*c  + z*s + HALF_PRECISION) >> COS_SIN_SHIFT;
	z =							(x*-s + z*c + HALF_PRECISION) >> COS_SIN_SHIFT;
	x = x1;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector4s Vector4s::GetRotatedY(int a) const
{
	const Type s = Sinus(a);
	const Type c = Cosinus(a);
	
	return Vector4s(	(x*c  + z*s + HALF_PRECISION) >> COS_SIN_SHIFT,
		y,
		(x*-s + z*c + HALF_PRECISION) >> COS_SIN_SHIFT);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector4s Vector4s::GetProjection(const Vector4s& v, const Vector4s& normal)
{
    Vector4s proj = normal;
    proj.Normalize();
    int nDot = Dot(v, proj);
    nDot >>= COS_SIN_SHIFT;
    proj.x = proj.x * nDot >> COS_SIN_SHIFT;
    proj.y = proj.y * nDot >> COS_SIN_SHIFT;
    proj.z = proj.z * nDot >> COS_SIN_SHIFT;
    return proj;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector4s Vector4s::GetReflexion(const Vector4s& v, const Vector4s& normal)
{
    Vector4s proj = GetProjection(v, normal);
    return proj + (proj - v);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Vector2s::SelfRotate(int a)
{  
	const Type s = Sinus(a);
	const Type c = Cosinus(a);
	
	const int x1 =	(x*c  + y*s + HALF_PRECISION) >> COS_SIN_SHIFT;
	y =							(x*-s + y*c + HALF_PRECISION) >> COS_SIN_SHIFT;
	x = x1;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Vector2s Vector2s::GetRotated(int a) const
{
	const Type s = Sinus(a);
	const Type c = Cosinus(a);
	
	return Vector2s(	(x*c  + y*s + HALF_PRECISION) >> COS_SIN_SHIFT,
						(x*-s + y*c + HALF_PRECISION) >> COS_SIN_SHIFT);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool RayPlaneInter(const Vector4s& tA,const Vector4s& normal,const Vector4s& rO,const Vector4s& rV,Vector4s& o_point)
{
	int n_dot_v = Vector4s::Dot(normal,rV);
	
	if(n_dot_v!=0)	// Test Paralelism
	{
		Vector4s Q = rO - tA;	// Segment origin, when translated to make d==0
		
		int n_dot_q = Vector4s::Dot(normal,Q);
		
		int t;
		{
			// All this code just for a division !!!
			if(n_dot_q < 0)
			{
				n_dot_q = -n_dot_q;
				n_dot_v = -n_dot_v;
			}
			
			if(n_dot_q &0x7F800000)
			{
				t = -n_dot_q / /**/DownShift16(n_dot_v);
			}
			else if(n_dot_q & 0x007F8000)
				t = -(n_dot_q<<8) / /**/DownShift8(n_dot_v);
			else
				t = -(n_dot_q<<16) / n_dot_v;
			
            CHK_PRECISION(-double(n_dot_q) * (1<<16) / n_dot_v, t, 5);
		}
		
		// insert "t" in the line equation;
		// We do not check if t is restricted to the 
		//	segment since we already checked if both end 
		//	of the segment are on separate side of the plane
		o_point.Init(	/**/DownShift16(rV.x * t) + rO.x,
						/**/DownShift16(rV.y * t) + rO.y,
						/**/DownShift16(rV.z * t) + rO.z);
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool RaytriangleIntersect(const Vector4s& tA,const Vector4s& tB,const Vector4s& tC,const Vector4s& normal,	// Triangles vertexes and Normal
						  const Vector4s& rO,const Vector4s& rV,	// Ray Origin and Direction
						  bool doubleSided)
{
	{	// 1- Check if both points (src and dst) are on the same side of the plane:
		// also check if the source is on the front side (backface culling)
		const bool c1 = FrontSide(tA,normal,rO);
		
		if(c1 ==false && !doubleSided)
			return false;
		
		const bool c2 = FrontSide(tA,normal,rO + rV);
		
		if(c1==c2)
			return false;
	}
	
	Vector4s point;
	if(RayPlaneInter(tA,normal,rO,rV,point))
	{
		const int* raxis = Vector4s::GetReciprocalAxis(normal.GetMainAxis());
		
		const Vector2s	p0 = point.GetVector2s(raxis);
		
		const Vector2s	pa = tA.GetVector2s(raxis);
		const Vector2s	pb = tB.GetVector2s(raxis);
		const Vector2s	pc = tC.GetVector2s(raxis);
		
		// If the point is on the same side of each of these vectors, 
		// it means it is inside the polygon.
		
        int d1 = Vector2s::Dot((pb-pa).Normal(),(p0-pa));
        int d2 = Vector2s::Dot((pc-pb).Normal(), (p0-pb));
        if ((d1 <= 0 && d2 <= 0) || (d1 >= 0 && d2 >= 0))
        {
            int d3 = Vector2s::Dot((pa-pc).Normal(), (p0-pc));
            if ((d1 <= 0 && d2 <= 0 && d3 <= 0) || (d1 >= 0 && d2 >= 0 && d3 >= 0))
            {
                return true;
            }
        }
	}
	return false;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int FindRaytriangleIntersectionPoint(	const Vector4s& tA,const Vector4s& tB,const Vector4s& tC,const Vector4s& normal,	// Triangles vertexes and Normal
									 const Vector4s& rO,Vector4s& dst,	// Ray Origin and destination
									 bool doubleSided)
{
	{	// 1- Check if both points (src and dst) are on the same side of the plane:
		// also check if the source is on the front side (backface culling)
		const bool c1 = FrontSide(tA,normal,rO);
		
		if(c1 ==false && !doubleSided)
			return INT_MAX;
		
		const bool c2 = FrontSide(tA,normal,dst);
		
		if(c1==c2)
			return INT_MAX;
	}
	
	Vector4s point;
	if(RayPlaneInter(tA,normal,rO,dst - rO,point))
	{
		const int minDist	= (dst - rO).Length2();
		const int dist2		= (point - rO).Length2();
		
		if(dist2 < minDist)
		{
			const int* raxis = Vector4s::GetReciprocalAxis(normal.GetMainAxis());
			
			const Vector2s	p0 = point.GetVector2s(raxis);
			
			const Vector2s	pa = tA.GetVector2s(raxis);
			const Vector2s	pb = tB.GetVector2s(raxis);
			const Vector2s	pc = tC.GetVector2s(raxis);
			
			// If the point is on the same side of each of these vectors, 
			// it means it is inside the polygon.
			
			int d1 = Vector2s::Dot((pb-pa).Normal(),(p0-pa));
			int d2 = Vector2s::Dot((pc-pb).Normal(), (p0-pb));
			
			if ((d1 <= 0 && d2 <= 0) || (d1 >= 0 && d2 >= 0))
			{
				int d3 = Vector2s::Dot((pa-pc).Normal(), (p0-pc));
				if ((d1 <= 0 && d2 <= 0 && d3 <= 0) || (d1 >= 0 && d2 >= 0 && d3 >= 0))
				{
					dst = point;
					return dist2;
				}
			}
		}
	}
	return INT_MAX;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int PointRayDist2(const Vector4s& P0,const Vector4s& V,const Vector4s& Q)
{
	const Vector4s QP = Q - P0;
	const int d2 = QP.Length2() - Square(Vector4s::Dot(QP,V))	/ V.Length2();
	
	return d2;
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool Vector2s::Intersect(const Vector2s& A,const Vector2s& B,const Vector2s& C,const Vector2s& D)
{
	A_ASSERT(A.x <= B.x && C.x <= D.x);
	
	const int BAx = B.x-A.x;
	const int BAy = B.y-A.y;
	
	const int DCx = D.x-C.x;
	const int DCz = D.y-C.y;
	
	const int d = BAx*DCz - BAy*DCx;
	
	if(d==0)	// Parellel
		return false;
	
	const int ACz = A.y - C.y;
	const int ACx = A.x - C.x;
	
	const int r = ACz*DCx - ACx*DCz;
	const int s = ACz*BAx - ACx*BAy;
	
	if(d<0)
	{
		if( r<=0 && r>=d && s<=0 && s>=d)
			return true;			
	}
	else
	{
		if( r>=0 && r<=d && s>=0 && s<=d)
			return true;		
	}
	return false;
}













