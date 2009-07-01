#ifndef _IMATH_H_
#define _IMATH_H_

class Vector4s;

namespace Lib3D
{
// Matrix and trigonometric calculations using integers

#define PI 3.14159265358979323846f               // floating point pi value

#define ANGLE2PI 2048                            // equivalent to 2*PI, must be power of 2
#define ANGLEPI	(ANGLE2PI >> 1)
#define ANGLEPI2	(ANGLE2PI >> 2)
#define PGL_PI (ANGLE2PI>>1)
#define ANGLEMASK (ANGLE2PI-1)                   // angle mask use to acces trigonometric tables

#define ATAN_SIZE 512                            // PI/4 precalculated array of atan(x)*ANGLE2PI

#define HALF_PRECISION 8192
#define COS_SIN_SHIFT 14                         // shifted 2^n value for sinus/cosinus result
#define COS_SIN_MUL (1 << COS_SIN_SHIFT)         // sinus/cosinus max value (==1)

extern const int TSIN[ANGLE2PI];               // sinus table


inline short Sinus(int a) {return TSIN[a & ANGLEMASK];}
inline short Cosinus(int a) {return TSIN[ (a + (ANGLE2PI >> 2)) & ANGLEMASK]; }

//inline static int DownShift16(const int x) {return (x + 0x7FFF) >> 16;}
//inline static int DownShift8 (const int x) {return (x + 0x7F  ) >>  8;}
//template<int K> inline static int DownShift(const int x) {return (x + ((1<<K-1)-1)) >> K;}

#define DownShift16(x) (((x)+0x7FFF) >> 16)		// [FIXME] this only rounds correctly for positive numbers.. negatives should use (((x)-0x7FFF) >> 16)
#define DownShift8(x) (((x)+0x7F) >> 8)			// [FIXME] this only rounds correctly for positive numbers.. negatives should use (((x)-0x7F) >> 8)
//template<int K> inline static int DownShift(const int x) {return (x + ((1<<K-1)-1)) >> K;}

inline long int Abs(long int a)
{
    return a > 0 ? a : -a;
}

inline int Abs(int a)
{
    return a > 0 ? a : -a;
}

inline short Abs(short a)
{
    return a > 0 ? a : -a;
}

inline char Abs(char a)
{
    return a > 0 ? a : -a;
}

inline int Max(int a, int b)
{
    return a > b ? a : b;
}

inline int Min(int a, int b)
{
    return a < b ? a : b;
}

inline int Clamped(int i_x, int i_min, int i_max)
{
	return (i_x < i_min) ? i_min : (i_x > i_max) ? i_max : i_x;
}

inline int Square(int i_x)
{
	return i_x * i_x;
}

inline int Sign(int a)
{
	return (a >= 0) ? +1 : -1;
}

// usefull functions
int Log2(int a);

int GetYOrient(Vector4s const & Src, Vector4s const & Dest);
int GetXOrient(Vector4s const & Src, Vector4s const & Dest);
int AngleDiff(int SrcAngle, int TargetAngle);

int GetFovFromXAngle(int in_nXAngle);
int GetFovFromYAngle(int in_nYAngle);
int GetXAngleFromFov(int in_nFov);
int GetYAngleFromFov(int in_nFov);

inline int Quotient12(int i_A, int i_B)
{
	// returns (A * 4096) / B
	if (Abs(i_A) >= (1 << 19))
	{
		// handle case where i_A << 12 overflows
		return (i_A / i_B) << 12;
	}
	return (i_A << 12) / i_B;
}

inline int QuotientCOS_SIN_SHIFT(int i_A, int i_B)
{
	// returns (A << COS_SIN_SHIFT) / B
	if (Abs(i_A) >= (1 << (31 - COS_SIN_SHIFT)))
	{
		// handle case where i_A << COS_SIN_SHIFT overflows
		return (i_A / i_B) << COS_SIN_SHIFT;
	}
	return (i_A << COS_SIN_SHIFT) / i_B;
}

inline int Product12(int i_A, int i_B)
{
	// returns (A * B) / 4096
	if (Abs(i_A) > 46340)
	{
		// handle cases where i_A * i_B might overflow
		return (i_A >> 12) * i_B;
	}
	if (Abs(i_B) > 46340)
	{
		// handle cases where i_A * i_B might overflow
		return i_A * (i_B >> 12);
	}
	return (i_A * i_B) >> 12;
}

inline int ProductQuotient(int i_A, int i_B, int i_C)
{
	// returns (A * B) / C
	if (Abs(i_A) > 46340)
	{
		// handle cases where i_A * i_B might overflow
		return (i_A / i_C) * i_B;
	}
	if (Abs(i_B) > 46340)
	{
		// handle cases where i_A * i_B might overflow
		return i_A * (i_B / i_C);
	}
	return (i_A * i_B) / i_C;
}

int PositiveQuadraticRoot12(int i_A, int i_B, int i_C);	// positive root of quadratic (or < 0 if no positive root)

}

int Atan2i(int x, int y);	// [FIXME] why is this outside the namespace?


#endif // _IMATH_H_
