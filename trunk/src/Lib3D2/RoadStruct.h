#ifndef _ROADSTRUCT_H_
#define _ROADSTRUCT_H_

#include "matrix.h"
#include "Vertex.h"
#include "texture.h"

using namespace Lib3D;


#define SECTION_NBVERT			9
#define SECTION_CENTER_IDX		(SECTION_NBVERT/2)		// center
#define SECTION_COL_LEFT_IDX	(SECTION_NBVERT-2)		// wall col
#define SECTION_COL_RIGHT_IDX	1
#define SECTION_EXT_LEFT_IDX	(SECTION_NBVERT-3)		// road exterior (grass)
#define SECTION_EXT_RIGHT_IDX	2
#define SECTION_VIB_LEFT_IDX	(SECTION_NBVERT-4)		// between road & exterior (vibreur)
#define SECTION_VIB_RIGHT_IDX	3

#define SECTION_SHOWN_MAX		30
#define SECTION_TEXTURE_GROUND_MAX		(SECTION_NBVERT-1)
#define SECTION_TEXTURE_MAX		((SECTION_NBVERT-1) + 4 + 2 + 1 + 2) // 4 for fence / 2 for billboard +1 for top + 2 for 2nd billboard
#define VISUAL_TEXTURE_MAX		128


struct RoadSection
{
	long			Vertex[SECTION_NBVERT * 3];		// section vertex XYZ
	
	short			SpeedFactor;					// Speed factor (computed from angles) + SIGN is for left/right
	unsigned short	VisualId;						// Textures / billboards

	CMatrix44		RotMat;							// Rotation Matrix for road orientation
	CMatrix44		RotMatInv;						// Rotation Matrix for road orientation CAMERA -> Inverse

	short			orientX;
	short			orientY;

	long			Length;
	long			LengthInv;						// (1<<20) / length of section for trafic, so they can have constant speed

	long			width;

	short			obstacles; // TODO: add flags here !!!!

#define ROAD_OBSTACLE_LEFT				0x01
#define ROAD_OBSTACLE_CENTER			0x02
#define ROAD_OBSTACLE_RIGHT				0x04
#define ROAD_SECTION_FLAG_TUNNEL		0x10
#define ROAD_SECTION_FLAG_HOLE			0x20

};

#define ROAD_MATERIAL_IDX_TUNNEL				12
#define ROAD_TEXTURE_IDX_TUNNEL					14
#define ROAD_MATERIAL_IDX_BILLBOARD				13

struct RoadSectionTextures
{
// rax: on OGL we use atlas textures instead
	//TTexture		*Textures[SECTION_TEXTURE_MAX];

	unsigned char	MaterialIdx[8 + 4 + 1 + 4]; // 8 road textures, 4 fence textures, 1 tunnel, 4 billboards
	unsigned char	flipX;
	unsigned char	flipY;
	
//#define BILLBOARD_INT		0x00
#define BILLBOARD_MID		0x01
#define BILLBOARD_EXT		0x02
//#define BILLBOARD_TRANS_NONE	0x00	// defined in board3d.h
//#define BILLBOARD_TRANS_MID	0x10
//#define BILLBAORD_TRANS_ADD	0x20

	unsigned char	BillboardPos[4];	// pos and trans
	unsigned char	Billboard2StartEnd[4];
	short			BillboardHeight[4];

	unsigned short	FenceHeight[4]; // from ext left to ext right

	unsigned char	EnvmapIdx;		// idx of envmap for this section
};

struct RoadStruct
{
	int					SectionNb;			// Nb section
	RoadSection			*Section;			// sections

	unsigned char		ShowMinimapNb;
	unsigned char		*ShowMinimap;				

	TVertex				Transformed[SECTION_NBVERT * SECTION_SHOWN_MAX];	// Transformed

	RoadSectionTextures	VisualTextures[VISUAL_TEXTURE_MAX];
	int					m_nbrVisualTextures;
//	unsigned char		TextureSlowDown[VISUAL_TEXTURE_MAX];
//#define TEXTURE_MAT_MASK		0x3F
//#define TEXTURE_FLAG_MASK		0xC0
//#define TEXTURE_FLAG_FLIP_X		(1 << 6)
//#define TEXTURE_FLAG_FLIP_Y		(1 << 7)
//#define TEXTURE_FLIP_X(a)		(((a) & 0x40) != 0)
//#define TEXTURE_FLIP_Y(a)		(((a) & 0x80) != 0)
	unsigned char		TextureTile[VISUAL_TEXTURE_MAX];

	unsigned char		FogColor[3];
	unsigned char		Reverse;

	unsigned char		Friction[4];	// first is for wall collision
	short				Radar;
	unsigned char		SkyLuminance;

};


#endif