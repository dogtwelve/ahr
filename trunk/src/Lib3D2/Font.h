
#ifndef _FONT_H_
#define _FONT_H_

#include "config.h"

#define FONT_Y 15        // char Y size in bits

namespace Lib3D
{
class CLib3D;

void IntToStr(int a, char *aStr);

class CFont
{
public:  
	static int		TextWidth(char *Text);
	static void		DrawTextH(int xd, int yd, int Intensity, char *Text, int* vScreen,int screenx,int screeny,int Center = 0);
};

}//namespace

#endif // _FONT_H_
