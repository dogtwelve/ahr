#ifndef TRACK_BG_H__
#define TRACK_BG_H__

#include "defines.h"

typedef struct
{	
	unsigned short	nb_obj;
	unsigned short	pad;

} BG_HEADER;

typedef struct	
{	
	unsigned short	nb_vert;
	unsigned short	nb_face;

	short			pos_x;
	short			pos_y;
	short			pos_z;
	short			radius;

	char			tag;

	unsigned char	pad8;
	unsigned short	pad16;

	unsigned short	index; // or 0xFFFF for none

	unsigned short	nb_frames;	// or 0 for none

} OBJ_BG_HEADER;

typedef struct
{
	short	pos_x;
	short	pos_y;
	short	pos_z;
	short	pad;

	char	rot[4];

} OBJ_BG_FRAME;

#define OBJ_BG_FRAME_MAX	1024

typedef struct
{
	//to many unic pairs (pos, uv) ... > 256
	//unsigned char	idx0, idx1, idx2;	// low part of idx
	unsigned short	idx0, idx1, idx2;
	unsigned char	idx012;				// 2bits high part of idx + matidx (ii) |ii001122|
	unsigned char	pad8;
	unsigned char	u0, v0, u1, v1, u2, v2;

} OBJ_FACE;

typedef struct
{
	float	u, v;

} OBJ_TV;


//this structure will make unic 
typedef struct tag_OBJ_VTX
{

	short m_x, m_y, m_z;				//position
	unsigned char m_u, m_v;			//tex coord
	
	tag_OBJ_VTX()
	{
		m_x = 0; 
		m_y = 0;
		m_z = 0;
		m_u = 0;
		m_v = 0;
	}
	tag_OBJ_VTX(short x, short y, short z, unsigned char u, unsigned char v)
	{
		m_x = x;
		m_y = y;
		m_z = z;
		m_u = u;
		m_v = v;
	}

	int operator==(const tag_OBJ_VTX &right)
	{
		if( m_x != right.m_x)
			return 0;
		if( m_y != right.m_y)
			return 0;
		if( m_z != right.m_z)
			return 0;

		if( m_u != right.m_u)
			return 0;
		if( m_v != right.m_v)
			return 0;

		return 1;
	}

	void set(short x, short y, short z, unsigned char u, unsigned char v)
	{
		m_x = x;
		m_y = y;
		m_z = z;
		m_u = u;
		m_v = v;
	}

} OBJ_VTX;

struct TextureInfo
{
	int materialIndex;
	int textureWidth;
	int textureHeight;
	char *name;			// texture name
	char *masterName;	// if not NULL, this will be exported as a palette
	char *maskName;
	int nPals;			// total number of palettes, if it's master texture
	int palId;			// palette id
};

bool GetRealTextureWidthBmp(const char *folderName, const char *fileName, int *textureWidth, int *textureHeight);
bool GetRealTextureWidthTga(const char *folderName, const char *fileName, int *textureWidth, int *textureHeight);
bool GetRealTextureWidth(const char *folderName, const char *fileName, int *textureWidth, int *textureHeight);
bool isPowerOfTwo(int n);

bool AddBgTexture(int materialId, char *textureName, const char *texturesFolder, TextureInfo *texturesInfoBg, int &textbg_nb, FILE *fTextures);
int SearchBgTexture(char *textureName, TextureInfo *texturesInfoBg, int textbg_nb);

int ExportTrackBg(const char *inFile, const char *outFile, TextureInfo * texturesInfoArray, short *vertexData, int vertexDataNb);

int SearchMasterTexture(TextureInfo *textureInfo, int id);

#endif // TRACK_BG_H__
