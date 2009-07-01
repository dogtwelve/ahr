#include "matrix.h"
#include "config.h"

#include "DevUtil.h"

#include <stdlib.h>
#include <limits.h>

using namespace Lib3D;

#ifdef USE_OGL

f32 CMatrix44::s_matrixFloat[16] = {0.0f};

static const f32 k_InvCOS_SIN_MUL = 1.0f / COS_SIN_MUL;

void CMatrix44::GetMatrixFloat( f32 *mfloat) const
{
//		   (m[0]   m[4]   m[8]    m[12]  )	     ( v[0]	)
//		   |m[1]   m[5]    m[9]   m[13]  |	     | v[1]	|
//	M(v) = |m[2]   m[6]   m[10]   m[14]  |  x	 | v[2]	|
//		   (m[3]   m[7]   m[11]   m[15]  )	     ( v[3]	)
	//1st line
	mfloat[0] = (float)m[INDEX(0, 0)] * k_InvCOS_SIN_MUL;
	mfloat[4] = (float)m[INDEX(0, 1)] * k_InvCOS_SIN_MUL;
	mfloat[8] = (float)m[INDEX(0, 2)] * k_InvCOS_SIN_MUL;
	mfloat[12] = (float)m[INDEX(0, 3)];
	
	//2nd line
	mfloat[1] = (float)m[INDEX(1, 0)] * k_InvCOS_SIN_MUL;
	mfloat[5] = (float)m[INDEX(1, 1)] * k_InvCOS_SIN_MUL;
	mfloat[9] = (float)m[INDEX(1, 2)] * k_InvCOS_SIN_MUL;
	mfloat[13] = (float)m[INDEX(1, 3)];	

	//3rd line
	mfloat[2]  = (float)m[INDEX(2, 0)] * k_InvCOS_SIN_MUL;
	mfloat[6]  = (float)m[INDEX(2, 1)] * k_InvCOS_SIN_MUL;
	mfloat[10] = (float)m[INDEX(2, 2)] * k_InvCOS_SIN_MUL;
	mfloat[14] = (float)m[INDEX(2, 3)];	

	mfloat[3] = 0.0f;
	mfloat[7] = 0.0f;
	mfloat[11] = 0.0f;
	mfloat[15] = 1.0f;

	//mfloat[0] = this->mfloat[INDEX(0, 0)];
	//mfloat[4] = this->mfloat[INDEX(0, 1)];
	//mfloat[8] = this->mfloat[INDEX(0, 2)];
	//mfloat[12] = this->mfloat[INDEX(0, 3)];
	//
	////2nd line
	//mfloat[1] = this->mfloat[INDEX(1, 0)];
	//mfloat[5] = this->mfloat[INDEX(1, 1)];
	//mfloat[9] = this->mfloat[INDEX(1, 2)];
	//mfloat[13] = this->mfloat[INDEX(1, 3)];	

	////3rd line
	//mfloat[2]  = this->mfloat[INDEX(2, 0)];
	//mfloat[6]  = this->mfloat[INDEX(2, 1)];
	//mfloat[10] = this->mfloat[INDEX(2, 2)];
	//mfloat[14] = this->mfloat[INDEX(2, 3)];	

}

//void CMatrix44::LoadIdentityf(void)
//{
//	mfloat[INDEX(0,0)] = 1.0f;		 mfloat[INDEX(0,1)] = 0;            mfloat[INDEX(0,2)] = 0;             mfloat[INDEX(0,3)] = 0;
//	mfloat[INDEX(1,0)] = 0;          mfloat[INDEX(1,1)] = 1.0f;			mfloat[INDEX(1,2)] = 0;             mfloat[INDEX(1,3)] = 0;
//	mfloat[INDEX(2,0)] = 0;          mfloat[INDEX(2,1)] = 0;            mfloat[INDEX(2,2)] = 1.0f;			mfloat[INDEX(2,3)] = 0;
//}
//
//void CMatrix44::DefTranslatef(Type x, Type y, Type z)
//{
//	LoadIdentityf();
//
//	mfloat[INDEX(0,3)] = (f32)x;
//	mfloat[INDEX(1,3)] = (f32)y;
//	mfloat[INDEX(2,3)] = (f32)z;
//}
//
//void CMatrix44::Translatef(Type x, Type y, Type z)
//{
//	f32 A, B, C;
//
//	A = mfloat[INDEX(0,0)];
//	B = mfloat[INDEX(0,1)];
//	C = mfloat[INDEX(0,2)];
//
//	mfloat[INDEX(0,3)] += ( A*x + B*y + C*z );
//
//	A = mfloat[INDEX(1,0)];
//	B = mfloat[INDEX(1,1)];
//	C = mfloat[INDEX(1,2)];
//
//	mfloat[INDEX(1,3)] += ( A*x + B*y + C*z );
//
//	A = mfloat[INDEX(2,0)];
//	B = mfloat[INDEX(2,1)];
//	C = mfloat[INDEX(2,2)];
//
//
//	mfloat[INDEX(2,3)] += ( A*x + B*y + C*z );
//}
//
//void CMatrix44::DefRotateXf(int a)
//{
//	f32 s, c;
//	s = (f32) Sinus(a) * k_InvCOS_SIN_MUL;
//	c = (f32) Cosinus(a) * k_InvCOS_SIN_MUL;
//
//	mfloat[INDEX(0,0)] = 1.0f;
//	mfloat[INDEX(0,1)] = 0;
//	mfloat[INDEX(0,2)] = 0;
//
//	mfloat[INDEX(1,0)] = 0;
//	mfloat[INDEX(1,1)] = c;  
//	mfloat[INDEX(1,2)] = - s;
//
//	mfloat[INDEX(2,0)] = 0;
//	mfloat[INDEX(2,1)] = s;  
//	mfloat[INDEX(2,2)] = c;
//
//	mfloat[INDEX(0,3)] = 0;  
//	mfloat[INDEX(1,3)] = 0;  
//	mfloat[INDEX(2,3)] = 0;  
//}
//
//void CMatrix44::DefRotateYf(int a)
//{
//	f32 s, c;
//	s = (f32) Sinus(a) * k_InvCOS_SIN_MUL;
//	c = (f32) Cosinus(a) * k_InvCOS_SIN_MUL;
//
//	mfloat[INDEX(0,0)] = c;
//	mfloat[INDEX(0,1)] = 0;  
//	mfloat[INDEX(0,2)] = s;
//
//	mfloat[INDEX(1,0)] = 0;
//	mfloat[INDEX(1,1)] = 1.0f;  
//	mfloat[INDEX(1,2)] = 0;
//
//	mfloat[INDEX(2,0)] = -s; 
//	mfloat[INDEX(2,1)] = 0;  
//	mfloat[INDEX(2,2)] = c;
//
//	mfloat[INDEX(0,3)] = 0;  
//	mfloat[INDEX(1,3)] = 0;  
//	mfloat[INDEX(2,3)] = 0;  
//}
//
//void CMatrix44::DefRotateZf(int a)
//{
//	f32 s, c;
//	s = (f32) Sinus(a) * k_InvCOS_SIN_MUL;
//	c = (f32) Cosinus(a) * k_InvCOS_SIN_MUL;
//
//	mfloat[INDEX(0,0)] = c;  
//	mfloat[INDEX(0,1)] = -s;
//	mfloat[INDEX(0,2)] = 0;
//
//	mfloat[INDEX(1,0)] = s;  
//	mfloat[INDEX(1,1)] = c;
//	mfloat[INDEX(1,2)] = 0;
//
//	mfloat[INDEX(2,0)] = 0;
//	mfloat[INDEX(2,1)] = 0;  
//	mfloat[INDEX(2,2)] = 1.0f;  
//
//	mfloat[INDEX(0,3)] = 0;  
//	mfloat[INDEX(1,3)] = 0;  
//	mfloat[INDEX(2,3)] = 0; 
//}
//
//void CMatrix44::RotateXf(int a)
//{
//	CMatrix44 rx;
//
//	rx.DefRotateXf(a);
//	rx.mfloat[INDEX(0,3)] = 0;
//	rx.mfloat[INDEX(1,3)] = 0;
//	rx.mfloat[INDEX(2,3)] = 0;
//	Multf(&rx);
//}
//
//void CMatrix44::RotateYf(int a)
//{
//	CMatrix44 ry;
//
//	ry.DefRotateYf(a);
//	ry.mfloat[INDEX(0,3)] = 0;
//	ry.mfloat[INDEX(1,3)] = 0;
//	ry.mfloat[INDEX(2,3)] = 0;
//	Multf(&ry);
//}
//
//void CMatrix44::RotateZf(int a)
//{
//	CMatrix44 rz;
//
//	rz.DefRotateZf(a);
//	rz.mfloat[INDEX(0,3)] = 0;
//	rz.mfloat[INDEX(1,3)] = 0;
//	rz.mfloat[INDEX(2,3)] = 0;
//	Multf(&rz);
//}
//
//void CMatrix44::Multf(const CMatrix44 *M2)
//{
//	f32 A, B, C;
//
//	A = mfloat[INDEX(0,0)];
//	B = mfloat[INDEX(0,1)];
//	C = mfloat[INDEX(0,2)];
//
//	mfloat[INDEX(0,0)] = (( (A*M2->mfloat[INDEX(0,0)] + B*M2->mfloat[INDEX(1,0)]) + C*M2->mfloat[INDEX(2,0)] ) );
//	mfloat[INDEX(0,1)] = (( (A*M2->mfloat[INDEX(0,1)] + B*M2->mfloat[INDEX(1,1)]) + C*M2->mfloat[INDEX(2,1)] ) );
//	mfloat[INDEX(0,2)] = (( (A*M2->mfloat[INDEX(0,2)] + B*M2->mfloat[INDEX(1,2)]) + C*M2->mfloat[INDEX(2,2)] ) );
//	mfloat[INDEX(0,3)] = (( (A*M2->mfloat[INDEX(0,3)] + B*M2->mfloat[INDEX(1,3)]) + C*M2->mfloat[INDEX(2,3)] ) ) + mfloat[INDEX(0,3)];
//
//	A = mfloat[INDEX(1,0)];
//	B = mfloat[INDEX(1,1)];
//	C = mfloat[INDEX(1,2)];
//
//	mfloat[INDEX(1,0)] = (( (A*M2->mfloat[INDEX(0,0)] + B*M2->mfloat[INDEX(1,0)]) + C*M2->mfloat[INDEX(2,0)] ) );
//	mfloat[INDEX(1,1)] = (( (A*M2->mfloat[INDEX(0,1)] + B*M2->mfloat[INDEX(1,1)]) + C*M2->mfloat[INDEX(2,1)] ) );
//	mfloat[INDEX(1,2)] = (( (A*M2->mfloat[INDEX(0,2)] + B*M2->mfloat[INDEX(1,2)]) + C*M2->mfloat[INDEX(2,2)] ) );
//	mfloat[INDEX(1,3)] = (( (A*M2->mfloat[INDEX(0,3)] + B*M2->mfloat[INDEX(1,3)]) + C*M2->mfloat[INDEX(2,3)] )) + mfloat[INDEX(1,3)];
//
//	A = mfloat[INDEX(2,0)];
//	B = mfloat[INDEX(2,1)];
//	C = mfloat[INDEX(2,2)];
//
//	mfloat[INDEX(2,0)] = (( (A*M2->mfloat[INDEX(0,0)] + B*M2->mfloat[INDEX(1,0)]) + C*M2->mfloat[INDEX(2,0)] ) );
//	mfloat[INDEX(2,1)] = (( (A*M2->mfloat[INDEX(0,1)] + B*M2->mfloat[INDEX(1,1)]) + C*M2->mfloat[INDEX(2,1)] ) );
//	mfloat[INDEX(2,2)] = (( (A*M2->mfloat[INDEX(0,2)] + B*M2->mfloat[INDEX(1,2)]) + C*M2->mfloat[INDEX(2,2)] ) );
//	mfloat[INDEX(2,3)] = (( (A*M2->mfloat[INDEX(0,3)] + B*M2->mfloat[INDEX(1,3)]) + C*M2->mfloat[INDEX(2,3)] ) ) + mfloat[INDEX(2,3)];
//}

#endif/* USE_OGL */

void CMatrix44::Load(const CMatrix44 *Sm)
{
	STATIC_ASSERT(HALF_PRECISION == (COS_SIN_MUL >> 1));

	m[0] = Sm->m[0];
	m[1] = Sm->m[1];
	m[2] = Sm->m[2];
	m[3] = Sm->m[3];
	m[4] = Sm->m[4];
	m[5] = Sm->m[5];
	m[6] = Sm->m[6];
	m[7] = Sm->m[7];
	m[8] = Sm->m[8];
	m[9] = Sm->m[9];
	m[10] = Sm->m[10];
	m[11] = Sm->m[11];
}

// init identity matrix
void CMatrix44::LoadIdentity(void)
{
	m[INDEX(0,0)] = COS_SIN_MUL;  m[INDEX(0,1)] = 0;            m[INDEX(0,2)] = 0;            m[INDEX(0,3)] = 0;
	m[INDEX(1,0)] = 0;            m[INDEX(1,1)] = COS_SIN_MUL;  m[INDEX(1,2)] = 0;            m[INDEX(1,3)] = 0;
	m[INDEX(2,0)] = 0;            m[INDEX(2,1)] = 0;            m[INDEX(2,2)] = COS_SIN_MUL;  m[INDEX(2,3)] = 0;
}

void CMatrix44::Mult(const CMatrix44 *M2)
{
	int A, B, C;

	A = m[INDEX(0,0)];
	B = m[INDEX(0,1)];
	C = m[INDEX(0,2)];

	m[INDEX(0,0)] = (( (A*M2->m[INDEX(0,0)] + B*M2->m[INDEX(1,0)]) + C*M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,1)] = (( (A*M2->m[INDEX(0,1)] + B*M2->m[INDEX(1,1)]) + C*M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,2)] = (( (A*M2->m[INDEX(0,2)] + B*M2->m[INDEX(1,2)]) + C*M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,3)] = (( (A*M2->m[INDEX(0,3)] + B*M2->m[INDEX(1,3)]) + C*M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + m[INDEX(0,3)];

	A = m[INDEX(1,0)];
	B = m[INDEX(1,1)];
	C = m[INDEX(1,2)];

	m[INDEX(1,0)] = (( (A*M2->m[INDEX(0,0)] + B*M2->m[INDEX(1,0)]) + C*M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,1)] = (( (A*M2->m[INDEX(0,1)] + B*M2->m[INDEX(1,1)]) + C*M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,2)] = (( (A*M2->m[INDEX(0,2)] + B*M2->m[INDEX(1,2)]) + C*M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,3)] = (( (A*M2->m[INDEX(0,3)] + B*M2->m[INDEX(1,3)]) + C*M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + m[INDEX(1,3)];

	A = m[INDEX(2,0)];
	B = m[INDEX(2,1)];
	C = m[INDEX(2,2)];

	m[INDEX(2,0)] = (( (A*M2->m[INDEX(0,0)] + B*M2->m[INDEX(1,0)]) + C*M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,1)] = (( (A*M2->m[INDEX(0,1)] + B*M2->m[INDEX(1,1)]) + C*M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,2)] = (( (A*M2->m[INDEX(0,2)] + B*M2->m[INDEX(1,2)]) + C*M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,3)] = (( (A*M2->m[INDEX(0,3)] + B*M2->m[INDEX(1,3)]) + C*M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + m[INDEX(2,3)];
}

void CMatrix44::MultFlipX(const CMatrix44 *M2)
{
	int A, B, C;

	A = m[INDEX(0,0)];
	B = m[INDEX(0,1)];
	C = m[INDEX(0,2)];

	m[INDEX(0,0)] = (( A*M2->m[INDEX(0,0)] - B*M2->m[INDEX(1,0)] - C*M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,1)] = (( -A*M2->m[INDEX(0,1)] + B*M2->m[INDEX(1,1)] + C*M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,2)] = (( -A*M2->m[INDEX(0,2)] + B*M2->m[INDEX(1,2)] + C*M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,3)] = (( -A*M2->m[INDEX(0,3)] + B*M2->m[INDEX(1,3)] + C*M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + m[INDEX(0,3)];

	A = m[INDEX(1,0)];
	B = m[INDEX(1,1)];
	C = m[INDEX(1,2)];

	m[INDEX(1,0)] = (( A*M2->m[INDEX(0,0)] - B*M2->m[INDEX(1,0)] - C*M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,1)] = (( -A*M2->m[INDEX(0,1)] + B*M2->m[INDEX(1,1)] + C*M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,2)] = (( -A*M2->m[INDEX(0,2)] + B*M2->m[INDEX(1,2)] + C*M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,3)] = (( -A*M2->m[INDEX(0,3)] + B*M2->m[INDEX(1,3)] + C*M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + m[INDEX(1,3)];

	A = m[INDEX(2,0)];
	B = m[INDEX(2,1)];
	C = m[INDEX(2,2)];

	m[INDEX(2,0)] = (( A*M2->m[INDEX(0,0)] - B*M2->m[INDEX(1,0)] - C*M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,1)] = (( -A*M2->m[INDEX(0,1)] + B*M2->m[INDEX(1,1)] + C*M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,2)] = (( -A*M2->m[INDEX(0,2)] + B*M2->m[INDEX(1,2)] + C*M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,3)] = (( -A*M2->m[INDEX(0,3)] + B*M2->m[INDEX(1,3)] + C*M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + m[INDEX(2,3)];
}


// matrix multiply
void CMatrix44::GetProduct(const CMatrix44 *M1, const CMatrix44 *M2)
{
	m[INDEX(0,0)] = ((((int)M1->m[INDEX(0,0)] * M2->m[INDEX(0,0)] + (int)M1->m[INDEX(0,1)] * M2->m[INDEX(1,0)]) + (int)M1->m[INDEX(0,2)] * M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,1)] = ((((int)M1->m[INDEX(0,0)] * M2->m[INDEX(0,1)] + (int)M1->m[INDEX(0,1)] * M2->m[INDEX(1,1)]) + (int)M1->m[INDEX(0,2)] * M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,2)] = ((((int)M1->m[INDEX(0,0)] * M2->m[INDEX(0,2)] + (int)M1->m[INDEX(0,1)] * M2->m[INDEX(1,2)]) + (int)M1->m[INDEX(0,2)] * M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(0,3)] = ((((int)M1->m[INDEX(0,0)] * M2->m[INDEX(0,3)] + (int)M1->m[INDEX(0,1)] * M2->m[INDEX(1,3)]) + (int)M1->m[INDEX(0,2)] * M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + M1->m[INDEX(0,3)];

	m[INDEX(1,0)] = ((((int)M1->m[INDEX(1,0)] * M2->m[INDEX(0,0)] + (int)M1->m[INDEX(1,1)] * M2->m[INDEX(1,0)]) + (int)M1->m[INDEX(1,2)] * M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,1)] = ((((int)M1->m[INDEX(1,0)] * M2->m[INDEX(0,1)] + (int)M1->m[INDEX(1,1)] * M2->m[INDEX(1,1)]) + (int)M1->m[INDEX(1,2)] * M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,2)] = ((((int)M1->m[INDEX(1,0)] * M2->m[INDEX(0,2)] + (int)M1->m[INDEX(1,1)] * M2->m[INDEX(1,2)]) + (int)M1->m[INDEX(1,2)] * M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(1,3)] = ((((int)M1->m[INDEX(1,0)] * M2->m[INDEX(0,3)] + (int)M1->m[INDEX(1,1)] * M2->m[INDEX(1,3)]) + (int)M1->m[INDEX(1,2)] * M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + M1->m[INDEX(1,3)];

	m[INDEX(2,0)] = ((((int)M1->m[INDEX(2,0)] * M2->m[INDEX(0,0)] + (int)M1->m[INDEX(2,1)] * M2->m[INDEX(1,0)]) + (int)M1->m[INDEX(2,2)] * M2->m[INDEX(2,0)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,1)] = ((((int)M1->m[INDEX(2,0)] * M2->m[INDEX(0,1)] + (int)M1->m[INDEX(2,1)] * M2->m[INDEX(1,1)]) + (int)M1->m[INDEX(2,2)] * M2->m[INDEX(2,1)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,2)] = ((((int)M1->m[INDEX(2,0)] * M2->m[INDEX(0,2)] + (int)M1->m[INDEX(2,1)] * M2->m[INDEX(1,2)]) + (int)M1->m[INDEX(2,2)] * M2->m[INDEX(2,2)] ) >> COS_SIN_SHIFT );
	m[INDEX(2,3)] = ((((int)M1->m[INDEX(2,0)] * M2->m[INDEX(0,3)] + (int)M1->m[INDEX(2,1)] * M2->m[INDEX(1,3)]) + (int)M1->m[INDEX(2,2)] * M2->m[INDEX(2,3)] ) >> COS_SIN_SHIFT ) + M1->m[INDEX(2,3)];
}

// Inverse rotate using transposed matrix
void CMatrix44::InvRotateVector(const Vector4s *Src, Vector4s *Dest)
{
	int x = Src->x;
	int y = Src->y;
	int z = Src->z;
	Dest->x = (x*m[INDEX(0,0)] + y*m[INDEX(1,0)] + z*m[INDEX(2,0)]) >> COS_SIN_SHIFT;
	Dest->y = (x*m[INDEX(0,1)] + y*m[INDEX(1,1)] + z*m[INDEX(2,1)]) >> COS_SIN_SHIFT;
	Dest->z = (x*m[INDEX(0,2)] + y*m[INDEX(1,2)] + z*m[INDEX(2,2)]) >> COS_SIN_SHIFT;
}

void CMatrix44::DefScale(int Scale)
{
	LoadIdentity();
	m[INDEX(0,0)] = Scale;  
	m[INDEX(1,1)] = Scale;  
	m[INDEX(2,2)] = Scale;  
}

// Ratio of 256 mean scale of 1.0f
// Ratio of 512 mean scale of 2.0f
void CMatrix44::Scale(int Ratio)
{
	CMatrix44 s;

	s.DefScale((COS_SIN_MUL*Ratio)>>8);
	Mult(&s);
}

void CMatrix44::DefTranslate(Type x, Type y, Type z)
{
	LoadIdentity();

	m[INDEX(0,3)] = x;
	m[INDEX(1,3)] = y;
	m[INDEX(2,3)] = z;
}

void CMatrix44::Translate(Type x, Type y, Type z)
{
	#ifdef WIN_DEBUG
		CMatrix44 m1 = *this;
	#endif
	int A, B, C;

	A = m[INDEX(0,0)];
	B = m[INDEX(0,1)];
	C = m[INDEX(0,2)];

	m[INDEX(0,3)] += ( A*x + B*y + C*z ) >> COS_SIN_SHIFT;

	A = m[INDEX(1,0)];
	B = m[INDEX(1,1)];
	C = m[INDEX(1,2)];


	m[INDEX(1,3)] += ( A*x + B*y + C*z ) >> COS_SIN_SHIFT;

	A = m[INDEX(2,0)];
	B = m[INDEX(2,1)];
	C = m[INDEX(2,2)];


	m[INDEX(2,3)] += ( A*x + B*y + C*z ) >> COS_SIN_SHIFT;

#ifdef WIN_DEBUG	
	CMatrix44 t;
	t.LoadIdentity();
	t.m[INDEX(0,3)] = x;
	t.m[INDEX(1,3)] = y;
	t.m[INDEX(2,3)] = z;
	m1.Mult(&t);
	A_ASSERT(*this == m1);
#endif
}



void CMatrix44::DefRotateX(int a)
{
	Type s, c;
	s = Sinus(a);
	c = Cosinus(a);

	m[INDEX(0,0)] = COS_SIN_MUL;
	m[INDEX(0,1)] = 0;
	m[INDEX(0,2)] = 0;

	m[INDEX(1,0)] = 0;
	m[INDEX(1,1)] = c;  
	m[INDEX(1,2)] = -s;

	m[INDEX(2,0)] = 0;
	m[INDEX(2,1)] = s;  
	m[INDEX(2,2)] = c;

	m[INDEX(0,3)] = 0;  
	m[INDEX(1,3)] = 0;  
	m[INDEX(2,3)] = 0;  
}

void CMatrix44::DefRotateY(int a)
{
	Type s, c;
	s = Sinus(a);
	c = Cosinus(a);

	m[INDEX(0,0)] = c;
	m[INDEX(0,1)] = 0;  
	m[INDEX(0,2)] = s;

	m[INDEX(1,0)] = 0;
	m[INDEX(1,1)] = COS_SIN_MUL;  
	m[INDEX(1,2)] = 0;

	m[INDEX(2,0)] = -s; 
	m[INDEX(2,1)] = 0;  
	m[INDEX(2,2)] = c;

	m[INDEX(0,3)] = 0;  
	m[INDEX(1,3)] = 0;  
	m[INDEX(2,3)] = 0;  
}

void CMatrix44::DefRotateZ(int a)
{
	Type s, c;
	s = (int)Sinus(a);
	c = (int)Cosinus(a);

	m[INDEX(0,0)] = c;  
	m[INDEX(0,1)] = -s;
	m[INDEX(0,2)] = 0;

	m[INDEX(1,0)] = s;  
	m[INDEX(1,1)] = c;
	m[INDEX(1,2)] = 0;

	m[INDEX(2,0)] = 0;
	m[INDEX(2,1)] = 0;  
	m[INDEX(2,2)] = COS_SIN_MUL;  

	m[INDEX(0,3)] = 0;  
	m[INDEX(1,3)] = 0;  
	m[INDEX(2,3)] = 0;  
}

int Component(int v0, int v1, int v2, int c, int s)
{
	const int shift = COS_SIN_SHIFT;
	const int one = COS_SIN_MUL;

	return ((((v0*v1)>>shift)*(one-c))>>shift) + ((v2*s)>>shift);
}


/*
void CMatrix44::DefRotateV(const Vector4s& v)
{
	Type s, c;
	s = Sinus(v.s);
	c = Cosinus(v.s);

	const int shift = COS_SIN_SHIFT;
	const int one = COS_SIN_MUL;

	Type xx = (v.x*v.x)>>shift;
	Type yy = (v.y*v.y)>>shift;
	Type zz = (v.z*v.z)>>shift;


	m[INDEX(0,0)] = xx + ((c*(one-xx))>>shift);
	m[INDEX(0,1)] = Component(v.x, v.y, v.z, c,s);
	m[INDEX(0,2)] = Component(v.x, v.z, -v.y, c,s);

	m[INDEX(1,0)] = Component(v.x, v.y, -v.z, c,s);;
	m[INDEX(1,1)] = yy + ( (c*(one-yy)) >> shift );  
	m[INDEX(1,2)] = Component(v.y, v.z, v.x, c,s);;

	m[INDEX(2,0)] = Component(v.x, v.z, v.y, c,s);;
	m[INDEX(2,1)] = Component(v.y, v.z, -v.x, c,s);;  
	m[INDEX(2,2)] = zz + ( (c*(one-zz)) >> shift );

	m[INDEX(0,3)] = 0;  
	m[INDEX(1,3)] = 0;  
	m[INDEX(2,3)] = 0;  
}
*/


void CMatrix44::RotateX(int a)
{
	CMatrix44 rx;

	rx.DefRotateX(a);
	rx.m[INDEX(0,3)] = 0;
	rx.m[INDEX(1,3)] = 0;
	rx.m[INDEX(2,3)] = 0;
	Mult(&rx);
}

void CMatrix44::RotateY(int a)
{
	CMatrix44 ry;

	ry.DefRotateY(a);
	ry.m[INDEX(0,3)] = 0;
	ry.m[INDEX(1,3)] = 0;
	ry.m[INDEX(2,3)] = 0;
	Mult(&ry);
}

void CMatrix44::RotateZ(int a)
{
	CMatrix44 rz;

	rz.DefRotateZ(a);
	rz.m[INDEX(0,3)] = 0;
	rz.m[INDEX(1,3)] = 0;
	rz.m[INDEX(2,3)] = 0;
	Mult(&rz);
}

void CMatrix44::ScaleX(int a)
{
	CMatrix44 rx;

	rx.m[INDEX(0,0)] *= a;

	Mult(&rx);
}

void CMatrix44::ScaleY(int a)
{
	CMatrix44 rx;

	rx.m[INDEX(1,1)] *= a;

	Mult(&rx);
}

void CMatrix44::ScaleZ(int a)
{
	CMatrix44 rx;

	rx.m[INDEX(2,2)] *= a;

	Mult(&rx);
}

void CMatrix44::Transpose()
{
	int		t;
	
	t = m[INDEX(0,1)];  
	m[INDEX(0,1)] = m[INDEX(1,0)];
	m[INDEX(1,0)] = t;

	t = m[INDEX(0,2)];  
	m[INDEX(0,2)] = m[INDEX(2,0)];
	m[INDEX(2,0)] = t;

	t = m[INDEX(1,2)];  
	m[INDEX(1,2)] = m[INDEX(2,1)];
	m[INDEX(2,1)] = t;
}


void CMatrix44::ProjectToXZ()
{
	m[INDEX(1,0)] = m[INDEX(1,1)] = m[INDEX(1,2)] = 0;
}

void CMatrix44::ProjectToXZAvoidZeroScale()
{
	m[INDEX(1,0)] = m[INDEX(1,1)] = m[INDEX(1,2)] = 0;

	//used for shadow ... 
	m[INDEX(1,1)] = 10; // 1 / k_InvCOS_SIN_MUL
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool CMatrix44::operator==(const CMatrix44& m1) const
{
	for(int i=0;i<sizeof(m)/sizeof(m[0]);++i)
		if(m[i] != m1.m[i])
			return false;
	return true;
}



// -------------------------------------------------
// matrix stack, used by rendered
CMatrixStack::CMatrixStack(int size)
	:m_maxStackSize(size)
{
	m_stack = NEW CMatrix44[m_maxStackSize];

	m_currentMatrix = m_stack;
	m_currentMatrix->LoadIdentity();
} 



CMatrixStack::~CMatrixStack()
{
	A_ASSERT(m_currentMatrix == m_stack);
	DELETE_ARRAY m_stack;
}



CMatrix44& CMatrixStack::PushMatrix(void)
{
	A_ASSERT(m_currentMatrix -m_stack <= m_maxStackSize);
 
	m_currentMatrix++;
	m_currentMatrix->Load(m_currentMatrix - 1);

	return *m_currentMatrix;
}


void CMatrixStack::PopMatrix(void)
{
	A_ASSERT(m_currentMatrix-m_stack > 0);
	m_currentMatrix--;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CMatrix44& CMatrixStack::ResetStack()
{
	A_ASSERT(m_currentMatrix == m_stack);

	m_currentMatrix = m_stack;
	m_currentMatrix->LoadIdentity();
	return *m_currentMatrix;
}

