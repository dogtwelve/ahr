#include "neon.h"
#include "devutil.h"
#include "texture.h"
#include "Lib3d.h"

using namespace Lib3D;

// ----------------------------------------------
//	class CNeon
// ----------------------------------------------
CNeon::CNeon(int x_left,int x_right,int y_front,int y_back):
	current_color(NONE),
	NeonTexture(NULL)
{
	//additive neon creation
	int x1 = x_left+10;
	int x2 = x_right-10;
	int y1 = y_front+25;
	int y2 = y_back-30;
	CreateAdditiveNeonGeom(	vert,
							face,
							Vector4s((x2+x1)/2,y2,0),
							x2-x1+30,
							NEON_WIDTH,
							true);

	CreateAdditiveNeonGeom(	vert+6,
							face+4,
							Vector4s((x2+x1)/2,y1,0),
							x2-x1+30,
							NEON_WIDTH,
							true);

	CreateAdditiveNeonGeom(	vert+12,
							face+8,
							Vector4s(x1,(y2+y1)/2,0),
							NEON_WIDTH,
							y2-y1+10,
							false);

	CreateAdditiveNeonGeom(	vert+18,
							face+12,
							Vector4s(x2,(y2+y1)/2,0),
							NEON_WIDTH,
							y2-y1+10,
							false);

}
CNeon::~CNeon()
{
	if(NeonTexture)
		MM_DELETE NeonTexture;
	NeonTexture = NULL;
}

void CNeon::Draw(	Lib3D::CLib3D& lib3d)
{
	if(current_color == NONE)
		return;

	DrawAdditiveNeonGeom(	lib3d,NeonTexture,vert,face);
	DrawAdditiveNeonGeom(	lib3d,NeonTexture,vert+6,face+4);
	DrawAdditiveNeonGeom(	lib3d,NeonTexture,vert+12,face+8);
	DrawAdditiveNeonGeom(	lib3d,NeonTexture,vert+18,face+12);
}

void CNeon::SetColor(neon_color color_in)
{
	current_color = color_in;
	if(NeonTexture)
	{
		MM_DELETE NeonTexture;
		NeonTexture = NULL;
	}
	switch(current_color)
	{
		case RED:
			//Neon texture
			NeonTexture = NEW TTexture("textures\\neon_red.rle", false); 
			break;
		case GREEN:
			NeonTexture = NEW TTexture("textures\\neon_green.rle", false); 
			break;
		case BLUE:
			NeonTexture = NEW TTexture("textures\\neon_blue.rle", false); 
			break;
		case ORANGE:
			NeonTexture = NEW TTexture("textures\\neon_orange.rle", false); 
			break;
		case YELLOW:
			NeonTexture = NEW TTexture("textures\\neon_yellow.rle", false); 
			break;
		case CYAN:
			NeonTexture = NEW TTexture("textures\\neon_cyan.rle", false); 
			break;
		//case GREY:
		//	NeonTexture = NEW TTexture("textures\\neon_grey.rle", false); 
		//	break;
		//case DARK_RED:
		//	NeonTexture = NEW TTexture("textures\\neon_dark_red.rle", false); 
		//	break;
		case PURPLE:
			NeonTexture = NEW TTexture("textures\\neon_purple.rle", false); 
			break;
		//case DARK_GREEN:
		//	NeonTexture = NEW TTexture("textures\\neon_dark_green.rle", false); 
		//	break;
	}
};

void CNeon::DrawAdditiveNeonGeom(	Lib3D::CLib3D& lib3d,
									TTexture* texture,
									TVertex* vtx,
									TFace* face)
{
	const unsigned int OrFlag = lib3d.m_renderer->TransformVertex(vtx,vtx + 6);
	
	if (!(OrFlag & (Lib3D::kRejectFar|Lib3D::kRejectNear)))
	{
		for(int i = 0; i < 4; i++)
		{
			face[i].SetTexture(texture);
			face[i].m_flag = TFACE_FLAG_TRANS_ADDITIVE;
			//		face[i].m_flag = TFACE_FLAG_ZCORRECTED;
			lib3d.m_renderer->OTTableAdd(face+i);
		}
	}
}

void CNeon::CreateAdditiveNeonGeom(	TVertex* vtx,
									TFace* face,
									const Vector4s& center,
									int size_x,
									int size_y,
									bool x_aligned)
{
	int		texture_u = (NEON_TEXTURE_SIZE-1) << TTexture::TEX_UV_SHIFT;
	int		texture_v = (NEON_TEXTURE_SIZE-1) << TTexture::TEX_UV_SHIFT;
	const int half_x = size_x/2;
	const int half_y = size_y/2;

	vtx[0].InitialPos = Vector4s(center.x + half_x,center.z,center.y + half_y);
	vtx[1].InitialPos = Vector4s(center.x - half_x,center.z,center.y + half_y);
	vtx[2].InitialPos = Vector4s(center.x - half_x,center.z,center.y - half_y);
	vtx[3].InitialPos = Vector4s(center.x + half_x,center.z,center.y - half_y);
	if(x_aligned)
	{
		vtx[4].InitialPos = Vector4s(	(vtx[0].InitialPos.x + vtx[1].InitialPos.x)/2,
										(vtx[0].InitialPos.y + vtx[1].InitialPos.y)/2,
										(vtx[0].InitialPos.z + vtx[1].InitialPos.z)/2);
		vtx[5].InitialPos = Vector4s(	(vtx[2].InitialPos.x + vtx[3].InitialPos.x)/2,
										(vtx[2].InitialPos.y + vtx[3].InitialPos.y)/2,
										(vtx[2].InitialPos.z + vtx[3].InitialPos.z)/2);
	}
	else
	{
		vtx[4].InitialPos = Vector4s(	(vtx[2].InitialPos.x + vtx[1].InitialPos.x)/2,
										(vtx[2].InitialPos.y + vtx[1].InitialPos.y)/2,
										(vtx[2].InitialPos.z + vtx[1].InitialPos.z)/2);
		vtx[5].InitialPos = Vector4s(	(vtx[0].InitialPos.x + vtx[3].InitialPos.x)/2,
										(vtx[0].InitialPos.y + vtx[3].InitialPos.y)/2,
										(vtx[0].InitialPos.z + vtx[3].InitialPos.z)/2);
	}

	if(x_aligned)
	{
		face[0].SetVectorA( vtx + 5 );
		face[0].SetuA(0);
		face[0].SetvA(0);

		face[0].SetVectorB( vtx + 2 );
		face[0].SetuB(0);
		face[0].SetvB(texture_v);

		face[0].SetVectorC( vtx + 1 );
		face[0].SetuC(texture_u);
		face[0].SetvC(texture_v);

		face[1].SetVectorA( vtx + 5 );
		face[1].SetuA(0);
		face[1].SetvA(0);

		face[1].SetVectorB( vtx + 1 );
		face[1].SetuB(texture_u);
		face[1].SetvB(texture_v);

		face[1].SetVectorC( vtx + 4 );
		face[1].SetuC(texture_u);
		face[1].SetvC(0);

		face[2].SetVectorA( vtx + 0 );
		face[2].SetuA(0);
		face[2].SetvA(texture_v);

		face[2].SetVectorB( vtx + 5 );
		face[2].SetuB(texture_u);
		face[2].SetvB(0);

		face[2].SetVectorC( vtx + 4 );
		face[2].SetuC(0);
		face[2].SetvC(0);

		face[3].SetVectorA( vtx + 0 );
		face[3].SetuA(0);
		face[3].SetvA(texture_v);

		face[3].SetVectorB( vtx + 3 );
		face[3].SetuB(texture_u);
		face[3].SetvB(texture_v);

		face[3].SetVectorC( vtx + 5 );
		face[3].SetuC(texture_u);
		face[3].SetvC(0);
	}
	else
	{
		face[0].SetVectorA( vtx + 0 );
		face[0].SetuA(texture_u);
		face[0].SetvA(texture_v);

		face[0].SetVectorB( vtx + 4 );
		face[0].SetuB(0);
		face[0].SetvB(0);

		face[0].SetVectorC( vtx + 1 );
		face[0].SetuC(0);
		face[0].SetvC(texture_v);

		face[1].SetVectorA( vtx + 0 );
		face[1].SetuA(texture_u);
		face[1].SetvA(texture_v);

		face[1].SetVectorB( vtx + 5 );
		face[1].SetuB(texture_u);
		face[1].SetvB(0);

		face[1].SetVectorC( vtx + 4 );
		face[1].SetuC(0);
		face[1].SetvC(0);

		face[2].SetVectorA( vtx + 5 );
		face[2].SetuA(0);
		face[2].SetvA(0);

		face[2].SetVectorB( vtx + 3 );
		face[2].SetuB(0);
		face[2].SetvB(texture_v);

		face[2].SetVectorC( vtx + 2 );
		face[2].SetuC(texture_u);
		face[2].SetvC(texture_v);

		face[3].SetVectorA( vtx + 5 );
		face[3].SetuA(0);
		face[3].SetvA(0);

		face[3].SetVectorB( vtx + 2 );
		face[3].SetuB(texture_u);
		face[3].SetvB(texture_v);

		face[3].SetVectorC( vtx + 4 );
		face[3].SetuC(texture_u);
		face[3].SetvC(0);
	}
}
