#include <windows.h>
#include <winbase.h>
#include <shlwapi.h>
#include <stdio.h>
#include <io.h>
//#include "IniContainer.h"

#include "defines.h"
#include "trackBg.h"
#include "parser.h"

//////////////////////////////////////////////////////////////////////////
//Used to count how much times given texture is used
int texturesUsageStatistics[MAX_TEXTURES];
//int textureIndexes[MAX_TEXTURES];

TextureInfo texturesInfoBg[MAX_TEXTURES];
TextureInfo	texturesInfo[MAX_TEXTURES];

bool bReverse;
int	 text_nb;
int  textbg_nb;
char	text[MAX_TEXTURES][128];

char *emptyString = "";

bool CheckTexture(const char *textureDirectory, const char *fileName);

//////////////////////////////////////////////////////////////////////////

#define MAX_ANIM_FRAMES	32
#define MAX_ANIMS		64
	
typedef struct {
	int nIndex;
	int nFrames;
	int textureBgId;
	char textures[MAX_ANIM_FRAMES][128];
	int framesMs[MAX_ANIM_FRAMES];
	int framesVisualId[MAX_ANIM_FRAMES];
} sAnimation;

int GetAnimationIndex(int value, sAnimation *anims, int nAnims)
{
	for (int i=0; i<nAnims; i++)
		if (anims[i].nIndex == value)
			return i;

	return -1;
}

//////////////////////////////////////////////////////////////////////////

typedef struct
{
	int id;
	char *symbol;
} sGameString;

#define GAME_STRINGS_MIN_SIZE		1000
sGameString *gameStrings = NULL;
int nGameStrings = 0;
int sizeGameStrings = 0;

void ResizeGameStrings()
{
	if (nGameStrings == sizeGameStrings)
	{
		sizeGameStrings += GAME_STRINGS_MIN_SIZE;
		sGameString *tmp = new sGameString[sizeGameStrings];
		memcpy(tmp, gameStrings, sizeof(sGameString) * sizeGameStrings);

		delete[] gameStrings;

		gameStrings = tmp;		
	}
}

int GetStringId(const char *str)
{
	for (int i=nGameStrings - 1; i >= 0; --i)
	{
		if (stricmp(gameStrings[i].symbol, str) == 0)
			return gameStrings[i].id;
	}

	return 0;
}

void GetGameStrings(const char* fname)
{
	FILE *f;

	nGameStrings = 0;
	sizeGameStrings = 0;
	gameStrings = NULL;

	f = fopen(fname, "r");
	if (!f)
		return;

	sizeGameStrings = GAME_STRINGS_MIN_SIZE;
	gameStrings = new sGameString[sizeGameStrings];

	char	line[1024];
	char	txt[256];
	int		val;
	while (1)
	{
		fgets(line, 1023, f);
		if (feof(f))
		{
			break;
		}

		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		parserBegin(p, " \t,;()");

		if (parserSkip("STRING_"))
		{
			ResizeGameStrings();

			parserGetInt(&val);
			gameStrings[nGameStrings].id = val;
			parserGetString(txt);
			gameStrings[nGameStrings].symbol = strdup(txt);

			nGameStrings++;
			
		}
	}

	fclose(f);
}

void CleanGameStrings()
{
	for (int i=nGameStrings - 1; i >= 0; --i)
	{
		free(gameStrings[i].symbol);
	}

	delete[] gameStrings;
}

//////////////////////////////////////////////////////////////////////////

class TextureValidator
{
public:
	TextureValidator(int n,int m):m_nbrTextures(n),m_currentMaterial(m){};

	void Check(int i) const
	{
		// if(i < 0 || i > m_nbrTextures)
			// fprintf(stderr,"** Invalid Texture Id: Material %d Texture %d max: %d\n",m_currentMaterial+1,i,m_nbrTextures);
	}


private:
	const int m_nbrTextures;
	const int m_currentMaterial;
};

bool isStringEmpty(char * stringToTrim)
{

	while (*stringToTrim != 0)
	{
		if (*stringToTrim != ' ' && 
			*stringToTrim != '\t' 
			)
		{
			return false;		
		}
		stringToTrim++;
	}
	return true;		
}

int decodeTextureIndex ( int idx, TextureInfo *textureIndexes, int *texturesUsageStatistics)
{
	if (idx > 0)
	{
		bool indexFound=false;
		for (int textureIndex=0 ; 
			textureIndex < MAX_TEXTURES ;
			textureIndex++)
		{
			if ((textureIndexes[textureIndex].materialIndex) == idx)
			{
				idx = textureIndex;
				texturesUsageStatistics[textureIndex]++;
				indexFound = true;
				break;
			}
		}
		if (!indexFound)
		{
			printf("TrackExp:Texture index ""%d"" not found in the config file! Track compilation aborted!\n",idx);
			return DECODE_TEXTURE_ERROR_CODE;
		}
		return idx;
	}
	else
	{
		//No texture
		return (idx-1);
	}

}
//int * LoadTextureIndexes(char * FileName)
//{
//	char	txt[512];
//	int * result = new int[MAX_TEXTURES];
//	FILE*	fin = fopen( FileName, "rt" );
//
//	if (fin == NULL)
//	{
//		printf("== FILE : %s == ", FileName );
//		printf("Config file not found!\n\n");
//		return NULL;
//	}
//
//	// find TEXTURES
//	txt[0]=0;
//	while (strcmp( txt, "[TRACK_TEXTURES]" ) )
//		fscanf( fin, "%s\n", txt );
//	int text_nb=0;
//	int temp1 = 0;
//	int temp2 = 0;
//	char indexString[20];
//	while (1)
//	{
//
//		fscanf( fin, "%s %s %d %d\n",indexString , txt, &temp1,&temp2);
//
//		if (strcmp( indexString, "[END]" ) == 0)
//			break;
//		if (!isStringEmpty(indexString))
//		{
//			sscanf(indexString,"%d" ,&result[text_nb]);
//			text_nb++;
//		}
//	}
//
//	fclose( fin );
//	return result;
//}

#define ARGUMENT_INDEX_ASE					1
#define ARGUMENT_INDEX_ASE_BG				2
#define ARGUMENT_INDEX_CONFIG				3
#define ARGUMENT_INDEX_NAME					4
#define ARGUMENT_INDEX_OUT_DIRECTORY		5
#define ARGUMENT_INDEX_TEXTURES_DIRECTORY	6
#define ARGUMENT_INDEX_REVERSE				7
#define ARGUMENT_INDEX_LANG					8
#define ARGUMENT_REQUIRED_NO				9

#define ARGUMENT_INDEX_FLAGS				9
#define ARGUMENT_NO							10

#define TRACK_FILE_EXT						".trk"
#define TRACK_BG_FILE_EXT					".bg"
#define CONFIG_FILE_EXT						".cfg"

//#define FLAG_FLIP_X							(1 << 6)
//#define FLAG_FLIP_Y							(1 << 7)
//#define FLAG_FLIP_XY						(FLAG_FLIP_X | FLAG_FLIP_Y)
//#define TEXTURE_FLIP_MASK					0xC0
//#define TEXTURE_MAT_MASK					0x3F
#define TEXTURE_MAT_MASK					0xFF

int ExportConfigFile(const char *cfgFile, const char *outFile, const char *texturesFile, const char *texturesFolder)
{
	// ===============================================
	// CONFIG
	// ===============================================

	FILE	*fin;
	char	txt[512];
	char	txt2[512];

	fin = fopen(cfgFile, "rt" );

	if (fin == NULL)
	{
		printf("Error creating file : %s\n", cfgFile);
		return -1;
	}

	FILE	*fTextures;
	fTextures = fopen(texturesFile, "wt" );

	if (fTextures == NULL)
	{
		printf("Error creating file : %s\n", texturesFile);
		return -1;
	}

	*txt = '\0';

	char	line[1024];

	//////////////////////////////////////////////////////////////////////////
	// find DAYTIME
	while (strcmp( txt, "[DAYTIME]" ) )
		fscanf( fin, "%s\n", txt );
	int		daytime;
	fscanf( fin, "%d\n", &daytime );

	//////////////////////////////////////////////////////////////////////////
	// find SKY
	while (strcmp( txt, "[SKY]" ) )
		fscanf( fin, "%s\n", txt );
	char	sky[128];
	int		sky_offset_x;
	int		sky_offset_y;

	fscanf( fin, "%s\n", sky );
	fscanf( fin, "%d\n", &sky_offset_y );
	fscanf( fin, "%d\n", &sky_offset_x );

	fprintf(fTextures, "%s\n", sky);
	//////////////////////////////////////////////////////////////////////////

	while (strcmp( txt, "[SUN]" ) )
		fscanf( fin, "%s\n", txt );
	char	sunFileName[128];
	char	sunRefFileName[128];
	char	sunMask[128] = "";

	fgets(line, 1023, fin);
	char *p = trimLine(line);		
	parserBegin(p, " \t,;");
	int index = 0;
	while (parserGetString(txt))
	{
		switch (index)
		{
		case 0:
			strcpy(sunFileName, txt);
			break;
		case 1:
			strcpy(sunMask, txt);
			break;
		}
		index++;
	}

	//fscanf( fin, "%s\n", sunFileName );
	fscanf( fin, "\n%s\n", sunRefFileName );
	
	int sunAngle,sunHeight;
	fscanf( fin, "%d %d %d\n", &sunAngle, &sunHeight);

	if (sunMask[0] != '\0')
		fprintf(fTextures, "%s -m %s\n", sunFileName, sunMask);
	else
		fprintf(fTextures, "%s\n", sunFileName);
	fprintf(fTextures, "%s\n", sunRefFileName);

	//////////////////////////////////////////////////////////////////////////
	// find FOG
	while (strcmp( txt, "[FOG]" ) )
		fscanf( fin, "%s\n", txt );
	int		fog[3];
	fscanf( fin, "%d %d %d\n", &fog[0], &fog[1], &fog[2] );

	//////////////////////////////////////////////////////////////////////////
	// find ENVMAP
	while (strcmp( txt, "[ENVMAP]" ) )
		fscanf( fin, "%s\n", txt );
	char	envmap1[128];
	fscanf( fin, "%s\n", envmap1 );
	char	envmap2[128];
	fscanf( fin, "%s\n", envmap2 );

	unsigned char	alphaBufferTrack[256];
	memset(alphaBufferTrack, 0, 256);
	//////////////////////////////////////////////////////////////////////////
	// find TEXTURES
	while (strcmp( txt, "[TRACK_TEXTURES]" ))
		fscanf( fin, "%s\n", txt );

	//int		text_nb = 0; // use global var
	//char	text[MAX_TEXTURES][128];
	int		text_slow[MAX_TEXTURES];
	int		text_tile[MAX_TEXTURES];	
	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		if (strcmp(p, "[END]" ) == 0)
			break;

		alphaBufferTrack[text_nb] = 0xFF;

		parserBegin(p, " \t,;");
		int index = 0;
		while (parserGetString(txt))
		{
			switch (index)
			{
			// material index
			case 0:
				sscanf(txt, "%d", &texturesInfo[text_nb].materialIndex);
				break;
			// texture name
			case 1:
				strcpy(text[text_nb], txt);
				break;
			// tiling
			case 2:
				sscanf(txt, "%d", &text_tile[text_nb]);
				break;
			// friction
			case 3:
				sscanf(txt, "%d", &text_slow[text_nb]);
				break;

			case 4:
				sscanf(txt, "%d", &alphaBufferTrack[text_nb]);
				break;

			}
			index++;
		}
		
		texturesInfo[text_nb].name = &text[text_nb][0];
		int masterIndex = SearchMasterTexture(texturesInfo, text_nb);
		if (masterIndex >= 0)
		{
			texturesInfo[text_nb].masterName = texturesInfo[masterIndex].name;			
			texturesInfo[text_nb].palId = texturesInfo[masterIndex].nPals;
			texturesInfo[masterIndex].nPals++;

			fprintf(fTextures, "%s -p %d %s\n", text[text_nb], texturesInfo[text_nb].palId, texturesInfo[text_nb].masterName);
		}
		else
		{
			fprintf(fTextures, "%s\n", text[text_nb]);
		}

		text_nb++;
	}

	//////////////////////////////////////////////////////////////////////////
	// find config
	while (strcmp( txt, "[MATERIAL]" ) )
		fscanf( fin, "%s\n", txt );

	int		mat_nb = 0;
	//int		mat[128][38];
	int		mat[128][40];
	memset( mat, 0, 4*128*38 );
	while (1)
	{
		fscanf( fin, "%s", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;
		if ((txt[0] < '0') || (txt[0] > '9')) 
		{
			// jump line
			char	c = 0;
			while (c != '\n')
				fread( &c, 1, 1, fin );
			continue;
		}

		if (bReverse == false)
		{
			// get section config (8 quads textures) + 1 for TOP
			sscanf( txt, "%d", &mat[mat_nb][0] );
			int index = 1;

			fgets(line, 1023, fin);
			parserBegin(line, " \t,;");
			while (index < 22 && parserGetString(txt))
			{
				if (!stricmp(txt, "FLIPX"))
					//mat[mat_nb][index-1] |= FLAG_FLIP_X;
					mat[mat_nb][38] |= (1 << (index - 1));
				else if (!stricmp(txt, "FLIPY"))
					//mat[mat_nb][index-1] |= FLAG_FLIP_Y;
					mat[mat_nb][39] |= (1 << (index - 1));
				else if (!stricmp(txt, "FLIPXY"))
				{
					//mat[mat_nb][index-1] |= FLAG_FLIP_XY;
					mat[mat_nb][38] |= (1 << (index - 1));
					mat[mat_nb][39] |= (1 << (index - 1));
				}
				else
				{
					if (index == 8)
						index = 22;
					if (sscanf(txt, "%d", &mat[mat_nb][index]) == 1)
						index++;
				}
			}

			/*fscanf( fin, "%d %d %d %d %d %d %d %d\n",	&mat[mat_nb][1], &mat[mat_nb][2], &mat[mat_nb][3],
				&mat[mat_nb][4], &mat[mat_nb][5], &mat[mat_nb][6], 
				&mat[mat_nb][7], &mat[mat_nb][22] );
			*/
			// get fence config (4 fence 4 height)
			fscanf( fin, "%d %d %d %d %d %d %d %d\n",	&mat[mat_nb][8], &mat[mat_nb][9], &mat[mat_nb][10], &mat[mat_nb][11],
				&mat[mat_nb][12], &mat[mat_nb][13], &mat[mat_nb][14], &mat[mat_nb][15] );

			// get BILLBOARD config (2 billboards + position I (interior) M (middle) E (exterior) + high + transparency N (none) M (mid) A (add)
			fscanf( fin, "%d %c %d %c %d %c %d %c\n",	&mat[mat_nb][16], &mat[mat_nb][17], &mat[mat_nb][23], &mat[mat_nb][20], 
				&mat[mat_nb][18], &mat[mat_nb][19], &mat[mat_nb][24], &mat[mat_nb][21] );

			// get 2nd BILLBOARD config (2 billboards + position I (interior) M (middle) E (exterior) + high + transparency N (none) M (mid) A (add) + startx + endx
			fscanf( fin, "%d %c %d %c %d %d %d %c %d %c %d %d\n",	&mat[mat_nb][25], &mat[mat_nb][26], &mat[mat_nb][27], &mat[mat_nb][28], &mat[mat_nb][29], &mat[mat_nb][30], 
				&mat[mat_nb][31], &mat[mat_nb][32], &mat[mat_nb][33], &mat[mat_nb][34], &mat[mat_nb][35], &mat[mat_nb][36] );
		}
		else
		{
			// get section config (8 quads textures) + 1 for TOP
			sscanf( txt, "%d", &mat[mat_nb][7] );
			fscanf( fin, "%d %d %d %d %d %d %d %d\n",	&mat[mat_nb][6], &mat[mat_nb][5], &mat[mat_nb][4],
				&mat[mat_nb][3], &mat[mat_nb][2], &mat[mat_nb][1], 
				&mat[mat_nb][0], &mat[mat_nb][22] );
			// get fence config (4 fence 4 height)
			fscanf( fin, "%d %d %d %d %d %d %d %d\n",	&mat[mat_nb][14], &mat[mat_nb][15], &mat[mat_nb][12], &mat[mat_nb][13],
				&mat[mat_nb][10], &mat[mat_nb][11], &mat[mat_nb][8], &mat[mat_nb][9] );

			// get BILLBOARD config (2 billboards + position I (interior) M (middle) E (exterior) + high + transparency N (none) M (mid) A (add)
			fscanf( fin, "%d %c %d %c %d %c %d %c\n",	&mat[mat_nb][18], &mat[mat_nb][19], &mat[mat_nb][24], &mat[mat_nb][21], 
				&mat[mat_nb][16], &mat[mat_nb][17], &mat[mat_nb][23], &mat[mat_nb][20] );


			// get 2nd BILLBOARD config (2 billboards + position I (interior) M (middle) E (exterior) + high + transparency N (none) M (mid) A (add) + startx + endx
			fscanf( fin, "%d %c %d %c %d %d %d %c %d %c %d %d\n",	&mat[mat_nb][31], &mat[mat_nb][32], &mat[mat_nb][33], &mat[mat_nb][34], &mat[mat_nb][35], &mat[mat_nb][36],
				&mat[mat_nb][25], &mat[mat_nb][26], &mat[mat_nb][27], &mat[mat_nb][28], &mat[mat_nb][29], &mat[mat_nb][30] );
		}

		// get envmap idx
		fscanf( fin, "%d\n", &mat[mat_nb][37] );
		//printf("TrackExp:Envmap mat nb %d, idx %d\n",mat_nb,mat[mat_nb][37]);

		const TextureValidator validator(text_nb,mat_nb);

		for(int i=0;i<8;i++)
			validator.Check(mat[mat_nb][i]);

		validator.Check(mat[mat_nb][22]);

		validator.Check(mat[mat_nb][ 8]);
		validator.Check(mat[mat_nb][10]);
		validator.Check(mat[mat_nb][12]);
		validator.Check(mat[mat_nb][14]);

		validator.Check(mat[mat_nb][16]);
		validator.Check(mat[mat_nb][18]);

		validator.Check(mat[mat_nb][25]);
		validator.Check(mat[mat_nb][31]);

		validator.Check(mat[mat_nb][37]);

		mat_nb++;
	}

	// find ANIMATIONS
	bool bAnimFound = false;

	sAnimation anims[MAX_ANIMS];
	int nAnims = 0;

	memset(anims, 0, MAX_ANIMS * sizeof (sAnimation));

	int ms;
	int maxFrame;
	int value;
	int ret;

	bool bNextTextureFlags = false;

	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);
		if (!p || !*p)
			continue;

		if (strcmp(p, "[BG_TEXTURES]" ) == 0)
			break;

		if (strcmp(p, "[BG_TEXTURES_FLAGS]" ) == 0)
		{
			bNextTextureFlags = true;
			break;
		}

		if (!bAnimFound)
		{
			if (strncmp(p, "[ANIM", 5) == 0)
			{
				bAnimFound = true;
				sscanf(p, "[ANIM%d", &value);

				anims[nAnims].nIndex = value;
				maxFrame = 0;
			}
		}
		else
		{
			if (strcmp(p, "[END]" ) == 0)
			{
				bAnimFound = false;
				if (maxFrame > anims[nAnims].nFrames)
					printf("\tWarning: Anim %d, not all frames have been entered\n", anims[nAnims].nIndex);
				nAnims++;
				continue;
			}

			ret = sscanf(p, "%d%s%d", &value, txt, &ms);

			if (ret != 3)
			{
				printf("Unexpected file structure %s\n", p);
				return -1;
			}

			if (value < 0)
			{
				printf("\tWarning: Anim %d, frame %d : frame value must be > 0\n", anims[nAnims].nIndex, value);
				continue;
			}

			if (ms <= 0)
				printf("\tWarning: Anim %d, frame %d : ms value must be >= 0\n", anims[nAnims].nIndex, value); 

			anims[nAnims].nFrames++;
			strcpy(anims[nAnims].textures[value], txt);
			anims[nAnims].framesMs[value] = ms;

			if (maxFrame < value)
				maxFrame = value;
		}
	}

	int nTextureFlags = 0;
	char szTextureFlags[64][128];

	if (bNextTextureFlags)
	{
		// Read background textures flags

		while (1)
		{
			fgets(line, 1023, fin);
			if (!line)
			{
				printf("Unexpected end of file\n");
				break;
			}

			char *p = trimLine(line);
			if (!p || !*p)
				continue;

			if (strcmp(p, "[END]" ) == 0)
				break;

			sscanf(p, "%s", szTextureFlags[nTextureFlags]);
			nTextureFlags++;
		}

		while (strcmp( txt, "[BG_TEXTURES]" ) )
			fscanf( fin, "%s\n", txt );
	}

	// Read background textures

	char	textbg[128][128];
	int		maxMaterialIndex = 0;
	bool	bAnimsUsed = false;
	unsigned char	alphaBuffer[256];
	memset(alphaBuffer, 0, 256);

	// Texture flags array
	int nTextureFlagsEntries = 0;
	unsigned char textureFlags[128][2];
	for (int i=0; i<128; i++)
		memset(textureFlags[i], 0, 2);

	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);
		if (!p || !*p)
			continue;

		parserBegin(p, " \t,;");
		int index = 0;
		int alpha = 0xFF;
		bool bCheckedAlpha = false;
		bool bHasTextureFlags = 0;

		texturesInfoBg[textbg_nb].maskName = NULL;

		while (parserGetString(txt2))
		{
			switch (index)
			{
			// material index
			case 0:
				strcpy(txt, txt2);
				break;
			case 1:
				strcpy(textbg[textbg_nb], txt2);
				break;
			default:
				{
					// check if we have a texture flag
					int textureFlagIndex = -1;
					for (int j=0; j<nTextureFlags; j++)
					{
						if (stricmp(szTextureFlags[j], txt2) == 0)
						{
							textureFlagIndex = j;
							break;
						}
					}

					if (textureFlagIndex < 0)
					{
						if (!bCheckedAlpha)
						{
							int ret = sscanf(txt2, "%d", &alpha);
							if (ret == 0)
							{
								if (CheckTexture(texturesFolder, txt2))
									texturesInfoBg[textbg_nb].maskName = strdup(txt2);
							}

							bCheckedAlpha = (ret == 1) || (texturesInfoBg[textbg_nb].maskName != NULL);
						}
						else
						{
							printf("Warning: [BG_TEXTURES] unknown parameter : %s\n", txt2);
						}
					}
					else
					{
						textureFlags[nTextureFlagsEntries][0] = textbg_nb;
						textureFlags[nTextureFlagsEntries][1] |= (1 << textureFlagIndex);
						bHasTextureFlags = true;
					}
				}
				break;
			}

			index++;

			// if animation, skip parsing
			if (strncmp(textbg[textbg_nb], "[ANIM", 5) == 0)
				break;
		}

		if (bHasTextureFlags)
			nTextureFlagsEntries++;

		alphaBuffer[textbg_nb] = (unsigned char) alpha;

		//sscanf( p, "%s %s\n", txt, textbg[textbg_nb]);
		if (strcmp( txt, "[END]" ) == 0)
			break;

		if (strncmp(textbg[textbg_nb], "[ANIM", 5) == 0)
		{
			p = strstr(p, "[ANIM") + 5;
			sscanf(p, "%d", &value);

			int index = GetAnimationIndex(value, anims, nAnims);

			if (index < 0)
				printf("Wrong animation index %s %s\n", txt, textbg[textbg_nb]);
			else
			{
				sscanf(txt, "%d" ,&texturesInfoBg[textbg_nb].materialIndex);
				anims[index].textureBgId = textbg_nb;
			}

			texturesInfoBg[textbg_nb].name = &emptyString[0];

			textbg_nb++;

			bAnimsUsed = true;

			continue;
		}
		
		//if (!isStringEmpty(txt))
		//{
		//	sscanf(txt, "%d", &textureBgIndexes[textbg_nb]);
		//}

		int materialId;
		sscanf(txt, "%d" ,&materialId);
		if (maxMaterialIndex < materialId)
			maxMaterialIndex = materialId;
		if (!AddBgTexture(materialId, textbg[textbg_nb], texturesFolder, texturesInfoBg, textbg_nb, fTextures))
			return -1;

		//int textureWidth = -1;
		//int textureHeight = -1;

		//GetRealTextureWidth(texturesFolder, textbg[textbg_nb], &textureWidth, &textureHeight);

		//texturesInfoBg[textbg_nb].textureWidth = textureWidth;
		//texturesInfoBg[textbg_nb].textureHeight = textureHeight;
		//
		//if (!isPowerOfTwo(textureWidth) || !isPowerOfTwo(textureHeight))
		//	printf("%d. %s width, height: %d, %d\n", textbg_nb, textbg[textbg_nb], textureWidth, textureHeight);

		//sscanf(txt, "%d" ,&texturesInfoBg[textbg_nb].materialIndex);

		//texturesInfoBg[textbg_nb].name = &textbg[textbg_nb][0];
		//int masterIndex = SearchMasterTexture(texturesInfoBg, textbg_nb);
		//if (masterIndex >= 0)
		//{
		//	texturesInfoBg[textbg_nb].masterName = texturesInfoBg[masterIndex].name;			
		//	texturesInfoBg[textbg_nb].palId = texturesInfoBg[masterIndex].nPals;
		//	texturesInfoBg[masterIndex].nPals++;

		//	fprintf(fTextures, "%s -p %d %s\n", textbg[textbg_nb], texturesInfoBg[textbg_nb].palId, texturesInfoBg[textbg_nb].masterName);
		//}
		//else
		//{
		//	fprintf(fTextures, "%s\n", textbg[textbg_nb]);
		//}

		//textbg_nb++;

		//if (textbg_nb > MAX_TEXTURES)
		//{
		//	printf("BgExp: Too many textures in the config file, check if [END] is specified.");
		//	return -1;
		//}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Add textures from animations
	if (bAnimsUsed)
	{
		for (int i=0; i<nAnims; i++)
		{
			for (int j=0; j<anims[i].nFrames; j++)
			{
				int id = SearchBgTexture(anims[i].textures[j], texturesInfoBg, textbg_nb);			
				int materialId;
				if (id < 0)
				{
					++maxMaterialIndex;
					materialId = maxMaterialIndex;

					if (!AddBgTexture(materialId, anims[i].textures[j], texturesFolder, texturesInfoBg, textbg_nb, fTextures))
						return -1;

					id = textbg_nb - 1;
				}
				else
				{
					materialId = texturesInfoBg[id].materialIndex;
				}

				// !!! store texture id, not the material ID, todo: make it clearer
				anims[i].framesVisualId[j] = id; // materialId;

				if (j == 0)
				{
					//texturesInfoBg[anims[i].textureBgId].name = &texturesInfoBg[id].name[0];
					texturesInfoBg[anims[i].textureBgId].textureWidth = texturesInfoBg[id].textureWidth;
					texturesInfoBg[anims[i].textureBgId].textureHeight = texturesInfoBg[id].textureHeight;
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	// find FRICTION
	while (strcmp( txt, "[FRICTION]" ) )
		fscanf( fin, "%s\n", txt );
	int		friction[4];
	fscanf( fin, "%d %d %d %d\n", &friction[0], &friction[1], &friction[2], &friction[3] );

	// find RADAR
	while (strcmp( txt, "[RADAR]" ) )
		fscanf( fin, "%s\n", txt );
	int		radar;
	fscanf( fin, "%d\n", &radar);

	//	find SHOWMINIMAP
	while (strcmp( txt, "[SHOWMINIMAP]" ) )
		fscanf( fin, "%s\n", txt );

	int		minimapshow_nb = 0;
	int		minimapshow[128][2];
	while (1)
	{
		fscanf( fin, "%s %d %d\n",txt, &(minimapshow[minimapshow_nb][0]),&(minimapshow[minimapshow_nb][1]));
		if (strcmp( txt, "[END]" ) == 0)
			break;
		minimapshow_nb++;
	}

	// find SKYLUMINANCE
	while (strcmp( txt, "[SKYLUMINANCE]" ) )
		fscanf( fin, "%s\n", txt );
	int		skyluminance;
	fscanf( fin, "%d\n", &skyluminance);


	// find TRAFIC
	while (strcmp( txt, "[TRAFIC_CARS]" ) )
		fscanf( fin, "%s\n", txt );

	int		traficcar_nb = 0;
	char	traficcar[128][128];
	while (1)
	{
		fscanf( fin, "%s %s\n", txt, traficcar[traficcar_nb] );
		if (strcmp( txt, "[END]" ) == 0)
			break;
		traficcar_nb++;
	}

	while (strcmp( txt, "[TRAFIC_SECTION]" ) )
		fscanf( fin, "%s\n", txt );

	int		traficsection_nb = 0;
	int		traficsection[128][5];
	while (1)
	{
		fscanf( fin, "%s", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;
		if ((txt[0] < '0') || (txt[0] > '9')) 
		{
			// jump line
			char	c = 0;
			while (c != '\n')
				fread( &c, 1, 1, fin );
			continue;
		}
		sscanf( txt, "%d", &traficsection[traficsection_nb][0] );
		fscanf( fin, "%d %d %d %d\n",	&traficsection[traficsection_nb][1], &traficsection[traficsection_nb][2],
			&traficsection[traficsection_nb][3], &traficsection[traficsection_nb][4] );

		traficsection_nb++;
	}


	while (strcmp( txt, "[START_POSITION]" ) )
		fscanf( fin, "%s\n", txt );
	int start_position = 0;
	fscanf( fin, "%s", txt );
	sscanf( txt, "%d", &start_position );



	while (strcmp( txt, "[CHECKPOINT]" ) )
		fscanf( fin, "%s\n", txt );

	int		checkpoints_nb = 0;
	int		checkpoints[128];
	while (1)
	{
		fscanf( fin, "%s", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;
		sscanf( txt, "%d", &checkpoints[checkpoints_nb] );
		checkpoints_nb++;
	}

	int			zones_nb = 0;
	short		zones[128];
	short		zonesId[128];
	int			val;
	bool		bStartZones = false;
	while (!feof(fin))
	{
		fgets(line, 1023, fin);
		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		if (strcmp(p, "[END]" ) == 0)
			break;

		if (bStartZones)
		{
			parserBegin(p, " \t,;");
			parserGetInt(&val);
			zones[zones_nb] = (short)val;
			parserGetString(txt);
			zonesId[zones_nb] = GetStringId(txt);

			zones_nb++;
		}

		if (!bStartZones && strcmp(p, "[ZONE NAMES]" ) == 0)
			bStartZones = true;
	}

	fclose( fin );

	FILE *fout = fopen( outFile, "wb" );
	if (fout == NULL)
	{
		printf("Error creating file : %s\n", outFile );
		return -1;
	}

	unsigned char	idx;
	char nameout[256];

	idx = daytime;
	fwrite( &idx, 1, 1, fout );

	idx = strlen( sky );
	fwrite( &idx, 1, 1, fout );
	fwrite( sky, idx, 1, fout );

	short offset;
	offset = short(sky_offset_x);
	fwrite(&offset, 2, 1, fout);
	offset = short(sky_offset_y);
	fwrite(&offset, 2, 1, fout);

	idx = strlen( sunFileName );
	fwrite( &idx, 1, 1, fout );
	fwrite( sunFileName, idx, 1, fout );

	idx = strlen( sunRefFileName );
	fwrite( &idx, 1, 1, fout );
	fwrite( sunRefFileName, idx, 1, fout );

	idx = sunAngle;
	fwrite( &idx, 1, 1, fout );

	idx = sunHeight;
	fwrite( &idx, 1, 1, fout );

	for (int i = 0; i < 3; i++)
	{
		idx = fog[i];
		fwrite( &idx, 1, 1, fout );
	}

	idx = strlen( envmap1 );
	fwrite( &idx, 1, 1, fout );
	fwrite( envmap1, idx, 1, fout );

	idx = strlen( envmap2 );
	fwrite( &idx, 1, 1, fout );
	fwrite( envmap2, idx, 1, fout );

	// Track textures
	idx = text_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < text_nb; i++)
	{
		if (texturesInfo[i].masterName)
			sprintf( nameout, "%s.pal%d.p", texturesInfo[i].masterName, texturesInfo[i].palId);
		else strcpy(nameout, texturesInfo[i].name);
		idx = strlen( nameout );
		fwrite( &idx, 1, 1, fout );
		fwrite( nameout, idx, 1, fout );
		//Dumps the textures, debug
		//printf("texture:%s\n",text[i]);
	}
	fwrite(alphaBufferTrack, 1, text_nb, fout);

	for (int i = 0; i < text_nb; i++)
	{
		idx = text_tile[i];
		fwrite( &idx, 1, 1, fout );
	}
	for (int i = 0; i < text_nb; i++)
	{
		idx = text_slow[i];
		fwrite( &idx, 1, 1, fout );
	}

	idx = mat_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < mat_nb; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			idx = mat[i][j];
			//int flag = 0;
			if ((j == 17) || (j == 19))
			{
				if (idx == 'I') // interior
					idx = 0x00;
				else if (idx == 'M') // middle
					idx = 0x01;
				else// if (idx == 'E') // exterior
					idx = 0x02;

				int	idx2;
				if (j == 17) idx2 = mat[i][20];
				if (j == 19) idx2 = mat[i][21];

				if (idx2 == 'M')	// mid trans
					idx |= 0x10;
				if (idx2 == 'A')	// add trans
					idx |= 0x20;
			}
			else
			{
				if ( j == 9 ||
					j== 11 ||
					j== 13 ||
					j== 15)
				{
					// fence heights
					idx--;
				}
				else
				{
					//flag = idx & TEXTURE_FLIP_MASK;
					idx = decodeTextureIndex(idx, texturesInfo, &texturesUsageStatistics[0]);
					if (idx == DECODE_TEXTURE_ERROR_CODE) return 1;
				}
			}

			//idx |= flag;

			fwrite( &idx, 1, 1, fout );
		}

		// add top texture
		if (mat[i][22] != -1)
		{
			idx = mat[i][22];
			idx = decodeTextureIndex(idx, texturesInfo, &texturesUsageStatistics[0]);
		}
		else
		{
			idx = decodeTextureIndex(-1, texturesInfo, &texturesUsageStatistics[0]);
		}

		if (idx == DECODE_TEXTURE_ERROR_CODE) return 1;

		fwrite( &idx, 1, 1, fout );
		// add billboard height
		idx = mat[i][23];
		fwrite( &idx, 1, 1, fout );
		// add billboard height
		idx = mat[i][24];
		fwrite( &idx, 1, 1, fout );

		// 2nd billboard
		for (int j = 25; j < 37; j++)
		{
			idx = mat[i][j];
			if ((j == 26) || (j == 32))
			{
				if (idx == 'I') // interior
					idx = 0x00;
				else if (idx == 'M') // middle
					idx = 0x01;
				else// if (idx == 'E') // exterior
					idx = 0x02;

				int	idx2;
				if (j == 26) idx2 = mat[i][28];
				if (j == 32) idx2 = mat[i][34];

				if (idx2 == 'M')	// mid trans
					idx |= 0x10;
				if (idx2 == 'A')	// add trans
					idx |= 0x20;
			}
			else
			{
				if ((j  == 25) || (j == 31))
				{
					//idx = idx-1;		// idx of texture are -1
					idx = decodeTextureIndex(idx, texturesInfo, &texturesUsageStatistics[0]);
					if (idx == DECODE_TEXTURE_ERROR_CODE) return 1;
				}
			}

			if ((j != 28) && (j != 34))
				fwrite( &idx, 1, 1, fout );
		}

		// add envmap texture idx
		idx = mat[i][37];
		//idx = decodeTextureIndex(idx,textureIndexes,&texturesUsageStatistics[0]);
		fwrite( &idx, 1, 1, fout );

		// flipping flags for road texture
		idx = mat[i][38];
		fwrite( &idx, 1, 1, fout );
		idx = mat[i][39];
		fwrite( &idx, 1, 1, fout );
	}

	// BG Textures
	idx = textbg_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < textbg_nb; i++)
	{
		if (texturesInfoBg[i].masterName)
			sprintf( nameout, "%s.pal%d.p", texturesInfoBg[i].masterName, texturesInfoBg[i].palId);
		else strcpy(nameout, texturesInfoBg[i].name);
		idx = strlen( nameout );
		fwrite( &idx, 1, 1, fout );
		fwrite( nameout, idx, 1, fout );

		//idx = strlen( textbg[i] );
		//fwrite( &idx, 1, 1, fout );
		//fwrite( textbg[i], idx, 1, fout );
	}

	fwrite(alphaBuffer, 1, textbg_nb, fout);

	// Textures flags
	idx = nTextureFlagsEntries;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < nTextureFlagsEntries; i++)
		fwrite(textureFlags[i], 1, 2, fout );

	//Animations
	if (bAnimsUsed)
		idx = nAnims;
	else idx = 0;

	fwrite( &idx, 1, 1, fout );
	if (bAnimsUsed)
	{
		short ms_short;
		for (int i=0; i<nAnims; i++)
		{
			idx = anims[i].nFrames;
			fwrite( &idx, 1, 1, fout );
			idx = anims[i].textureBgId;
			fwrite( &idx, 1, 1, fout );
			for (int j=0; j<anims[i].nFrames; j++)
			{
				idx = anims[i].framesVisualId[j];
				fwrite(&idx, 1, 1, fout);
				ms_short = (short) anims[i].framesMs[j];
				fwrite(&ms_short, 2, 1, fout);
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		idx = friction[i];
		fwrite( &idx, 1, 1, fout );
	}

	short radar_short = radar;
	fwrite( &radar_short, 2, 1, fout );

	unsigned char	minmap_start;
	unsigned char	minmap_end;
	idx = minimapshow_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < minimapshow_nb; i++)
	{
		minmap_start = minimapshow[i][0];
		minmap_end = minimapshow[i][1];
		fwrite( &minmap_start, 1, 1, fout );
		fwrite( &minmap_end, 1, 1, fout );
	}

	unsigned char skyluminance_char = skyluminance;
	fwrite( &skyluminance_char, 1, 1, fout );

	idx = traficcar_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < traficcar_nb; i++)
	{
		idx = strlen( traficcar[i] );
		fwrite( &idx, 1, 1, fout );
		fwrite( traficcar[i], idx, 1, fout );
	}

	idx = traficsection_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < traficsection_nb; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			unsigned short	us = traficsection[i][j];
			fwrite( &us, 2, 1, fout );
		}
	}

	{
		unsigned short	us = start_position;
		fwrite( &us, 2, 1, fout );
	}

	idx = checkpoints_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < checkpoints_nb; i++)
	{
		unsigned short	us = checkpoints[i];
		fwrite( &us, 2, 1, fout );
	}

	idx = zones_nb;
	fwrite( &idx, 1, 1, fout );
	for (int i = 0; i < zones_nb; i++)
	{
		unsigned short sh = zones[i];
		fwrite( &sh, 2, 1, fout );
		sh = zonesId[i];
		fwrite( &sh, 2, 1, fout );
	}
	
	fclose( fout );

	return 0;
}

short* ExportTrack(const char *inFile, const char *outFile, int &vertexNb)
{
	FILE*	fin = fopen(inFile, "rt" );

	if (fin == NULL)
	{
		printf("Error opening file : %s\n", inFile);
		return NULL; //return -1;
	}

	// find mesh
	char	txt[512];
	int		nbv, tmp, nbf;
	txt[0] = 0;

	while (strcmp( txt, "*MESH" ) )
		fscanf( fin, "\t%s\t{\n", txt );

	fscanf( fin, "\t%s\t%d\n", txt, &tmp );
	fscanf( fin, "\t%s\t%d\n", txt, &nbv );
	fscanf( fin, "\t%s\t%d\n", txt, &nbf );
	fscanf( fin, "\t%s\t{\n", txt );

	vertexNb = nbv;


	if (nbv % SECTION_VERT)
	{
		printf("== FILE : %s == ", inFile);
		printf("Error number of vertex incorrect, should be %%%d\n\n", SECTION_VERT );
		return NULL; //return -1;
	}

	short	*data = (short*)malloc( 3 * 2 * nbv );
	short	*p = data;
	short	n = nbv;

	while (n--)
	{
		float		x, y, z;

		fscanf( fin, "\t%s\t%d\t%f\t%f\t%f\n", txt, &tmp, &x, &y, &z );

		long		xl, yl, zl;

		//if (newTrack)
		//{
		//	// rax - the scale: meters; for precision, convert to cm

		//	xl = (long) (x * 100.0f);
		//	yl = (long) (y * 100.0f);
		//	zl = (long) (z * 100.0f);

		//	xl >>= 1;
		//	yl >>= 1;
		//	zl >>= 1;

		//	xl /= 2;
		//	yl /= 2;
		//	zl /= 2;
		//}
		//else
		{
			xl = (long)(x/8);	// 4x scale + 1cm = 2cm in engine
			yl = (long)(y/8);
			zl = (long)(z/8);
		}

		if ((abs(xl)&0xFFFF0000) || (abs(yl)&0xFFFF0000) || (abs(zl)&0xFFFF0000))
		{
			printf("Overflow -> two big !!!\n" );
			return NULL;
		}

		*p++ = (short)-xl;	// changing axis
		*p++ = (short)zl;
		*p++ = (short)yl;
	}

	fscanf( fin, "\t}\n", txt );
	fscanf( fin, "\t%s\t{\n", txt );

	//----------------------------------
	// FACES

	short	*tags = (short*)malloc( (nbv/SECTION_VERT)*2 );
	memset( tags, 0, (nbv/SECTION_VERT)*2 );
	n = nbf;

	int		idx_face = 0;
	while (n--)
	{
		char		str[256];
		int			a, b, c;
		int			idx;

		fscanf( fin, "\t*MESH_FACE\t%d:\tA:\t%d\tB:\t%d\tC:\t%d", &idx, &a, &b, &c );
		str[0] = 0;
		while (strcmp( str, "*MESH_MTLID" ))
			fscanf( fin, "%s", str );


		fscanf( fin, "\t%d", &idx );

		// 		bool indexFound=false;
		// 		for (int textureIndex=0 ; 
		// 			textureIndex < MAX_TEXTURES ;
		// 			textureIndex++)
		// 		{
		// 			if ((textureIndexes[textureIndex]-1) == idx)
		// 			{
		// 				idx = textureIndex;
		// 				indexFound = true;
		// 				break;
		// 			}
		// 		}
		// 		if (!indexFound)
		// 		{
		// 			printf("TrackExp:Texture index ""%d"" not found in the config file! Track compilation aborted!\n",idx);
		// 			return -1;
		// 		}
		/*
		int res = fscanf( fin, "\t%d", &idx );
		if(res==1)
		printf("readid= %d\n",idx);
		else
		printf("not read\n");
		*/

		int			id[ 3 ];

		id[0] = a;
		id[1] = b;
		id[2] = c;

		// sort
		int	swap = 1;
		while (swap)
		{
			swap = 0;
			for (int i = 0; i < (3-1); i++)
			{
				if (id[i] > id[i+1])
				{
					swap = id[i];
					id[i] = id[i+1];
					id[i+1] = swap;
					swap = 1;
				}
			}
		}

		// we need to find a face with 2 indexes in the current section
		// the smallest should be on the idx 0 of the section, and the second smallest 
		// should be on the idx 1 (idx of section go from 0 to (SECTION_VERT-1)
		if (((id[1] - id[0]) < SECTION_VERT) &&
			((id[0] % SECTION_VERT) == 0) && ((id[1] % SECTION_VERT) == 1))
		{
			// get section idx
			//			int	sec = id[0] / SECTION_VERT;
			int	sec = idx_face / ((SECTION_VERT-1)*2);
			tags[ sec ] = idx;
			//fprintf(stderr,"section %d id: %d\n",sec,idx);
		}

		idx_face++;
	}

	//--------------------------------

	fclose( fin );

	FILE*	fout = fopen(outFile, "wb" );
	if (fout == NULL)
	{
		printf("Error creating file : %s\n", outFile);
		return NULL;
	}

	unsigned short	version = 1;
	unsigned short	vertnb;

	// check if start is also end
	if (data[0] == data[nbv - SECTION_VERT])
		vertnb = (unsigned short)(nbv - SECTION_VERT);	// we remove the last section because it s the start
	else
		vertnb = (unsigned short)nbv;

	if (bReverse)
		version = 2;	// version 2 is for reverse track (so the bg obj can be re-indexed nb_section-idx)

	fwrite( &version, 2, 1, fout );
	fwrite( &vertnb, 2, 1, fout );
	//fwrite( data, 3*2*nbv, 1, fout );

	if (bReverse == false)
	{
		// normal way

		// flipping idx for a section
		for (int j = 0; j < vertnb/SECTION_VERT; j++)
		{
			fwrite( tags + j, 2, 1, fout );		// write tags

			for (int s = SECTION_VERT-1; s >= 0; s--)
			{
				fwrite( data + (j*SECTION_VERT + s)*3, 3*2, 1, fout );	// write verts flipped
			}
		}
	}
	else
	{
		// reverse the track !

		for (int j = ((vertnb/SECTION_VERT)-1); j >= 0; j--)
		{
			// start is idx 0
			int	k = j + 1;
			if (k > ((vertnb/SECTION_VERT)-1))
				k = 0;

			//			fwrite( tags + k, 2, 1, fout );		// write tags
			// dec tags
			//			int	k2 = j - 1;
			//			if (k2 < 0)
			//				k2 = ((vertnb/SECTION_VERT)-1);
			fwrite( tags + j, 2, 1, fout );		// write tags

			//			for (int s = SECTION_VERT-1; s >= 0; s--)
			//			{
			//				fwrite( data + (j*SECTION_VERT + s)*3, 3*2, 1, fout );	// write verts flipped
			//			}
			for (int s = 0 ; s < SECTION_VERT; s++)
			{
				fwrite( data + (k*SECTION_VERT + s)*3, 3*2, 1, fout );	// write verts flipped
			}
		}
	}

	fclose(fout);

	free( tags );
	//free( data );

	return data;
}

void CheckTextures(const char *textureDirectory)
{
	for (int i=0;i<text_nb;i++)
	{
		char fileName[512]=""; 
		sprintf(fileName, "%s\\%s.tga", textureDirectory, text[i]);

		if (texturesUsageStatistics[i]==0 )
		{			
			printf("TrackExp:Texture \"%s\" not used!\n",text[i]);
			//DeleteFileA(fileName);
		}
		else
		{
			if (!PathFileExists(fileName))
			{
				printf("TrackExp:Warning, file \"%s\" does not exists!\n",fileName);
			}
		}
	}
}

bool CheckTexture(const char *textureDirectory, const char *name)
{
	char fileName[512]=""; 
	sprintf(fileName, "%s\\%s.tga", textureDirectory, name);

	if (!PathFileExists(fileName))
	{
		printf("TrackExp: Warning, file \"%s\" does not exists!\n", fileName);
		return false;
	}

	return true;
}

void Clean()
{
	for (int i=0; i<textbg_nb; i++)
		if (texturesInfoBg[i].maskName)
			delete[] texturesInfoBg[i].maskName;

	CleanGameStrings();
}

int mFlags = 0;

int	main( int argc, char** argv )
{
	printf("Track exporter begins ...\n");
	if ((argc < ARGUMENT_REQUIRED_NO))
	{
		printf("Usage : TRACKEXP inputASEfilename inputBG_ASEfilename inputTXT outputName outputFolder pathToTextures normal/reverse pathTo_intl_Lang_arm.hpp\n");
		return -1;
	}

	if (argc > ARGUMENT_INDEX_FLAGS)
	{
		for (int i = ARGUMENT_INDEX_FLAGS; i<argc; ++i)
		{
			if (!stricmp(argv[i], "-pvrtc"))
				mFlags |= FLAG_USE_PVRTC;
		}
	}
     		
	memset(texturesUsageStatistics, 0, MAX_TEXTURES * sizeof (*texturesUsageStatistics));
	memset(texturesInfo, 0, MAX_TEXTURES * sizeof (*texturesInfo));
	memset(texturesInfoBg, 0, MAX_TEXTURES * sizeof (*texturesInfoBg));

	//int * textureIndexes = LoadTextureIndexes(argv[ARGUMENT_INDEX_CONFIG]);
	//if (textureIndexes==NULL)
	//	return -1;

	bReverse = false;
	text_nb = 0;

	if (strcmp(argv[ARGUMENT_INDEX_REVERSE], "reverse") ==0)
	{
		printf("TrackExp: making reverse track\n");
		bReverse = true;
	}
	else
	{
		printf("TrackExp: making normal track\n");
	}
	char name[256];
	char textureName[256];
	sprintf(name, "%s\\%s%s", argv[ARGUMENT_INDEX_OUT_DIRECTORY], argv[ARGUMENT_INDEX_NAME], CONFIG_FILE_EXT);
	sprintf(textureName, "%s\\%s.txt", argv[ARGUMENT_INDEX_TEXTURES_DIRECTORY], argv[ARGUMENT_INDEX_NAME]);

	GetGameStrings(argv[ARGUMENT_INDEX_LANG]);

	ExportConfigFile(argv[ARGUMENT_INDEX_CONFIG], name, textureName, argv[ARGUMENT_INDEX_TEXTURES_DIRECTORY]);

	sprintf(name, "%s\\%s%s", argv[ARGUMENT_INDEX_OUT_DIRECTORY], argv[ARGUMENT_INDEX_NAME], TRACK_FILE_EXT);
	int vertexDataNb;
	short *vertexData = ExportTrack(argv[ARGUMENT_INDEX_ASE], name, vertexDataNb);

	if (vertexData)
	{
		sprintf(name, "%s\\%s%s", argv[ARGUMENT_INDEX_OUT_DIRECTORY], argv[ARGUMENT_INDEX_NAME], TRACK_BG_FILE_EXT);
		ExportTrackBg(argv[ARGUMENT_INDEX_ASE_BG], name, texturesInfoBg, vertexData, vertexDataNb);

		CheckTextures(argv[ARGUMENT_INDEX_TEXTURES_DIRECTORY]);

		free(vertexData);
	}

	Clean();

	printf("Track exporter ends ...\n");

	return 0;
}
