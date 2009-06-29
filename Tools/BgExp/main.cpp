#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <math.h>



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
	unsigned char	idx0, idx1, idx2;	// low part of idx
	unsigned char	idx012;				// 2bits high part of idx + matidx (ii) |ii001122|
	unsigned char	u0, v0, u1, v1, u2, v2;

} OBJ_FACE;

typedef struct
{
	float	u, v;

} OBJ_TV;

#define MAX_TEXTURES	128
struct TextureInfo
{
	int materialIndex;
	int textureWidth;
	int textureHeight;
};

bool GetRealTextureWidthBmp(char *folderName, char *fileName, int *textureWidth, int *textureHeight)
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

bool GetRealTextureWidthTga(char *folderName, char *fileName, int *textureWidth, int *textureHeight)
{
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

	char imageName[512];
	FILE *f;

	sprintf(imageName, "%s%s.tga", folderName, fileName);
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

bool powOf2(int n)
{
	while (n > 0 && (n & 0x01) == 0)
		n >>= 1;

	return n == 1;
}

bool GetRealTextureWidth(char *folderName, char *fileName, int *textureWidth, int *textureHeight)
{
	if (GetRealTextureWidthTga(folderName, fileName, textureWidth, textureHeight))	
		return true;

	if (GetRealTextureWidthBmp(folderName, fileName, textureWidth, textureHeight))	
		return true;

	return false;
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

TextureInfo * LoadTextureInfo(char * FileName)
{
	char	txt[512];
	TextureInfo * result = new TextureInfo[MAX_TEXTURES];
	FILE*	fin = fopen( FileName, "rt" );

	if (fin == NULL)
	{
		printf("== FILE : %s == ", FileName );
		printf("Config file not found!\n\n");
		return NULL;
	}

	char *p = FileName;
	char TexturesDir[256]="";
	while (p != NULL && (p = findStr(p, "track")) != NULL)
	{
		int value = 0;
		int res = sscanf (p + 5, "%d\\", &value);

		if (res == 1)
		{
			sprintf(TexturesDir, "Textures\\Track%d\\", value);
			break;
		}

		p += 5;
	}

	// find TEXTURES
	txt[0]=0;
	while (strcmp( txt, "[BG_TEXTURES]" ) )
		fscanf( fin, "%s\n", txt );
	int text_nb=0;
	int temp1 = 0;
	int temp2 = 0;
	char indexString[20];
	char fileName[256];
	while (1)
	{
		fgets(txt,512,fin);

		int textureWidth = -1;
		int textureHeight = -1;

		//int readValues = sscanf( txt, "%s %s %d\n",indexString , fileName,&textureWidth);
		int readValues = sscanf( txt, "%s %s\n",indexString , fileName);

		if (strcmp( indexString, "[END]" ) == 0)
			break;
		if ( readValues == -1) 
			continue; //empty line , go ahead

		if ( readValues != 2)
		{
			printf("BgExp:Texture parameters for texture with index %d are incorrect !\n",text_nb);
			printf("BgExp:Terminating ...\n");
			return NULL;
		}
		else
		{
			GetRealTextureWidth(TexturesDir, fileName, &textureWidth, &textureHeight);

			result[text_nb].textureWidth = textureWidth;
			result[text_nb].textureHeight = textureHeight;
			if (!powOf2(textureWidth) || !powOf2(textureHeight))
				printf("%d. %s width, height: %d, %d\n", text_nb, fileName, textureWidth, textureHeight);
		}
		
		sscanf(indexString,"%d" ,&result[text_nb].materialIndex);
		text_nb++;
		if (text_nb>MAX_TEXTURES)
		{
			printf("BgExp: Too many textures in the config file, check if [END] is specified.");
			return NULL;
		}
	}

	fclose( fin );
	return result;
}
#define ARGUMENT_INDEX_CONFIG_FILE	4

int		main( int argc, char** argv )
{
	printf("BgExp starts...\n");
	if (argc != 5)
	{
		printf("Usage : BGEXP inputAS2filename outputBGfilename texturewidth configfile\n\n");
		return -1;
	}		

	TextureInfo * texturesInfoArray = LoadTextureInfo(argv[ARGUMENT_INDEX_CONFIG_FILE]);
	if (texturesInfoArray==NULL)
		return -1;

	//int		texturewidth = atoi( argv[3] );
// 	if (texturewidth > 128)
// 	{
// 		printf("Input texture size Error (must be <= 128)!\n");
// 		return -1;
// 	}


	//-----------------------------------------------
	// check if the files have the same creation time
	{
		bool	doit1 = true;
/* //LAngelov: Always create the tracks
		HANDLE	hfile = CreateFile( argv[1], GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (hfile != INVALID_HANDLE_VALUE)
		{
			BY_HANDLE_FILE_INFORMATION	fileinfo1;
			GetFileInformationByHandle( hfile, &fileinfo1 );
			CloseHandle( hfile );

			hfile = CreateFile( argv[2], GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hfile != INVALID_HANDLE_VALUE)
			{
				BY_HANDLE_FILE_INFORMATION	fileinfo2;
				GetFileInformationByHandle( hfile, &fileinfo2 );
				CloseHandle( hfile );

				if ((fileinfo1.ftLastWriteTime.dwLowDateTime == fileinfo2.ftCreationTime.dwLowDateTime) &&
					(fileinfo1.ftLastWriteTime.dwHighDateTime == fileinfo2.ftCreationTime.dwHighDateTime))
					doit1 = false;
			}
		}	
*/
		if (doit1 == false)
		{
			printf( "%s : Skipped (no changes)\n", argv[1] );
			exit(0);
		}
	}
	//-----------------------------------------------



	FILE*	fin = fopen( argv[1], "rt" );

	if (fin == NULL)
	{
		printf("== FILE : %s == ", argv[1] );
		printf("Input File Error!\n\n");
		return -1;
	}


	FILE*	fout = fopen( argv[2], "wb" );
	if (fout == NULL)
	{
		printf("== FILE : %s == ", argv[1] );
		printf("Error opening output file\n\n");
		return -1;
	}

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
		// get road index (number after _)
		//char*	p = strstr( name, "_" );
		char *p = strrchr(name, '_');
		if (p)
		{
			char*	p2 = p+1;
			while ((*p2 >= '0') && (*p2 <= '9'))
				p2++;
			*p2 = 0;
			index = atoi( p+1 ) / 9;	// section idx
		}
		else
		{
			index = 0xFFFF;	// no road index
		}

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
		
		if (nbv >= 256)
		{
			printf("== FILE : %s == ", argv[1] );
			printf("Input nb vertex Error (must be < 256)!\n\n");
			return -1;
		}

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

			//xl >>= 1;
			//yl >>= 1;
			//zl >>= 1;

			//xl /= 2;
			//yl /= 2;
			//zl /= 2;

			if ((abs(xl)&0xFFFF0000) || (abs(yl)&0xFFFF0000) || (abs(zl)&0xFFFF0000))
			{
				printf("== FILE : %s == ", argv[1] );
				printf("Overflow -> to big !!!\n\n" );
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
			printf("== FILE : %s == ", argv[1] );
			printf("Error : nb tface != nb face\n\n");
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

			pf->u0 = (unsigned char)((datatv[a].u * textureWidth) + 0.5f);
			pf->v0 = (unsigned char)((datatv[a].v * textureHeight) + 0.5f);
			pf->u1 = (unsigned char)((datatv[b].u * textureWidth) + 0.5f);
			pf->v1 = (unsigned char)((datatv[b].v * textureHeight) + 0.5f);
			pf->u2 = (unsigned char)((datatv[c].u * textureWidth) + 0.5f);
			pf->v2 = (unsigned char)((datatv[c].v * textureHeight) + 0.5f);

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
				printf("== FILE : %s == ", argv[1] );
				printf("Animation Error -> no TRANSLATION track on <%s> !!!\n\n", name );
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
					printf("== FILE : %s == ", argv[1] );
					printf("Overflow anim pos -> to big !!!\n\n" );
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
				printf("== FILE : %s == ", argv[1] );
				printf("Animation Error -> no ROTATION track on <%s> !!!\n\n", name );
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
		else
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


		//===================================
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

		fwrite( &header, sizeof(OBJ_BG_HEADER), 1, fout );
		fwrite( datav, nbv*3*2, 1, fout );
		fwrite( dataf, nbf*sizeof(OBJ_FACE), 1, fout );

		if (nb_frames != 0)
		{
			fwrite( frames, sizeof(OBJ_BG_FRAME)*nb_frames, 1, fout );
		}

		free( datatv );
		free( datav );
		free( dataf );

		// inc number of objects
		bg_header.nb_obj++;
	}

end:

	fclose( fin );

	// rewrite header
	fseek( fout, 0, SEEK_SET );
	fwrite( &bg_header, sizeof( BG_HEADER ), 1, fout );


	fclose(fout);





	//--------------------------------------------------------
	// set the same creation time for the 2 files (out = in)
	{
		HANDLE	hfile = CreateFile( argv[1], GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (hfile != INVALID_HANDLE_VALUE)
		{
			FILETIME	time;
			GetFileTime( hfile, NULL, NULL, &time );
			CloseHandle( hfile );

			hfile = CreateFile( argv[2], GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hfile != INVALID_HANDLE_VALUE)
			{
				SetFileTime( hfile, &time, &time, &time );
				CloseHandle( hfile );
			}
		}	
	}
	//---------------------------------------------------------



	delete texturesInfoArray;
	printf("BgExp ends\n");
	return 0;
}