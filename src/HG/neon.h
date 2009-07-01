#ifndef __NEON_H
#define __NEON_H

#include "vertex.h"
#include "face.h"

// ----------------------------------------------
//	class CNeon
//
//	This class handle the 4 neons needed to illuminate a car
//	
// ----------------------------------------------
namespace Lib3D
{
	class TTexture;
	class CLib3D;
//	class TVertex;



#define NEON_WIDTH 40

#define NEON_TEXTURE_SIZE 64
class CNeon
{
	public:

	enum neon_color
	{
		NONE,
		RED,
		ORANGE,
		YELLOW,
		PURPLE,
		GREEN,
		BLUE,
		CYAN,
		//GREY,
		//DARK_RED,
		//DARK_GREEN,
		kTotalColors
	};
	CNeon(int x_left,int x_right,int y_front,int y_back);
	~CNeon();
	void Draw(	Lib3D::CLib3D& lib3d);
	void SetColor(neon_color color_in);
	bool IsActive(){return current_color != NONE;}

	static int GetColorNum(){return kTotalColors;}
	static unsigned short GetColorRGB(int color_index)
	{		
		switch(color_index)
		{
		case NONE: return 0;
		case RED: return 0xF00;
		case ORANGE: return 0xF50;
		case YELLOW: return 0xFF4;
		case PURPLE: return 0xC0E;
		case GREEN: return 0x0F0;
		case BLUE: return 0x27C;
		case CYAN: return 0x7AC;
		//case GREY: return 0x888;
		//case DARK_RED: return 0xF00;
		//case DARK_GREEN: return 0x070;
		}
		return 0;
	}
	private:
	neon_color current_color;
	TVertex	vert[6*4];
	TFace	face[4*4];
	TTexture* NeonTexture;
	void DrawAdditiveNeonGeom(	Lib3D::CLib3D& lib3d,TTexture* texture,TVertex* vtx,TFace* face);
	void CreateAdditiveNeonGeom(TVertex* vtx,
								TFace* face,
								const Vector4s& center,
								int size_x,
								int size_y,
								bool x_aligned);
};
}

#endif //__NEON_H
