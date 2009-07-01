#ifndef FSQRT_H
#define FSQRT_H

namespace Lib3D
{
// Fast square root with integers

int FSqrt(int l2) ;
int FSqrtI(int l2);			// With Interpolation

int FSqrt4(int l2);     // return sqrt(l2)*16
int FSqrt8(int l2);     // return sqrt(l2)*256

//CFixed FSqrt(CFixed);

}
#endif //FSQRT_H