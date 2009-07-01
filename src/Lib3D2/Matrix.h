#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "IMath.h"
#include "Vector.h"
#include "GenDef.h"



namespace Lib3D
{

// Matrix calculations using integers

class CMatrix44
{
	inline static int INDEX(int i,int j) {return (i + j*3);} // cannot be changed due to union with Vector4s vectors

public:
	typedef Vector4s::Type Type;

	void Load(const CMatrix44 *Sm);
	void Mult(const CMatrix44 *n);
	void MultFlipX(const CMatrix44 *n);
	void LoadIdentity(void);
	void DefScale(int Scale);
	void Scale(int Ratio);
	void DefTranslate(Type x, Type y, Type z);
	void Translate(Type x, Type y, Type z);
inline	void Translate(const Vector4s *v){Translate(v->x,v->y,v->z);}
inline	void Translate(const Vector4s& v){Translate(v.x,v.y,v.z);}
	void Transpose();

	void ProjectToXZ();
	void ProjectToXZAvoidZeroScale();

	void DefRotateX(int a);
	void DefRotateY(int a);
	void DefRotateZ(int a);
	//void DefRotateV(const Vector4s& v);
	void RotateX(int a);
	void RotateY(int a);
	void RotateZ(int a);

	void ScaleX(int a);
	void ScaleY(int a);
	void ScaleZ(int a);

	void GetProduct(const CMatrix44 *M1, const CMatrix44 *M2);

	inline void TransformVector(const Vector4s *Src, Vector4s *Dest)const
	{		
		Dest->x = (((Src->x*m[INDEX(0,0)] + Src->y*m[INDEX(0,1)]) + Src->z*m[INDEX(0,2)] + HALF_PRECISION) >> COS_SIN_SHIFT) + m[INDEX(0,3)];
		Dest->y = (((Src->x*m[INDEX(1,0)] + Src->y*m[INDEX(1,1)]) + Src->z*m[INDEX(1,2)] + HALF_PRECISION) >> COS_SIN_SHIFT) + m[INDEX(1,3)];
		Dest->z = (((Src->x*m[INDEX(2,0)] + Src->y*m[INDEX(2,1)]) + Src->z*m[INDEX(2,2)] + HALF_PRECISION) >> COS_SIN_SHIFT) + m[INDEX(2,3)];
	}

	inline void RotateVector(const Vector4s *Src, Vector4s *Dest)const
	{
		const int x = Src->x;
		const int y = Src->y;
		const int z = Src->z;
		Dest->x = (x*m[INDEX(0,0)] + y*m[INDEX(0,1)] + z*m[INDEX(0,2)]) >> COS_SIN_SHIFT;
		Dest->y = (x*m[INDEX(1,0)] + y*m[INDEX(1,1)] + z*m[INDEX(1,2)]) >> COS_SIN_SHIFT;
		Dest->z = (x*m[INDEX(2,0)] + y*m[INDEX(2,1)] + z*m[INDEX(2,2)]) >> COS_SIN_SHIFT;
	}
	inline void RotateVectorx(const Vector4s *Src, Vector4s *Dest)const	{Dest->x = (Src->x*m[INDEX(0,0)] + Src->y*m[INDEX(0,1)] + Src->z*m[INDEX(0,2)]) >> COS_SIN_SHIFT;}
	inline void RotateVectory(const Vector4s *Src, Vector4s *Dest)const	{Dest->y = (Src->x*m[INDEX(1,0)] + Src->y*m[INDEX(1,1)] + Src->z*m[INDEX(1,2)]) >> COS_SIN_SHIFT;}
	inline void RotateVectorz(const Vector4s *Src, Vector4s *Dest)const	{Dest->z = (Src->x*m[INDEX(2,0)] + Src->y*m[INDEX(2,1)] + Src->z*m[INDEX(2,2)]) >> COS_SIN_SHIFT;}

	void InvRotateVector(const Vector4s *Src, Vector4s *Dest);


	inline int TransformVectorz(const Vector4s *Src) const
	{
		CHK_MULT(Src->x,m[INDEX(2,0)]);
		CHK_MULT(Src->y,m[INDEX(2,1)]);
		CHK_MULT(Src->z,m[INDEX(2,2)]);

		return (((Src->x*m[INDEX(2,0)] + Src->y*m[INDEX(2,1)] + Src->z*m[INDEX(2,2)]) >> COS_SIN_SHIFT) + m[INDEX(2,3)]);
	}

		
	inline int TransformVectorx(const Vector4s *Src) const
	{
		CHK_MULT(Src->x,m[INDEX(0,0)]);
		CHK_MULT(Src->y,m[INDEX(0,1)]);
		CHK_MULT(Src->z,m[INDEX(0,2)]);

		return ((((int)Src->x*m[INDEX(0,0)] + (int)Src->y*m[INDEX(0,1)] + (int)Src->z*m[INDEX(0,2)]) >> COS_SIN_SHIFT) + m[INDEX(0,3)]);
	}

		// transform x & y only
	inline int TransformVectory(const Vector4s *Src) const
	{
		CHK_MULT(Src->x,m[INDEX(1,0)]);
		CHK_MULT(Src->y,m[INDEX(1,1)]);
		CHK_MULT(Src->z,m[INDEX(1,2)]);

		
		return ((((int)Src->x*m[INDEX(1,0)] + (int)Src->y*m[INDEX(1,1)] + (int)Src->z*m[INDEX(1,2)]) >> COS_SIN_SHIFT) + m[INDEX(1,3)]);
	}

	// inline functions very frequently used
	// transform z only
	inline void TransformVectorz(const Vector4s *Src, Vector4s *Dest) const
	{		
		Dest->z = TransformVectorz(Src);
	}
	// transform x & y only
	inline void TransformVectorxy(const Vector4s *Src, Vector4s *Dest) const
	{
		Dest->x = TransformVectorx(Src);
		Dest->y = TransformVectory(Src);
	}

inline	Vector4s &Pos(void)   { return *(Vector4s *)&m[9]; };  // allow to access to position datas
inline	Vector4s &Dir(void)   { return *(Vector4s *)&m[6]; };  // dir  = z vector
inline	Vector4s &Up(void)    { return *(Vector4s *)&m[3]; };  // up   = y vector
inline	Vector4s &Right(void) { return *(Vector4s *)&m[0]; };  // left = x vector (if look on -z)

inline	const Vector4s &Pos(void)   const { return *(Vector4s *)&m[9]; };  // allow to access to position datas
inline	const Vector4s &Dir(void)   const { return *(Vector4s *)&m[6]; };  // dir  = z vector
inline	const Vector4s &Up(void)    const { return *(Vector4s *)&m[3]; };  // up   = y vector
inline	const Vector4s &Right(void) const { return *(Vector4s *)&m[0]; };  // left = x vector (if look on -z)

inline Type	Get(int i,int j) const	{return m[ INDEX(i,j) ];}
inline Type&	Set(int i,int j)	{return m[ INDEX(i,j) ];}

bool operator==(const CMatrix44&) const;

#ifdef USE_OGL

public:
	static f32 s_matrixFloat[16];

	void GetMatrixFloat(f32 *mfloat) const;

	//void LoadIdentityf(void);
	//void Translatef(Type x, Type y, Type z);
	//void DefTranslatef(Type x, Type y, Type z);
	//void DefRotateXf(int a);
	//void DefRotateYf(int a);
	//void DefRotateZf(int a);
	//void RotateXf(int a);
	//void RotateYf(int a);
	//void RotateZf(int a);

	//void Multf(const CMatrix44 *n);

	//f32 mfloat[16];

#endif /* USE_OGL */

private:
	Type m[12];
};


//class CFastMatrix44 : public CMatrix44
//{
//	public:
//	Type coef_op[3];
//	CFastMatrix44(const CMatrix44& mtx_in)
//	{
//		for(int i = 0; i < 12; i++)
//			m[i] = mtx_in.m[i];
//
//		coef_op[0] = (m[INDEX(0,0)]*m[INDEX(0,1)]) >> COS_SIN_SHIFT;
//		coef_op[1] = (m[INDEX(1,0)]*m[INDEX(1,1)]) >> COS_SIN_SHIFT;
//		coef_op[2] = (m[INDEX(2,0)]*m[INDEX(2,1)]) >> COS_SIN_SHIFT;	
//	}
//	inline void TransformVector(const Vector4s *Src, Vector4s *Dest)const
//	{
//		Type src_xy = Src->x * Src->y;
//		Type src_x = Src->x << COS_SIN_SHIFT;
//		Type src_y = Src->y << COS_SIN_SHIFT;
//
//		Dest->x = (((((m[INDEX(0,1)] + src_x) * (m[INDEX(0,0)] + src_y)) >> COS_SIN_SHIFT) + (m[INDEX(0,2)] * Src->z) - coef_op[0]) >> COS_SIN_SHIFT) - src_xy + m[INDEX(0,3)];
//		Dest->y = (((((m[INDEX(1,1)] + src_x) * (m[INDEX(1,0)] + src_y)) >> COS_SIN_SHIFT) + (m[INDEX(1,2)] * Src->z) - coef_op[1]) >> COS_SIN_SHIFT) - src_xy + m[INDEX(1,3)];
//		Dest->z = (((((m[INDEX(2,1)] + src_x) * (m[INDEX(2,0)] + src_y)) >> COS_SIN_SHIFT) + (m[INDEX(2,2)] * Src->z) - coef_op[2]) >> COS_SIN_SHIFT) - src_xy + m[INDEX(2,3)];
//
//	}
//	inline int TransformVectorz(const Vector4s *Src) const
//	{
//		Type src_xy = Src->x * Src->y;
//		Type src_x = Src->x << COS_SIN_SHIFT;
//		Type src_y = Src->y << COS_SIN_SHIFT;
//		return (((((m[INDEX(2,1)] + (src_x<< COS_SIN_SHIFT)) * (m[INDEX(2,0)] +(src_y<< COS_SIN_SHIFT))) >> COS_SIN_SHIFT) + (m[INDEX(2,2)] * Src->z) - coef_op[2]) >> COS_SIN_SHIFT) - src_xy + m[INDEX(2,3)];
//	}
//
//
//	inline int TransformVectorx(const Vector4s *Src) const
//	{
//		Type src_xy = Src->x * Src->y;
//		Type src_x = Src->x << COS_SIN_SHIFT;
//		Type src_y = Src->y << COS_SIN_SHIFT;
//		return (((((m[INDEX(0,1)] + (src_x<< COS_SIN_SHIFT)) * (m[INDEX(0,0)] + (src_y<< COS_SIN_SHIFT))) >> COS_SIN_SHIFT) + (m[INDEX(0,2)] * Src->z) - coef_op[0]) >> COS_SIN_SHIFT) - src_xy + m[INDEX(0,3)];
//	}
//
//	// transform x & y only
//	inline int TransformVectory(const Vector4s *Src) const
//	{
//		Type src_xy = Src->x * Src->y;
//		Type src_x = Src->x << COS_SIN_SHIFT;
//		Type src_y = Src->y << COS_SIN_SHIFT;
//		return (((((m[INDEX(1,1)] + (src_x<< COS_SIN_SHIFT)) * (m[INDEX(1,0)] + (src_y<< COS_SIN_SHIFT))) >> COS_SIN_SHIFT) + (m[INDEX(1,2)] * Src->z) - coef_op[1]) >> COS_SIN_SHIFT) - src_xy + m[INDEX(1,3)];
//	}
//
//
//};

// ---------------------------------------------------------------------------
// matrix stack (like on opengl)
// ---------------------------------------------------------------------------
class CMatrixStack                  
{
public:
	CMatrixStack(int size = 8);
	~CMatrixStack();
  
	CMatrix44&				PushMatrix(void);
	void					PopMatrix(void);
	const CMatrix44&		CurrentMatrix()const{return *m_currentMatrix;}
	
	CMatrix44&				ResetStack();

private:
	CMatrix44*		m_stack;
	CMatrix44*		m_currentMatrix;
	const int		m_maxStackSize;
};  



}//namespace
#endif // _MATRIX_H_
