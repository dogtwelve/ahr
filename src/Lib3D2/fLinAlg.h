#ifndef _FLINALG_H_
#define _FLINALG_H_

#include "Vector.h"

#include "assert.h"
#include <math.h>


// ---------------------------------------------------------------------------
//	Floating Point Linear Algebra Classes
//
//
//	Note: Thoses class shound not be used on the n-gage, only in the map compiler
// ---------------------------------------------------------------------------
namespace Lib3D
{
	class CMatrix44;

	typedef double FType;

	inline FType Square(FType x) {return x*x;}



// ---------------------------------------------------------------------------
//	
// ---------------------------------------------------------------------------
class FVector3
{
	public:
		FVector3(){};
		FVector3(FType a,FType b,FType c){m_val[0]=a;m_val[1]=b;m_val[2]=c;}
		FVector3(const FType* vals){m_val[0]=vals[0];m_val[1]=vals[1];m_val[2]=vals[2];}



		const FType&	operator[](int i) const {return m_val[i];}
		FType&				operator[](int i)				{return m_val[i];}

		void operator+=(const FVector3& v){m_val[0]+= v.m_val[0];m_val[1]+= v.m_val[1];m_val[2]+= v.m_val[2];}
		void operator/=(FType x){assert(x!=0.0);m_val[0]/=x;m_val[1]/=x;m_val[2]/=x;}
		void operator*=(FType x){m_val[0]*=x;m_val[1]*=x;m_val[2]*=x;}


		FVector3	operator-(const FVector3& v) const {return FVector3(m_val[0]-v.m_val[0],m_val[1]-v.m_val[1],m_val[2]-v.m_val[2]);}


		FType Length2() const {return Square(m_val[0]) + Square(m_val[1]) + Square(m_val[2]);}
		FType Length() const {return ::sqrt(Length2());}

		Vector4s	CreateVector4s(int bias) const;


		

	private:
		FType	m_val[3];
};


FVector3	CrossProduct(const FVector3& v1,const FVector3& v2);


FType Dot(const FVector3&,const FVector3&);




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class FMatrix33
{
	public:
		FMatrix33&	LoadIdentity()	{m_vec[0] = FVector3(1,0,0);m_vec[1] = FVector3(0,1,0);m_vec[2] = FVector3(0,0,1);return *this;}
		FMatrix33&	LoadNUL()				{m_vec[0] = FVector3(0,0,0);m_vec[1] = FVector3(0,0,0);m_vec[2] = FVector3(0,0,0);return *this;}

		const FVector3& operator[](int i) const {assert(i>=0 && i<3);return m_vec[i];}
		FVector3&				operator[](int i)				{assert(i>=0 && i<3);return m_vec[i];}
		void						operator*=(FType x)			{	m_vec[0]*=x;m_vec[1]*=x;m_vec[2]*=x;}


		FMatrix33 GetEigenVectors()const;

		FMatrix33	Transpose()const;

		FType			SubDet(int i,int j) const;

		FType			Determinant() const;
		FMatrix33	Inverse() const;

		FVector3	operator*(const FVector3&) const;

		void			CreateCMatrix44(CMatrix44&)const;
		
	protected:
		FVector3	m_vec[3];
};



FVector3 operator*(const FVector3&,const FMatrix33 &);



// ---------------------------------------------------------------------------
//	Covariant Matrix used to find the aligned bounding box fron an array of points
//	ref:	Mathematics for 3D Game Programming anf Conputer Graphics, p 183+
// ---------------------------------------------------------------------------


template<class T>
FMatrix33 CreateCovariantMatrix(const T& vertexArray)
{
	if(vertexArray.size()>0)
	{
		int i,j;
		typename T::const_iterator v;
		FVector3 m(0,0,0);

		// mean vector
		for( v= vertexArray.begin();v!=vertexArray.end();++v)
			m += *v;

		m /= vertexArray.size();

		// FIll the covariance Matrix
		// Note that this matrix is symmetric

		FMatrix33 matrix;
		matrix.LoadNUL();
		for( v= vertexArray.begin();v!=vertexArray.end();++v)
		{
			const FVector3	dif = *v - m;

			for(i=0;i<3;++i)
			for(j=0;j<3;++j)
			{
				matrix[i][j] += (dif[i]*dif[j]);
			}
		}

		for(i=0;i<3;++i)
			matrix[i] /= vertexArray.size();

		return matrix;
	}
	else
		return FMatrix33().LoadIdentity();
}















		


}	// Namespace Lib3D

#endif // _FLINALG_H_
