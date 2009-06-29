#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <math.h>
#include <assert.h>

#include "defines.h"
#include "trackBg.h"

extern int mFlags;

#define IPHONE_BUG_SQUARE_PVR_TEXTURES
//#define USE_ROAD_INDEX_FROM_OBJECT_NAME


//#define SHOW_CONV_INFO
//make unic pairs (vtx pos, vtx uv) ... and reindex the vertexes
//rewrite the arrays accordingly
unsigned char tempFace[sizeof(OBJ_FACE)];
void ConvertToOpenglFormat( int &nbv, short *&datav, const int &nbf, OBJ_FACE *dataf )
{

#ifdef SHOW_CONV_INFO
	printf("--------------------------------\nBEGIN EXPORT TO OPENGL FORMAT\n");
#endif

	int nbIDXs = nbf*3;
	volatile int nbUnicIDXs = 0;

	OBJ_VTX *objVTX = new OBJ_VTX[nbIDXs];

	
	//OBJ_FACE tempFace = new OBJ_FACE();
	
	if (tempFace == NULL)
	{
		printf("OUT OF MEMORY\n");
		exit(-1);
	}

	//sort by material
	//buble sort
	for(int i =0; i < nbf; i++)
	{		
		bool swap = false;
		for(int j = nbf-1; j > i; j--)
		{
			if( dataf[j-1].idx012 > dataf[j].idx012 )
			{
				//swap
				swap = true;
				//save j-1
				memcpy( tempFace, dataf + (j-1), sizeof(OBJ_FACE) );
				//replace j-1 with j
				memcpy( dataf + (j-1), dataf + j, sizeof(OBJ_FACE) );
				//replace j with save j-1
				memcpy( dataf + j, tempFace, sizeof(OBJ_FACE));				
			}
		}

		if(!swap)
			break;
	}	

	//determine the number of unic pairs (pos,uv) saved in OBJ_VTX
	OBJ_VTX tempObj_Vtx;	
	for(int faceID = 0; faceID < nbf; faceID++)
	{
		volatile short x = 0, y = 0, z = 0;
		volatile int idx = -1;
		volatile int unicIDX = 0; //counter through OBJ_VTX array
		
		//check vertex 0
		//--------------------------------------------------------------------
		idx = dataf[faceID].idx0;
		x = datav[idx * 3 + 0];
		y = datav[idx * 3 + 1];
		z = datav[idx * 3 + 2];
		tempObj_Vtx.set(x, y, z, dataf[faceID].u0, dataf[faceID].v0);
		
		for( unicIDX = 0; unicIDX < nbUnicIDXs; unicIDX++ )
		{
			if( tempObj_Vtx == objVTX[unicIDX] )//check if was allready added 
				break;
		}
		
		if( unicIDX == nbUnicIDXs ) //if new unic pair
		{
			objVTX[nbUnicIDXs].set(	tempObj_Vtx.m_x, tempObj_Vtx.m_y, tempObj_Vtx.m_z,
												tempObj_Vtx.m_u, tempObj_Vtx.m_v );
			
			nbUnicIDXs++;
		}
		
		//reindex
		dataf[faceID].idx0 = unsigned short (unicIDX & 0xFFFF);
		

		//check vertex 1
		//--------------------------------------------------------------------		
		idx = dataf[faceID].idx1;
		x = datav[idx * 3 + 0];
		y = datav[idx * 3 + 1];
		z = datav[idx * 3 + 2];
		tempObj_Vtx.set(x, y, z, dataf[faceID].u1, dataf[faceID].v1);

		for( unicIDX = 0; unicIDX < nbUnicIDXs; unicIDX++ )
		{
			if( tempObj_Vtx == objVTX[unicIDX] )//check if was allready added 
				break;
		}
		
		if( unicIDX == nbUnicIDXs ) //if new unic pair
		{
			objVTX[nbUnicIDXs].set(	tempObj_Vtx.m_x, tempObj_Vtx.m_y, tempObj_Vtx.m_z,
												tempObj_Vtx.m_u, tempObj_Vtx.m_v );
			
			nbUnicIDXs++;
		}
		
		//reindex
		dataf[faceID].idx1 = unsigned short (unicIDX & 0xFFFF);
	

		//check vertex 2
		//--------------------------------------------------------------------		
		idx = dataf[faceID].idx2;
		x = datav[idx * 3 + 0];
		y = datav[idx * 3 + 1];
		z = datav[idx * 3 + 2];
		tempObj_Vtx.set(x, y, z, dataf[faceID].u2, dataf[faceID].v2);

		for( unicIDX = 0; unicIDX < nbUnicIDXs; unicIDX++ )
		{
			if( tempObj_Vtx == objVTX[unicIDX] )//check if was allready added 
				break;
		}
		
		if( unicIDX == nbUnicIDXs ) //if new unic pair
		{
			objVTX[nbUnicIDXs].set(	tempObj_Vtx.m_x, tempObj_Vtx.m_y, tempObj_Vtx.m_z,
												tempObj_Vtx.m_u, tempObj_Vtx.m_v );
			
			nbUnicIDXs++;
		}
		
		//reindex
		dataf[faceID].idx2 = unsigned short (unicIDX & 0xFFFF);				

	}//for(int faceID = 0; faceID < nbf; faceID++)

	//indexes should be on short if this exceeds
	assert(nbUnicIDXs <= 0x10000);

#ifdef SHOW_CONV_INFO
	if( nbv != nbUnicIDXs )
		printf("\nNEEDED REINDEX\t");
	printf("OLD nbv[%d]  NEW nbv[%d] OPTIMIZATION[%f]\n", nbv, nbUnicIDXs, (nbUnicIDXs * 100.0f )/(nbf*3) );
#endif

	nbv = nbUnicIDXs;

	//update datav to the new vtx
	free(datav);
	datav = (short*)malloc( nbv * 3 * 2 );

	for(int i = 0; i < nbv; i++)
	{
		datav[i*3 + 0] = (short)(objVTX[i].m_x & 0xFFFF);
		datav[i*3 + 1] = (short)(objVTX[i].m_y & 0xFFFF);
		datav[i*3 + 2] = (short)(objVTX[i].m_z & 0xFFFF);		
	}

#ifdef SHOW_CONV_INFO
	//print faces
	for(int i = 0; i < nbf; i++)	
		printf("FACE [%d] [%d\t%d\t%d] MAT[%d]\n", i, dataf[i].idx0, dataf[i].idx1, dataf[i].idx2, dataf[i].idx012 );

	printf("\n\nEND EXPORT TO OPENGL FORMAT\n----------------------------------------------\n\n\n");
#endif
	
	delete[] objVTX;
}


bool GetRealTextureWidthBmp(const char *folderName, const char *fileName, int *textureWidth, int *textureHeight)
{
	char imageName[512];
	FILE *f;

	sprintf(imageName, "%s%s.bmp", folderName, fileName);
	f = fopen(imageName, "rb");

	if (!f)
		return false;

	BITMAPFILEHEADER	bmpfh;
	BITMAPINFOHEADER	bmpih;

	fread( &bmpfh, sizeof( BITMAPFILEHEADER ), 1, f );
	fread( &bmpih, sizeof( BITMAPINFOHEADER ), 1, f );

	*textureWidth = bmpih.biWidth;
	*textureHeight = bmpih.biHeight;

	fclose(f);

	return true;
}

typedef struct
{
	unsigned char dum1[2];		 // 2 unused bytes
	unsigned char code;
	unsigned char dummy[9];		 // 9 unused bytes
	unsigned short x_res;
	unsigned short y_res;
	unsigned char twenty_four;
	unsigned char space;

} TGA_HEADER;
//typedef struct
//{
//	unsigned char idLength;
//	unsigned char colorMapType;
//	unsigned char code;
//	unsigned short palStart;       /* 03h  Color map origin */
//	unsigned short palLength;      /* 05h  Color map length */
//	unsigned char palDepth;       /* 07h  Depth of color map entries */
//	unsigned char dummy[4];		 // 9 unused bytes
//	unsigned short x_res;
//	unsigned short y_res;
//	unsigned char twenty_four;
//	unsigned char space;
//} TGA_HEADER;

typedef struct
{
	unsigned char	r, g, b;

} RGB24;

bool GetRealTextureWidthTga(const char *folderName, const char *fileName, int *textureWidth, int *textureHeight)
{
	char imageName[512];
	FILE *f;

	sprintf(imageName, "%s\\%s.tga", folderName, fileName);
	f = fopen(imageName, "rb");

	if (!f)
	{
		printf("File %s doesn't exists\n", imageName);
		return false;
	}

	TGA_HEADER	tgah;

	fread( &tgah, sizeof( TGA_HEADER ), 1, f );

	*textureWidth = (int)tgah.x_res;
	*textureHeight = (int)tgah.y_res;

	fclose(f);

	return true;
}

bool GetRealTextureWidth(const char *folderName, const char *fileName, int *textureWidth, int *textureHeight)
{
	if (GetRealTextureWidthTga(folderName, fileName, textureWidth, textureHeight))	
		return true;

	if (GetRealTextureWidthBmp(folderName, fileName, textureWidth, textureHeight))	
		return true;

	return false;
}

bool isTextureOpaque(const char *folderName, const char *fileName)
{
	char imageName[512];
	FILE *f;

	sprintf(imageName, "%s\\%s.tga", folderName, fileName);
	f = fopen(imageName, "rb");

	if (!f)
	{
		printf("File %s doesn't exists\n", imageName);
		return false;
	}

	TGA_HEADER	tgah;
	fread( &tgah, sizeof( TGA_HEADER ), 1, f );

	//if (((tgah.twenty_four != 24) || (tgah.code != 2)) &&
	//	(tgah.colorMapType == 0))
	//{
	//	printf("== FILE : %s == ", namein );
	//	printf("Error TGA format is not 24bits uncompressed or paletted\n");
	//	fclose(fpin);
	//	return -1;
	//}

	bool bOpaque = true;

	// read input data
	if (tgah.dum1[1] != 0) //if (tgah.colorMapType != 0)
	{
		// Indexed
		int n = tgah.dummy[4] / 8; // tgah.palDepth / 8;

		//if (n != 3)
		//{
		//	printf("== FILE : %s == ", namein );
		//	printf("Error TGA format has not 24bits palette depth\n\n");
		//	fclose(fpin);
		//	return -1;
		//}

		int			size = tgah.x_res * tgah.y_res;
		int			palLength = tgah.dummy[2] | (tgah.dummy[3] << 8);
		int			palSize = (tgah.dummy[4] * palLength) / 8; // (tgah.palDepth * tgah.palLength / 8);
		unsigned char *pal = new unsigned char[palSize];
		fread(pal, 1, palSize, f);
		unsigned char *data = new unsigned char[size];
		fread(data, 1, size, f);

		fclose(f);

		for (int i=0, j=0; i<palSize && j<palLength; j++, i+=n)
		{
			RGB24 color;
			color.r = pal[i];
			color.g = pal[i+1];
			color.b = pal[i+2];

			if (color.r == 0xFF && color.g == 0 && color.b == 0xFF)
			{
				// see if the color is actually used
				unsigned char *d = data;
				for (int k=size-1; k>=0; --k)
				{
					if (*d == j)
					{
						bOpaque = false;
						false;
					}
					++d;
				}

				if (!bOpaque)
					break;
			}
		}

		delete[] data;
		delete[] pal;
	}
	else
	{
		int			size = tgah.x_res * tgah.y_res;
		RGB24*		rgbin = (RGB24*)malloc( sizeof(RGB24) * size );
		fread( rgbin, sizeof(RGB24) * size, 1, f );
		fclose( f );

		for (int i=0; i<size; i++)
		{
			RGB24 color = rgbin[i];

			if (color.r == 0xFF && color.g == 0 && color.b == 0xFF)
			{
				bOpaque = false;
				break;
			}
		}

		free( rgbin );
	}

	return bOpaque;
}

extern TextureInfo	texturesInfo[MAX_TEXTURES];
extern int	 text_nb;

bool isRoadTexture(char *textureName)
{
	for (int i = 0; i < text_nb; i++)
	{
		if (!strcmp(texturesInfo[i].name, textureName))
		{
			return true;
		}
	}

	return false;
}

bool isPowerOfTwo(int n)
{
	while (n > 0 && (n & 0x01) == 0)
		n >>= 1;

	return n == 1;
}

bool AddBgTexture(int materialId, char *textureName, const char *texturesFolder, TextureInfo *texturesInfoBg, int &textbg_nb, FILE *fTextures)
{
	int textureWidth = -1;
	int textureHeight = -1;

	GetRealTextureWidth(texturesFolder, textureName, &textureWidth, &textureHeight);

	texturesInfoBg[textbg_nb].textureWidth = textureWidth;
	texturesInfoBg[textbg_nb].textureHeight = textureHeight;
	
	if (!isPowerOfTwo(textureWidth) || !isPowerOfTwo(textureHeight))
		printf("%d. %s width, height: %d, %d\n", textbg_nb, textureName, textureWidth, textureHeight);

	texturesInfoBg[textbg_nb].materialIndex = materialId;

	texturesInfoBg[textbg_nb].name = &textureName[0];

	if ((mFlags & FLAG_USE_PVRTC) &&
#ifdef IPHONE_BUG_SQUARE_PVR_TEXTURES
		textureWidth == textureHeight &&
#endif // IPHONE_BUG_SQUARE_PVR_TEXTURES
		!texturesInfoBg[textbg_nb].maskName &&
		!isRoadTexture(textureName))
//		isTextureOpaque(texturesFolder, textureName))
	{
		//printf("\t\t opaque, square: %s\n", textureName);

		fprintf(fTextures, "%s -pvrtc\n", texturesInfoBg[textbg_nb].name);
	}
	else
	{
		int masterIndex = SearchMasterTexture(texturesInfoBg, textbg_nb);
		if (masterIndex >= 0)
		{
			texturesInfoBg[textbg_nb].masterName = texturesInfoBg[masterIndex].name;			
			texturesInfoBg[textbg_nb].palId = texturesInfoBg[masterIndex].nPals;
			texturesInfoBg[masterIndex].nPals++;

			if (texturesInfoBg[textbg_nb].maskName)
				fprintf(fTextures, "%s -p %d %s -m %s\n", texturesInfoBg[textbg_nb].name, texturesInfoBg[textbg_nb].palId, texturesInfoBg[textbg_nb].masterName, texturesInfoBg[textbg_nb].maskName);
			else
				fprintf(fTextures, "%s -p %d %s\n", texturesInfoBg[textbg_nb].name, texturesInfoBg[textbg_nb].palId, texturesInfoBg[textbg_nb].masterName);
		}
		else
		{
			if (texturesInfoBg[textbg_nb].maskName)
				fprintf(fTextures, "%s -m %s\n", texturesInfoBg[textbg_nb].name, texturesInfoBg[textbg_nb].maskName);
			else
				fprintf(fTextures, "%s\n", texturesInfoBg[textbg_nb].name);
		}
	}

	textbg_nb++;

	if (textbg_nb > MAX_TEXTURES)
	{
		printf("BgExp: Too many textures in the config file, check if [END] is specified.");
		return false;
	}

	return true;
}

int SearchBgTexture(char *textureName, TextureInfo *texturesInfoBg, int textbg_nb)
{
	for (int i=0; i<textbg_nb; i++)
	{
		if (strcmp(textureName, texturesInfoBg[i].name) == 0)
			return i;
	}

	return -1;
}

char *findStr(char *src, char *val)
{
	char *p = src;	

	while (*p)
	{
		char *s = p;
		char *d = val;

		while (*s && *d)
		{
			if (toupper(*s) != toupper(*d))
				break;

			s++;
			d++;
		}

		if (*d == 0)
			break;

		p++;
	}

	return p;
}

//TextureInfo * LoadTextureInfo(char * FileName)
//{
//	char	txt[512];
//	TextureInfo * result = new TextureInfo[MAX_TEXTURES];
//	FILE*	fin = fopen( FileName, "rt" );
//
//	if (fin == NULL)
//	{
//		printf("== FILE : %s == ", FileName );
//		printf("Config file not found!\n\n");
//		return NULL;
//	}
//
//	char *p = FileName;
//	char TexturesDir[256]="";
//	while (p != NULL && (p = findStr(p, "track")) != NULL)
//	{
//		int value = 0;
//		int res = sscanf (p + 5, "%d\\", &value);
//
//		if (res == 1)
//		{
//			sprintf(TexturesDir, "Textures\\Track%d\\", value);
//			break;
//		}
//
//		p += 5;
//	}
//
//	// find TEXTURES
//	txt[0]=0;
//	while (strcmp( txt, "[BG_TEXTURES]" ) )
//		fscanf( fin, "%s\n", txt );
//	int text_nb=0;
//	int temp1 = 0;
//	int temp2 = 0;
//	char indexString[20];
//	char fileName[256];
//	while (1)
//	{
//		fgets(txt,512,fin);
//
//		int textureWidth = -1;
//		int textureHeight = -1;
//
//		//int readValues = sscanf( txt, "%s %s %d\n",indexString , fileName,&textureWidth);
//		int readValues = sscanf( txt, "%s %s\n",indexString , fileName);
//
//		if (strcmp( indexString, "[END]" ) == 0)
//			break;
//		if ( readValues == -1) 
//			continue; //empty line , go ahead
//
//		if ( readValues != 2)
//		{
//			printf("BgExp:Texture parameters for texture with index %d are incorrect !\n",text_nb);
//			printf("BgExp:Terminating ...\n");
//			return NULL;
//		}
//		else
//		{
//			GetRealTextureWidth(TexturesDir, fileName, &textureWidth, &textureHeight);
//
//			result[text_nb].textureWidth = textureWidth;
//			result[text_nb].textureHeight = textureHeight;
//			if (!isPowerOfTwo(textureWidth) || !isPowerOfTwo(textureHeight))
//				printf("%d. %s width, height: %d, %d\n", text_nb, fileName, textureWidth, textureHeight);
//		}
//
//		sscanf(indexString,"%d" ,&result[text_nb].materialIndex);
//		text_nb++;
//		if (text_nb>MAX_TEXTURES)
//		{
//			printf("BgExp: Too many textures in the config file, check if [END] is specified.");
//			return NULL;
//		}
//	}
//
//	fclose( fin );
//	return result;
//}

/*short* SortVerticesZ(short *vertexData, int vertexDataNb, int *sortedIndexes)
{
	int n = vertexDataNb / SECTION_VERT;
	short *p = vertexData + ((SECTION_VERT >> 1) * 3) + 2;
	short *centerVert = new short[n];
	for (int i=0; i<n; i++)
	{
		centerVert[i] = *p;
		p += SECTION_VERT * 3;
	}

	for (int i=0; i<n; i++)
		sortedIndexes[i] = i;

	for (int i=0; i<n-1; i++)
		for (int j=i+1; j<n; j++)
		{
			if (centerVert[i] > centerVert[i])
			{
				int tmp = centerVert[i];
				centerVert[i] = centerVert[j];
				centerVert[j] = tmp;
			}
		}

	return centerVert;
}
*/

int FindBgObjectIndex(short *center, short *vertexData, int vertexDataNb, char tag)
{
	int n = vertexDataNb / SECTION_VERT;
	int index = 0xFFFF;
	long long distMin = 0xFFFFFFFF;
	long long dist;

	bool bInsideRoadSection = false;

	int p[8];

	for (int i=0; i<n; i++)
	{
		int xm = vertexData[i * SECTION_VERT * 3 + 3 * (SECTION_VERT / 2)];
		int zm = vertexData[i * SECTION_VERT * 3 + 3 * (SECTION_VERT / 2) + 2];

		dist = (center[0] - xm) * (center[0] - xm) +
			(center[2] - zm) * (center[2] - zm);

		if (dist < distMin)
		{
			distMin = dist;
			index = i;
		}

		// Check if inside current road section
		/*p[0] = vertexData[i * SECTION_VERT * 3 + 3];
		p[1] = vertexData[i * SECTION_VERT * 3 + 3 + 2];

		p[2] = vertexData[i * SECTION_VERT * 3 + 3 * (SECTION_VERT - 2)];
		p[3] = vertexData[i * SECTION_VERT * 3 + 3 * (SECTION_VERT - 2) + 2];

		int next_i = i + 1;
		if (next_i >= n)
			next_i = 0;

		p[4] = vertexData[next_i * SECTION_VERT * 3 + 3 * (SECTION_VERT - 2)];
		p[5] = vertexData[next_i * SECTION_VERT * 3 + 3 * (SECTION_VERT - 2) + 2];

		p[6] = vertexData[next_i * SECTION_VERT * 3 + 3];
		p[7] = vertexData[next_i * SECTION_VERT * 3 + 3 + 2];

		bool bInsideSection = true;

		for (int k=0; k<4; k++)
		{
			int i1 = 2 * k;
			int i2 = 2 * ((k + 1) % 4);
			int i3 = 2 * ((k + 2) % 4);
			
			int a = 0;
			int b = 0;
			if (p[i2] - p[i1] != 0)
			{
				int slope = ((p[i2 + 1] - p[i1 + 1]) << 16) / (p[i2] - p[i1]);
			
				a = (p[i3 + 1] - p[i1 + 1]) - ((slope * (p[i3] - p[i1])) >> 16);
				b = (center[2] - p[i1 + 1]) - ((slope * (center[0] - p[i1])) >> 16);
			}
			else
			{
				a = p[i3] - p[i1];
				b = center[2] - p[i1];
			}
			
			if (a * b <= 0)
			{
				bInsideSection = false;
				break;
			}
		}

		if (bInsideSection)
		{
			bInsideRoadSection = true;
			index = i;
			break;
		}
		*/
	}

	/*if (tag == 'N')
	{
		printf("Nitro: %d %d\n", index, bInsideRoadSection);
	}
	else if (bInsideRoadSection)
	{
		printf("Object: %d %c\n", index, tag);
	}*/

	return index;
}

unsigned char ClampTexUV(int val)
{
	if (val < 0)
		val = 0;

	if (val > 0xFD)
		val = 0xFD;

	return (unsigned char) val;
}

int ExportTrackBg(const char *inFile, const char *outFile, TextureInfo * texturesInfoArray, short *vertexData, int vertexDataNb)
{
	printf("BgExp starts...\n");	

	if (texturesInfoArray == NULL)
		return -1;

	FILE*	fin = fopen(inFile, "rt" );

	if (fin == NULL)
	{
		printf("Error opening file : %s\n", inFile);
		return -1;
	}

	FILE*	fout = fopen(outFile, "wb" );
	if (fout == NULL)
	{
		printf("Error creating file : %s\n", outFile);
		return -1;
	}

	int nLensFlareObjects = 0;
#define LENS_FLARE_OBJECTS_MAX			256
	short lensFlareVertexBuf[LENS_FLARE_OBJECTS_MAX * 4];
	memset(lensFlareVertexBuf, 0, 2 * LENS_FLARE_OBJECTS_MAX * 4);

	BG_HEADER	bg_header;

	bg_header.nb_obj = 0; // we will re-write it after all objects have been written
	bg_header.pad = 0;
	fwrite( &bg_header, sizeof( BG_HEADER ), 1, fout );

	while (1)
	{
		char	txt[512];
		char	name[512];
		char	tag;
		int		index;
		int		nbv, nbf, tmp;
		txt[0] = 0;

		// find node name
		while (strcmp( txt, "*NODE_NAME" ) )
		{
			int	ret = fscanf( fin, "%s", txt );
			if (ret == EOF)	// check end of file
				goto end;
		}
		fscanf( fin, "%s", name );
		// get tag character (FIRST letter of the name)
		tag = name[1];

		// check if it's lens flare
		bool bLensFlareObject = false;
		bool bFaceCulling = false;
		if (strncmp(name, "\"lens_flare", 11) == 0)
			bLensFlareObject = true;
		if (strstr(name, "building") != NULL ||
			strstr(name, "house") != NULL)
			bFaceCulling = true;

#ifdef USE_ROAD_INDEX_FROM_OBJECT_NAME
		// get road index (number after _)
		char *p = strrchr(name, '_');
		if (p)
		{
			index = 0xFFFF;	// no road index

			char*	p2 = p+1;
			while (*p2 && (*p2 >= '0') && (*p2 <= '9'))
				p2++;

			if (p2 > p + 1)
			{
				*p2 = 0;
				index = atoi( p+1 ) / 9;	// section idx
			}			
		}
		else
		{
			index = 0xFFFF;	// no road index
		}
#endif // USE_ROAD_INDEX_FROM_OBJECT_NAME

		// find mesh
		while (strcmp( txt, "*MESH" ) )
		{
			int	ret = fscanf( fin, "\t%s\t{\n", txt );
			if (ret == EOF)	// check end of file
				goto end;
		}

		fscanf( fin, "\t%s\t%d\n", txt, &tmp );
		fscanf( fin, "\t%s\t%d\n", txt, &nbv );
		fscanf( fin, "\t%s\t%d\n", txt, &nbf );
		fscanf( fin, "\t%s\t{\n", txt );


		//----------------------------------
		// VERTEX
		// rax - save it on short
		/*if (nbv >= 256)
		{
			printf("== FILE : %s == ", inFile);
			printf("Input nb vertex Error (must be < 256)!\n\n");

			//free(sortedVerts);

			return -1;
		}
		*/

		short	*datav = (short*)malloc( 3 * 2 * nbv );
		short	*pv = datav;
		short	n = nbv;

		while (n--)
		{
			float		x, y, z;
			int			idx;

			fscanf( fin, "\t*MESH_VERTEX\t%d\t%f\t%f\t%f\n", &idx, &x, &y, &z );

			long		xl, yl, zl;

			xl = (long)(x/8);	// 4x scale + 1cm = 2cm in engine
			yl = (long)(y/8);
			zl = (long)(z/8);

			//xl = (long) (x * 100.0f);
			//yl = (long) (y * 100.0f);
			//zl = (long) (z * 100.0f);

			//xl >>= 2;
			//yl >>= 2;
			//zl >>= 2;

			if ((abs(xl)&0xFFFF0000) || (abs(yl)&0xFFFF0000) || (abs(zl)&0xFFFF0000))
			{
				printf("== FILE : %s == ", inFile );
				printf("Overflow -> to big !!!\n\n" );

				//free(sortedVerts);

				return -1;
			}

			*pv++ = (short)-xl;	// changing axis
			*pv++ = (short)zl;
			*pv++ = (short)yl;
		}

		fscanf( fin, "\t}\n", txt );
		fscanf( fin, "\t%s\t{\n", txt );

		//----------------------------------
		// FACES

		OBJ_FACE	*dataf = (OBJ_FACE*)malloc( sizeof(OBJ_FACE) * nbf );
		OBJ_FACE	*pf = dataf;
		n = nbf;

		while (n--)
		{
			char		str[256];
			int			a, b, c;
			int			idx;

			fscanf( fin, "\t*MESH_FACE\t%d:\tA:\t%d\tB:\t%d\tC:\t%d", &idx, &b, &a, &c );
			str[0] = 0;
			while (strcmp( str, "*MESH_MTLID" ))
				fscanf( fin, "%s", str );
			fscanf( fin, "\t%d", &idx );


			bool indexFound=false;
			for (int textureIndex=0 ; 
				textureIndex < MAX_TEXTURES ;
				textureIndex++)
			{
				if ((texturesInfoArray[textureIndex].materialIndex-1) == idx)
				{
					idx = textureIndex;
					indexFound = true;
					break;
				}
			}
			if (!indexFound)
			{
				printf("BgExp:Texture index ""%d"" not found in the config file! Track compilation aborted!\n",idx);

				//free(sortedVerts);

				return -1;
			}

			//			// save indexes (8 low bits in idx? and 2 higher bits in idx012) + material idx 
			// save indexes (8 bit only 255 vert, and 8bit material idx)

			pf->idx0 = a&0xFF;
			pf->idx1 = b&0xFF;
			pf->idx2 = c&0xFF;
			//			pf->idx012 = (((a&0xF00)>>8)<<4) | (((b&0xF00)>>8)<<2) | ((c&0xF00)>>8) | (idx<<6);
			pf->idx012 = idx;

			pf++;
		}


		//----------------------------------
		// TVERT
		int		nbtv;

		fscanf( fin, "\t}\n", txt );
		fscanf( fin, "\t%s\t%d\n", txt, &nbtv );
		fscanf( fin, "\t%s\t{\n", txt );


		OBJ_TV	*datatv = (OBJ_TV*)malloc( sizeof(OBJ_TV) * nbtv );
		OBJ_TV	*ptv = datatv;
		n = nbtv;

		while (n--)
		{
			float		u, v, w;
			int			idx;

			fscanf( fin, "\t*MESH_TVERT\t%d\t%f\t%f\t%f\n", &idx, &u, &v, &w );

			//if (u < 0.0f)
			//	u = abs(u);
			//if (v < 0.0f)
			//	v = abs(v);


			//			ptv->u = (unsigned char)((u * texturewidth) + 0.5f);
			//			ptv->v = (unsigned char)((v * texturewidth) + 0.5f);
			ptv->u = u;
			ptv->v = v;

			ptv++;
		}

		//----------------------------------
		// TFACE
		int		nbtf;

		fscanf( fin, "\t}\n", txt );
		fscanf( fin, "\t%s\t%d\n", txt, &nbtf );
		fscanf( fin, "\t%s\t{\n", txt );

		if (nbtf != nbf)
		{
			printf("== FILE : %s == ", inFile);
			printf("Error : nb tface != nb face\n\n");

			//free(sortedVerts);

			return -1;
		}


		pf = dataf;
		n = nbf;

		while (n--)
		{
			int		a, b, c;
			int		idx;

			fscanf( fin, "\t*MESH_TFACE\t%d\t%d\t%d\t%d\n", &idx, &b, &a, &c );

			//			ptv->u = (unsigned char)((u * texturewidth) + 0.5f);
			//			ptv->v = (unsigned char)((v * texturewidth) + 0.5f);
			int textureWidth = texturesInfoArray[pf->idx012].textureWidth;
			int textureHeight = texturesInfoArray[pf->idx012].textureHeight;
			
			pf->u0 = ClampTexUV((datatv[a].u * textureWidth) + 0.5f);
			pf->v0 = ClampTexUV((datatv[a].v * textureHeight) + 0.5f);
			pf->u1 = ClampTexUV((datatv[b].u * textureWidth) + 0.5f);
			pf->v1 = ClampTexUV((datatv[b].v * textureHeight) + 0.5f);
			pf->u2 = ClampTexUV((datatv[c].u * textureWidth) + 0.5f);
			pf->v2 = ClampTexUV((datatv[c].v * textureHeight) + 0.5f);

			//printf("\tMesh face %d ... %d,%d %d,%d %d,%d (w %d, h %d, idx %d)\n", idx, pf->u0, pf->v0, pf->u1, pf->v1, pf->u2, pf->v2, textureWidth, textureHeight, pf->idx012);

			pf++;
		}

		// ========== ANIMS ===============

		OBJ_BG_FRAME	frames[OBJ_BG_FRAME_MAX];
		int				nb_frames = 0;

		// find ANIM node or GEOMOBJ( in this case no anim!)
		while (strcmp( txt, "*TM_ANIMATION" ) && strcmp( txt, "*GEOMOBJECT" ))
		{
			int	ret = fscanf( fin, "%s", txt );
			if (ret == EOF)	// check end of file
				break;
		}

		// do we have anim info ?
		if (strcmp( txt, "*TM_ANIMATION" ) == 0)
		{
			int		pos, i;
			float	fx, fy, fz, fw;

			// TRANS
			while (strcmp( txt, "*CONTROL_POS_TRACK" ) && strcmp( txt, "*GEOMOBJECT" ) && strcmp( txt, "*CONTROL_ROT_TRACK" ))
			{
				int	ret = fscanf( fin, "%s", txt );
				if (ret == EOF)	// check end of file
					break;
			}
			if ((strcmp( txt, "*GEOMOBJECT") == 0) || (strcmp( txt, "*CONTROL_ROT_TRACK" ) == 0))
			{
				printf("== FILE : %s == ", inFile);
				printf("Animation Error -> no TRANSLATION track on <%s> !!!\n\n", name );
				//free(sortedVerts);
				return -1;
			}
			fscanf( fin, "%s", txt );

			i = 0;
			pos = 1;
			while (pos != -1)
			{
				pos = -1;
				fscanf( fin, "\t*CONTROL_POS_SAMPLE\t%d\t%f\t%f\t%f\n", &pos, &fx, &fy, &fz );
				if (pos == -1)	break;

				int xl = (long)(fx/8);	// 4x scale + 1cm = 2cm in engine
				int yl = (long)(fy/8);
				int zl = (long)(fz/8);

				if ((abs(xl)&0xFFFF0000) || (abs(yl)&0xFFFF0000) || (abs(zl)&0xFFFF0000))
				{
					printf("== FILE : %s == ", inFile);
					printf("Overflow anim pos -> to big !!!\n\n" );
					//free(sortedVerts);
					return -1;
				}

				frames[i].pos_x = (short)-xl;
				frames[i].pos_y = (short)zl;
				frames[i].pos_z = (short)yl;

				i++;
				nb_frames++;	//inc frames
			}


			// ROT
			while (strcmp( txt, "*CONTROL_ROT_TRACK" ) && strcmp( txt, "*GEOMOBJECT" ))
			{
				int	ret = fscanf( fin, "%s", txt );
				if (ret == EOF)	// check end of file
					break;
			}
			if (strcmp( txt, "*GEOMOBJECT") == 0)
			{
				printf("== FILE : %s == ", inFile);
				printf("Animation Error -> no ROTATION track on <%s> !!!\n\n", name );
				//free(sortedVerts);
				return -1;
			}

			fscanf( fin, "%s", txt );

			float	XL = 0.0f, YL = 0.0f, ZL = 1.0f, WL = 0.0f;

			i = 0;
			pos = 1;
			while (pos != -1)
			{
				pos = -1;
				fscanf( fin, "\t*CONTROL_ROT_SAMPLE\t%d\t%f\t%f\t%f\t%f\n", &pos, &fx, &fy, &fz, &fw );
				if (pos == -1)	break;

				// transcode ang/axis to quaternion

				float	sin_a = (float)sin( fw / 2 );
				float	cos_a = (float)cos( fw / 2 );

				float	X    = -(fx * sin_a);
				float	Y    = fz * sin_a;
				float	Z    = fy * sin_a;
				float	W    = cos_a;

				// all transformation are cumulative.. add ! so we have a true rotation
				fx = WL*X + XL*W + YL*Z - ZL*Y;
				fy = WL*Y - XL*Z + YL*W + ZL*X;
				fz = WL*Z + XL*Y - YL*X + ZL*W;
				fw = WL*W - XL*X - YL*Y - ZL*Z;

				XL = fx;
				YL = fy;
				ZL = fz;
				WL = fw;

				int xl = (long)(fx*127);
				int yl = (long)(fy*127);
				int zl = (long)(fz*127);
				int wl = (long)(fw*127);

				frames[i].rot[0] = (char)xl;
				frames[i].rot[1] = (char)yl;
				frames[i].rot[2] = (char)zl;
				frames[i].rot[3] = (char)wl;

				i++;
			}
		}

		// ====================================
		// RADIUS & CENTER

		short	center[3];
		short	radius;

		if (nb_frames)
		{
			// center is first frame !
			center[0] = frames[0].pos_x;
			center[1] = frames[0].pos_y;
			center[2] = frames[0].pos_z;

			// find extrems
			int		max = -999999;

			pv = datav;
			n = nbv;
			while (n--)
			{
				int	d = (pv[0] - center[0]) * (pv[0] - center[0]) + 
					(pv[1] - center[1]) * (pv[1] - center[1]) +
					(pv[2] - center[2]) * (pv[2] - center[2]);
				if (max < d)
					max = d;

				pv+=3;
			}

			// compute radius
			radius = (short)sqrt( (double)max );

			// recenter data
			pv = datav;
			n = nbv;
			while (n--)
			{
				for (int i = 0; i < 3; i++)
				{
					*pv -= center[i];
					pv++;
				}
			}
		}
		else if (!bLensFlareObject)
		{
			// find extrems
			short	max[3];
			short	min[3];

			max[0] = max[1] = max[2] = -32768;
			min[0] = min[1] = min[2] = 32767;

			pv = datav;
			n = nbv;
			while (n--)
			{
				for (int i = 0; i < 3; i++)
				{
					if (max[i] < *pv)
						max[i] = *pv;
					if (min[i] > *pv)
						min[i] = *pv;
					pv++;
				}
			}

			// find center
			center[0] = (min[0] + max[0]) / 2;
			center[1] = (min[1] + max[1]) / 2;
			center[2] = (min[2] + max[2]) / 2;

			// compute radius
			radius =(short)sqrt((double)((max[0] - min[0])/2) * ((max[0] - min[0])/2) +
				((max[1] - min[1])/2) * ((max[1] - min[1])/2) +
				((max[2] - min[2])/2) * ((max[2] - min[2])/2));


			// recenter data
			pv = datav;
			n = nbv;
			while (n--)
			{
				for (int i = 0; i < 3; i++)
				{
					*pv -= center[i];
					pv++;
				}
			}
		}

#ifndef USE_ROAD_INDEX_FROM_OBJECT_NAME
		if (nbv == 0)
			index = 0xFFFF;
		else index = FindBgObjectIndex(center, vertexData, vertexDataNb, tag);
#endif

		//===================================
		if (!bLensFlareObject)
		{
			ConvertToOpenglFormat(nbv, datav, nbf, dataf);

			OBJ_BG_HEADER	header;

			header.nb_vert = nbv;
			header.nb_face = nbf;
			header.pos_x = center[0];
			header.pos_y = center[1];
			header.pos_z = center[2];
			header.radius = radius;
			header.tag = tag;
			header.pad8 = 0;
			header.pad16 = 0;
			header.index = index;
			header.nb_frames = nb_frames;

#define FLAG_CULL_FACE					0x8000

			if (bFaceCulling)
				header.nb_frames |= FLAG_CULL_FACE;

			fwrite( &header, sizeof(OBJ_BG_HEADER), 1, fout );
			fwrite( datav, nbv*3*2, 1, fout );
			fwrite( dataf, nbf*sizeof(OBJ_FACE), 1, fout );

			if (nb_frames != 0)
			{
				fwrite( frames, sizeof(OBJ_BG_FRAME)*nb_frames, 1, fout );
			}

			// inc number of objects
			bg_header.nb_obj++;
		}
		else
		{
			int id = nLensFlareObjects * 4;
			lensFlareVertexBuf[id + 0] = datav[0];
			lensFlareVertexBuf[id + 1] = datav[1];
			lensFlareVertexBuf[id + 2] = datav[2];
			lensFlareVertexBuf[id + 3] = index;

			nLensFlareObjects++;			
		}

		free( datatv );
		free( datav );
		free( dataf );
	}

end:

	unsigned char idx = (unsigned char) nLensFlareObjects;
	fwrite(&idx, 1, 1, fout);
	if (nLensFlareObjects > 0)
	{
		// sort vertices by index
		for (int i=0; i<nLensFlareObjects-1; i++)
		{
			for (int j=i+1; j<nLensFlareObjects; j++)
			{
				int id1 = 4 * i + 3;
				int id2 = 4 * j + 3;
				if ((int)lensFlareVertexBuf[id1] > (int)lensFlareVertexBuf[id2])
				{
					id1 -= 3;
					id2 -= 3;
					for (int k=0; k<4; k++)
					{
						int tmp = lensFlareVertexBuf[id1 + k];
						lensFlareVertexBuf[id1 + k] = lensFlareVertexBuf[id2 + k];
						lensFlareVertexBuf[id2 + k] = tmp;
					}
				}
			}
		}
		fwrite(lensFlareVertexBuf, nLensFlareObjects * 4 * 2, 1, fout);
	}

	fclose( fin );

	// rewrite header
	fseek( fout, 0, SEEK_SET );
	fwrite( &bg_header, sizeof( BG_HEADER ), 1, fout );

	//free(sortedVerts);

	fclose(fout);

	//delete texturesInfoArray;
	printf("BgExp ends\n");

	return 0;
}

//////////////////////////////////////////////////////////////////////////

int SearchMasterTexture(TextureInfo *textureInfo, int id)
{
	char *p = strchr(textureInfo[id].name, '_');

	if (!p)
		return -1;

	int n = p - textureInfo[id].name;

	for (int i=0; i<id; i++)
	{
		// Don't take it into account if it's the same texture name
		if (!strcmp(textureInfo[id].name, textureInfo[i].name))
			continue;

		char *q = strchr(textureInfo[i].name, '_');
		if (!q)
			continue;

		if (q - textureInfo[i].name != n)
			continue;

		if (!strncmp(textureInfo[id].name, textureInfo[i].name, n))
			return i;
	}

	return -1;
}
