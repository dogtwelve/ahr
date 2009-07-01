#include "flinalg.h"

#include "Matrix.h"
#include "IMath.h"

#include <math.h>

using namespace Lib3D;

template<class T> void			Swap(T& a,T& b)							{const T c = a;	a = b;	b = c;}
template<class T> inline T	SIGN(const T& a,const T& b)	{return	b >= 0.0 ? fabs(a) : -fabs(a);}
inline CMatrix44::Type			ToInt(FType x)							{return CMatrix44::Type(x);}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
FType Lib3D::Dot(const FVector3& v1,const FVector3& v2)
{
	return	v1[0] * v2[0] + 
					v1[1] * v2[1] + 
					v1[2] * v2[2];
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
FVector3	Lib3D::CrossProduct(const FVector3& v1,const FVector3& v2)
{
	return FVector3(	v1[1] * v2[2] - v1[2] * v2[1],
										v1[2] * v2[0] - v1[0] * v2[2],
										v1[0] * v2[1] - v1[1] * v2[0]);  
}

// ---------------------------------------------------------------------------
//	Computes (a2 + b2)1/2 without destructive underflow or overflow.
// ---------------------------------------------------------------------------
static FType pythag(FType a, FType b)
{
	
	FType absa=fabs(a);
	FType absb=fabs(b);

	if (absa > absb) 
		return absa*sqrt(1.0+Square(absb/absa));
	else 
		return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+Square(absa/absb)));
}





/*
// ---------------------------------------------------------------------------
	Code Taken from "Numerical Recipes"

Householder reduction of a real, symmetric matrix a[0..n-1][0..n-1]. 

On output, a is replaced by the orthogonal matrix Q effecting the transformation.

d[0..n-1] returns the diagonal elements of the tridiagonal matrix, 
e[0..n-1] return the off-diagonal elements, with e[1]=0.

Several statements, as noted in comments, can be omitted if only eigenvalues are to be found, in which
case a contains no useful information on output.
Otherwise they are to be included.

// ---------------------------------------------------------------------------
*/
static void tred2(FMatrix33& matrix, const int n, FVector3& d, FVector3& e)
{
	int k,j,i;
	FType scale,hh,h,g,f;
	
	for (i=n-1;i>=1;i--)
	{
		const int l=i-1;
		h=scale=0.0;
		
		if (l > 0)
		{
			for (k=0;k<=l;k++)
				scale += fabs(matrix[i][k]);
			
			if (scale == 0.0) //	Skip transformation.
				e[i]=matrix[i][l];
			else 
			{
				for (k=0;k<=l;k++) 
				{
					matrix[i][k] /= scale;				//	Use scaled a’s for transformation.
					h += matrix[i][k]*matrix[i][k]; //	Form ó in h.
				}
				f=matrix[i][l];
				
				g=(f >= 0.0 ? -sqrt(h) : sqrt(h));
				
				e[i]=scale*g;
				
				h -= f*g; //	Now h is equation (11.2.4).
				
				matrix[i][l]=f-g; //Store u in the ith row of matrix.
				
				f=0.0;
				for (j=0;j<=l;j++) 
				{
					// Next statement can be omitted if eigenvectors not wanted
					matrix[j][i]=matrix[i][j]/h; //Store u/H in ith column of matrix.
					g=0.0; //Form an element of A · u in g.
					for (k=0;k<=j;k++)
						g += matrix[j][k]*matrix[i][k];
					
					for (k=j+1;k<=l;k++)
						g += matrix[k][j]*matrix[i][k];
					
					e[j]=g/h; //Form element of p in temporarily unused element of e.
					
					f += e[j]*matrix[i][j];
				}
				hh=f/(h+h); //Form K, equation (11.2.11).
				for (j=0;j<=l;j++) 
				{		//Form q and store in e overwriting p.
					f=matrix[i][j];
					e[j]=g=e[j]-hh*f;
					
					for (k=0;k<=j;k++) //	Reduce matrix, equation (11.2.13).
						matrix[j][k] -= (f*e[k]+g*matrix[i][k]);
				}
			}
		}
		else
			e[i]=matrix[i][l];
		d[i]=h;
	}
	// Next statement can be omitted if eigenvectors not wanted 
	d[0]=0.0;
	e[0]=0.0;
	//	Contents of this loop can be omitted if eigenvectors not
	//	wanted except for statement d[i]=matrix[i][i]; 
	
	
	//Begin accumulation of transformation matrices.
	for (i=0;i<n;i++) 
	{ 
		const int l = i-1;

		if (d[i])		//This block skipped when i=1.
		{ 
			for (j=0;j<=l;j++) 
			{
				g=0.0;
				for (k=0;k<=l;k++) //Use u and u/H stored in matrix to form P·Q.
					g += matrix[i][k]*matrix[k][j];
				
				for (k=0;k<=l;k++)
					matrix[k][j] -= g*matrix[k][i];
			}
		}
		d[i]=matrix[i][i]; //This statement remains.
		matrix[i][i]=1.0; //Reset row and column of matrix to identitymatrix for next iteration. 
		
		for (j=0;j<=l;j++)
			matrix[j][i]=matrix[i][j]=0.0;
	}
}


/*
// ---------------------------------------------------------------------------
QL algorithm with implicit shifts, to determine the eigenvalues and eigenvectors of a real, symmetric,
tridiagonal matrix, or of a real, symmetric matrix previously reduced by tred2 §11.2. 
				
					
On input, d[1..n] contains the diagonal elements of the tridiagonal matrix. 
On output, it returns the eigenvalues. 

The vector e[1..n] inputs the subdiagonal elements of the tridiagonal matrix,
with e[1] arbitrary. 
On output e is destroyed. 

When finding only the eigenvalues, several lines may be omitted, as noted in the comments. 

If the eigenvectors of a tridiagonal matrix are desired, the matrix z[1..n][1..n] is input as 
the identity matrix. 

If the eigenvectors of a matrix that has been reduced by tred2 are required, then 
z is input as the matrix output by tred2.

In either case, the kth column of z returns the normalized eigenvector corresponding to d[k].
// ---------------------------------------------------------------------------
*/
static void tqli(FVector3& d, FVector3& e, int n, FMatrix33& z)
{
	int m,l,i,k;
	FType s,r,p,g,f,dd,c,b;
	
	//Convenient to renumber the elements of e. 
	for (i=1;i<n;i++)
		e[i-1]=e[i]; 

	e[n-1]=0.0;

	for (l=0;l<n;l++) 
	{
		int iter=0;
		do 
		{
			for(m=l;m < n-1;m++)
			{ //Look for a single small subdiagonal element to split the matrix.
				dd=fabs(d[m])+fabs(d[m+1]);

				if((fabs(e[m])+dd) == dd) 
					break;
			}
			if (m != l) 
			{
				iter++;
				if(iter >= 30) 
				{
					assert(false);	//
					return;
				}
				
				g=(d[l+1]-d[l])/(2.0*e[l]); //Form shift.
				r=pythag(g,1.0);
				g=d[m]-d[l]+e[l]/(g+SIGN(r,g)); //This is dm - ks.
				s=c=1.0;
				p=0.0;
				
				
				for (i=m-1;i>=l;i--)
				{ //A plane rotation as in the original QL, followed by Givens rotations to restore tridiagonal form.
					f=s*e[i];
					b=c*e[i];
					e[i+1]=(r=pythag(f,g));
					if (r == 0.0) 
					{ //Recover from underflow.
						d[i+1] -= p;
						e[m]=0.0;
						break;
					}
					s=f/r;
					c=g/r;
					g=d[i+1]-p;
					r=(d[i]-g)*s+2.0*c*b;
					d[i+1]=g+(p=s*r);
					g=c*r-b;
					// Next loop can be omitted if eigenvectors not wanted
					for (k=0;k<n;k++) 
					{ //Form eigenvectors.
						f	=z[k][i+1];
						z[k][i+1]=s*z[k][i]+c*f;
						z[k][i]=c*z[k][i]-s*f;
					}
				}
				if (r == 0.0 && i >= l) 
					continue;
				d[l] -= p;
				e[l]=g;
				e[m]=0.0;
			}
		}
		while (m != l);
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
FMatrix33 FMatrix33::GetEigenVectors() const
{
	FMatrix33 eigenVector = *this;

	eigenVector = *this;
	FVector3 d;
	FVector3 e;

	::tred2(eigenVector,3,d,e);
	::tqli(d, e, 3, eigenVector);

	return eigenVector;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
FMatrix33 FMatrix33::Transpose()const
{
	FMatrix33 t = *this;
	::Swap(t.m_vec[0][1],t.m_vec[1][0]);
	::Swap(t.m_vec[0][2],t.m_vec[2][0]);
	::Swap(t.m_vec[1][2],t.m_vec[2][1]);

	return t;
}







// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
FType FMatrix33::Determinant() const
{
	const FVector3* a = m_vec;

	return	(a[0][0] * ( (a[1][1] * a[2][2]) - (a[1][2] * a[2][1]) ))
				-	(a[0][1] * ( (a[1][0] * a[2][2]) - (a[1][2] * a[2][0]) ))
				+	(a[0][2] * ( (a[1][0] * a[2][1]) - (a[1][1] * a[2][0]) ));
}


// ---------------------------------------------------------------------------
// ref: Mathematics for 3D Game Programming and Conputer Grpahics, p. 41
// ---------------------------------------------------------------------------
FMatrix33 FMatrix33::Inverse() const 
{
	const FType det = Determinant();
	if(det == 0.0)
	{
		assert(0);
		return FMatrix33().LoadIdentity();
	}

	FMatrix33 inv;
	
  inv[0][0] = m_vec[1][1]*m_vec[2][2] - m_vec[1][2]*m_vec[2][1];
  inv[0][1] = m_vec[0][2]*m_vec[2][1] - m_vec[0][1]*m_vec[2][2];
  inv[0][2] = m_vec[0][1]*m_vec[1][2] - m_vec[0][2]*m_vec[1][1];
  inv[1][0] = m_vec[1][2]*m_vec[2][0] - m_vec[1][0]*m_vec[2][2];
  inv[1][1] = m_vec[0][0]*m_vec[2][2] - m_vec[0][2]*m_vec[2][0];
  inv[1][2] = m_vec[0][2]*m_vec[1][0] - m_vec[0][0]*m_vec[1][2];
  inv[2][0] = m_vec[1][0]*m_vec[2][1] - m_vec[1][1]*m_vec[2][0];
  inv[2][1] = m_vec[0][1]*m_vec[2][0] - m_vec[0][0]*m_vec[2][1];
  inv[2][2] = m_vec[0][0]*m_vec[1][1] - m_vec[0][1]*m_vec[1][0];

	inv *= 1.0/det;

	return inv;                          
}



// ---------------------------------------------------------------------------
// convert float matrix to integer scaled matrix
// ---------------------------------------------------------------------------
void FMatrix33::CreateCMatrix44(CMatrix44& m)const
{
  const FType TrigMul = FType(COS_SIN_MUL);

  m.Set(0,0) = ::ToInt(m_vec[0][0]*TrigMul);
  m.Set(1,0) = ::ToInt(m_vec[1][0]*TrigMul);
  m.Set(2,0) = ::ToInt(m_vec[2][0]*TrigMul);

  m.Set(0,1) = ::ToInt(m_vec[0][1]*TrigMul);
  m.Set(1,1) = ::ToInt(m_vec[1][1]*TrigMul);
  m.Set(2,1) = ::ToInt(m_vec[2][1]*TrigMul);

  m.Set(0,2) = ::ToInt(m_vec[0][2]*TrigMul);
  m.Set(1,2) = ::ToInt(m_vec[1][2]*TrigMul);
  m.Set(2,2) = ::ToInt(m_vec[2][2]*TrigMul);

  m.Set(0,3) = 0;
  m.Set(1,3) = 0;
  m.Set(2,3) = 0;
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
FVector3 Lib3D::operator*(const FVector3& v, const FMatrix33 &m) 
{
	return FVector3(	m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2],
										m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2],
										m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2]);
}




Vector4s FVector3::CreateVector4s(int bias) const
{
	const Vector4s v(	int(m_val[0])+bias,
										int(m_val[1])+bias,
										int(m_val[2])+bias);

	return v;
} 