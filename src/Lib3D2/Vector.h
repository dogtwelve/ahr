#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "GenDef.h"
#include "config.h"
#include "devutil.h"
#include "imath.h"


// 2D  vector, used in collision detection
class Vector2s
{
	typedef long Type;
public:
	Type x;
	Type y;

	Vector2s()	{};
	Vector2s(Type x1,Type y1):x(x1),y(y1){};

	inline void		Init(Type x2, Type y2)							{ x = x2; y = y2; };
	inline void		Add(const Vector2s *v2)							{ x += v2->x; y += v2->y; };
	inline void		GetDelta(const Vector2s *A, const Vector2s *B)	{ x = A->x - B->x; y = A->y - B->y; };  
	inline Type		Length2() const									{CHK_MULT(x,x);CHK_MULT(y,y);return x*x + y*y; };
	Type			Length() const;
	Type			SafeLength() const;			// works even for large vectors
	inline Type L1Norm() const { return Lib3D::Max(Lib3D::Abs(x), Lib3D::Abs(y)); }

	Vector2s operator-() const {return Vector2s(-x,-y);}

	void	operator+=(const Vector2s& v) {x+=v.x;y+=v.y;}
	void	operator-=(const Vector2s& v) {x-=v.x;y-=v.y;}

	void	operator/=(int i) {x/=i;y/=i;}
	void	operator*=(int i) {	CHK_MULT(x,i);CHK_MULT(y,i);x*=i;y*=i;}

	Vector2s & operator >>= (int i) { x >>= i; y >>= i; return *this; }
	Vector2s & operator <<= (int i) { x <<= i; y <<= i; return *this; }

    inline bool operator==(const Vector2s v2) const { return x == v2.x && y == v2.y; }
    inline bool operator!=(const Vector2s v2) const { return x != v2.x || y != v2.y; }

	void				Normalize();
	bool SafeNormalize();						// works even for large vectors, returns true if old size is nonzero
	inline Vector2s		Normal() const {return Vector2s(-y,x);}
	void Resize(int in_nNewLength);
	bool SafeResize(int in_nNewLength);			// works even for large vectors, returns true if old size is nonzero

	inline static int	Dot(const Vector2s& v1,const Vector2s& v2) {CHK_MULT(v1.x,v2.x);CHK_MULT(v1.y,v2.y);return v1.x*v2.x + v1.y*v2.y; };
	inline static int	Cross(const Vector2s& v1,const Vector2s& v2) {CHK_MULT(v1.x,v2.y);CHK_MULT(v1.y,v2.x);return v1.x*v2.y - v1.y*v2.x; };
	static bool			Intersect(const Vector2s& A,const Vector2s& B,const Vector2s& C,const Vector2s& D);
	
	inline bool			RightSide(const Vector2s& point) const {return RightSide(*this,point);}
	static bool			RightSide(const Vector2s& vec,const Vector2s& point);	

	void			SelfRotate(int);
	Vector2s	GetRotated(int) const;
};
#define Vector2s0 Vector2s(0,0)

class Vector4s
{
public:
	typedef long Type;

	Type x;
	Type y;
	Type z;
protected:
//	Type s;
public:

	inline Vector4s(){};
	inline Vector4s(int ix,int iy,int iz):x(ix),y(iy),z(iz){};
//	inline Vector4s(int ix,int iy,int iz,int is):x(ix),y(iy),z(iz),s(is){};

	inline Vector4s(const Vector4s& v):x(v.x),y(v.y),z(v.z){};
	explicit inline Vector4s(Type const * t):x(t[0]),y(t[1]),z(t[2]){};
	
	inline void Init(const Vector4s *v2) { x = v2->x; y = v2->y; z = v2->z; };
	inline void Init(const Vector4s& v2) { x = v2.x; y = v2.y; z = v2.z; };

	inline void Init(Type x2, Type y2, Type z2) { x = x2; y = y2; z = z2; };
	//inline void Init(Type x2, Type y2, Type z2,Type s2) { x = x2; y = y2; z = z2; s=s2;};
	inline void Copy(Vector4s *v2) { v2->x = x; v2->y = y;  v2->z = z; };	
	inline void Add(const Vector4s *v2) { x += v2->x; y += v2->y; z += v2->z; };
	inline void Add(Type x2, Type y2, Type z2) { x += x2; y += y2; z += z2; };
	inline void Sub(const Vector4s *v2) { x -= v2->x; y -= v2->y; z -= v2->z; };
	inline void Sub(Type x2, Type y2, Type z2) { x -= x2; y -= y2; z -= z2; };
  

	inline static int	Dot(const Vector4s& v1,const Vector4s& v2)	
	{
		CHK_MULT(v1.x,v2.x);
		CHK_MULT(v1.y,v2.y);
		CHK_MULT(v1.z,v2.z);
		return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; 
	};
	inline static int	DotXZ(const Vector4s& v1,const Vector4s& v2)	
	{
		CHK_MULT(v1.x,v2.x);
		CHK_MULT(v1.z,v2.z);
		return v1.x*v2.x + v1.z*v2.z; 
	};

	inline Type L1Norm() const { return Lib3D::Max(Lib3D::Max(Lib3D::Abs(x), Lib3D::Abs(y)), Lib3D::Abs(z)); }

	inline int  Length2(void) const { return (int)x*x + (int)y*y + (int)z*z; };
	int					Length() const;
	Type			SafeLength() const;			// works even for large vectors

	inline void GetDelta(const Vector4s *A, const Vector4s *B) { x = A->x - B->x; y = A->y - B->y; z = A->z - B->z; };
	inline int  EqualXYZ(const Vector4s *v2) const { return (x == v2->x) && (y == v2->y) && (z == v2->z); };
	inline int  Sup(const Vector4s *v2) { return (x > v2->x) || (y > v2->y) || (z > v2->z); };
	inline int  Inf(const Vector4s *v2) { return (x < v2->x) || (y < v2->y) || (z < v2->z); };



	int  LinIndex(int Ix, int Iy, int Iz);                   // vector used as 3D array index
	void Scale(int s);
	void GetMin(const Vector4s *v2);
	void GetMax(const Vector4s *v2);
	void CrossShift(const Vector4s *V2, Vector4s *Res)const;      // use if one input vector is normalized
	void Normalize();                           // fast normalize, require to pass square root object ptr
	bool SafeNormalize();						// works even for large vectors, returns true if old size is nonzero
	void Resize(int in_nNewLength);
	bool SafeResize(int in_nNewLength);			// works even for large vectors, returns true if old size is nonzero

	inline void	Clone(const Vector4s& v)	{Init(&v);}
	inline void Clear(){x=0;y=0;z=0;}


	int				GetMainAxis() const;
	inline Vector2s	GetVector2s(const int* raxis) const {return Vector2s(	operator[](raxis[0]),operator[](raxis[1]));}
	inline Vector2s	GetVector2s(int excludedAxis) const {return GetVector2s(GetReciprocalAxis(excludedAxis));}
	


	Type	operator[](int i) const {A_ASSERT(i>=0 && i<3); return ((const Type*)(this))[i];}


	Vector4s operator-() const {return Vector4s(-x,-y,-z);}

	bool operator==(const Vector4s& v) const {return x==v.x && y==v.y && z==v.z;}
	bool operator!=(const Vector4s& v) const {return x!=v.x || y!=v.y || z!=v.z;}

	void	operator+=(const Vector4s& v) {x+=v.x;y+=v.y;z+=v.z;}
	void	operator-=(const Vector4s& v) {x-=v.x;y-=v.y;z-=v.z;}

	void	operator/=(int i) {x/=i;y/=i;z/=i;}
	void	operator*=(int i) {	CHK_MULT(x,i);CHK_MULT(y,i);CHK_MULT(z,i);x*=i;y*=i;z*=i;}
	void	operator*=(const Vector4s& v) {	CHK_MULT(x,v.x);CHK_MULT(y,v.y);CHK_MULT(z,v.z);x*=v.z;y*=v.y;z*=v.z;}

	void	operator >>= (int i) {x>>=i;y>>=i;z>>= i;}
	void	operator <<= (int i) {x<<=i;y<<=i;z<<= i;}

	void			SelfRotateY(int);
	Vector4s		GetRotatedY(int) const;

    static Vector4s GetProjection(const Vector4s& v, const Vector4s& normal);
    static Vector4s GetReflexion(const Vector4s& v, const Vector4s& normal);
	
	static inline Vector4s Cross(const Vector4s &V1,const Vector4s &V2)
	{
		CHK_MULT(V1.x,V2.y);CHK_MULT(V1.y,V2.x);CHK_MULT(V1.y,V2.z);CHK_MULT(V1.z,V2.y);CHK_MULT(V1.z,V2.x);CHK_MULT(V1.x,V2.z);

		return Vector4s(	V1.y * V2.z - V1.z * V2.y,
							V1.z * V2.x - V1.x * V2.z,
							V1.x * V2.y - V1.y * V2.x);
	}

	static inline Type CrossY(const Vector4s &V1,const Vector4s &V2)
	{
		CHK_MULT(V1.z,V2.x);CHK_MULT(V1.x,V2.z);
		return V1.z * V2.x - V1.x * V2.z;
	}

	static const Vector4s& NullVector();
	static const int*	GetReciprocalAxis(int axis);
};

#define Vector4s0 Vector4s(0,0,0)
#define Vector4sX Vector4s(1,0,0)
#define Vector4sY Vector4s(0,1,0)
#define Vector4sZ Vector4s(0,0,1)


inline Vector2s operator+(const Vector2s& v1,const Vector2s& v2)
{
	return Vector2s(v1.x + v2.x,  v1.y+v2.y);
}


inline Vector2s operator-(const Vector2s& v1,const Vector2s& v2)
{
	return Vector2s(v1.x - v2.x,  v1.y-v2.y);
}

inline Vector2s operator*(const Vector2s& v1,const Vector2s& v2)
{
	CHK_MULT(v1.x,v2.x);
	CHK_MULT(v1.y,v2.y);
	
	return Vector2s(v1.x * v2.x,  v1.y*v2.y);
}

inline Vector2s operator*(int x,const Vector2s& v2)
{
	CHK_MULT(x,v2.x);
	CHK_MULT(x,v2.y);
	
	return Vector2s(x * v2.x,  x*v2.y);
}

inline Vector2s operator*(const Vector2s& v2,int x)
{
	return operator*(x,v2);
}

inline Vector2s operator/(const Vector2s& v2,int x)
{
	return Vector2s(v2.x/x , v2.y/x);
}

inline Vector2s operator >> (Vector2s const & in_V, int in_I)
{
	return Vector2s(in_V.x >> in_I, in_V.y >> in_I);
}

inline Vector2s operator << (Vector2s const & in_V, int in_I)
{
	return Vector2s(in_V.x << in_I, in_V.y << in_I);
}

inline Vector4s operator+(const Vector4s& v1,const Vector4s& v2)
{
	return Vector4s(v1.x + v2.x,  v1.y+v2.y, v1.z+v2.z);
}


inline Vector4s operator-(const Vector4s& v1,const Vector4s& v2)
{
	return Vector4s(v1.x - v2.x,  v1.y-v2.y, v1.z-v2.z);
}

inline Vector4s operator*(const Vector4s& v1,const Vector4s& v2)
{
	CHK_MULT(v1.x,v2.x);
	CHK_MULT(v1.y,v2.y);
	CHK_MULT(v1.z,v2.z);
	
	return Vector4s(v1.x * v2.x,  v1.y*v2.y, v1.z*v2.z);
}

inline Vector4s operator*(int x,const Vector4s& v2)
{
	CHK_MULT(x,v2.x);
	CHK_MULT(x,v2.y);
	CHK_MULT(x,v2.z);
	
	return Vector4s(x * v2.x,  x*v2.y, x*v2.z);
}

inline Vector4s operator*(const Vector4s& v2,int x)
{
	return operator*(x,v2);
}

inline Vector4s operator/(const Vector4s& v2,int x)
{
	return Vector4s(v2.x/x , v2.y/x ,v2.z/x);
}

inline Vector4s operator >> (Vector4s const & in_V, int in_I)
{
	return Vector4s(in_V.x >> in_I, in_V.y >> in_I, in_V.z >> in_I);
}

inline Vector4s operator << (Vector4s const & in_V, int in_I)
{
	return Vector4s(in_V.x << in_I, in_V.y << in_I, in_V.z << in_I);
}


// 32 bits vector required when scaled values must be stored
class Vector3i
{
public:
	typedef Vector4s::Type Type;

	Type x;
	Type y;
	Type z;

	inline void Init(int x2, int y2, int z2) { x = x2; y = y2; z = z2; };
	inline void Init(const Vector3i *v2) { x = v2->x; y = v2->y; z = v2->z; };
	inline void Init(const Vector3i& v2) { x = v2.x; y = v2.y; z = v2.z; };
	inline void Init(const Vector4s *v2) { x = v2->x; y = v2->y; z = v2->z; };
	inline void Add(int x2, int y2, int z2) { x += x2; y += y2; z += z2; };
	inline void Add(const Vector3i *v2) { x += v2->x; y += v2->y; z += v2->z; };
	inline void Add(const Vector4s *v2) { x += v2->x; y += v2->y; z += v2->z; };
	inline void Sub(int x2, int y2, int z2) { x -= x2; y -= y2; z -= z2; };
	inline void Sub(const Vector3i *v2) { x -= v2->x; y -= v2->y; z -= v2->z; };
	inline void Sub(const Vector4s *v2) { x -= v2->x; y -= v2->y; z -= v2->z; };
	inline void GetDelta(const Vector3i *A, const Vector3i *B) { x = A->x - B->x; y = A->y - B->y; z = A->z - B->z; };
	inline void GetDelta(const Vector4s *A, const Vector4s *B) { x = A->x - B->x; y = A->y - B->y; z = A->z - B->z; };

	void Normalize();
};



inline bool	FrontSide(const Vector4s& tA,const Vector4s& normal,const Vector4s& point)
{
	const int dot = ((	point.x - tA.x) * normal.x) + 
					((	point.y - tA.y) * normal.y) + 
					((	point.z - tA.z) * normal.z);
	
	#if WIN_DEBUG		
		const int _dot = Vector4s::Dot(point - tA, normal);
		A_ASSERT(dot==_dot);
	#endif

	return dot >=0;
}

bool	RayPlaneInter(const Vector4s& tA,const Vector4s& normal,const Vector4s& rO,const Vector4s& rV,Vector4s& o_point);
bool	RaytriangleIntersect(	const Vector4s& tA,const Vector4s& tB,const Vector4s& tC,const Vector4s& normal,const Vector4s& rO,const Vector4s& rV,bool doubleSided);
int		FindRaytriangleIntersectionPoint(	const Vector4s& tA,const Vector4s& tB,const Vector4s& tC,const Vector4s& normal,const Vector4s& rO,Vector4s& dst,bool doubleSided);

int		PointRayDist2(const Vector4s& rayPoint,const Vector4s& rayVector,const Vector4s& point);

#ifdef USE_OGL

	typedef struct
	{
		f32 x;
		f32 y;
		f32 z;
	} Vector3f;

#endif /* USE_OGL */

#endif // _VECTOR_H_
