#include <windows.h>
#include <stdio.h>
#include <io.h>

#include "defines.h"

#define USE_PVR_TEXTURES

#ifdef USE_PVR_TEXTURES
	#include "pvrtc.h"	
#endif //USE_PVR_TEXTURES

enum
{
	FLAG_USE_PVRTC = (1 << 0)
};

int mFlags = 0;

void fillTgaHeader(TGA_HEADER *tgah, unsigned char *data)
{
	unsigned char *d = data;
	tgah->idLength = *d++;
	tgah->colorMapType = *d++;
	tgah->code = *d++;
	tgah->palStart = (*d) | ((*(d+1)) << 8); d+=2;
	tgah->palLength = (*d) | ((*(d+1)) << 8); d+=2;
	tgah->palDepth = *d++;
	d += 4;
	tgah->x_res = (*d) | ((*(d+1)) << 8); d+=2;
	tgah->y_res = (*d) | ((*(d+1)) << 8); d+=2;
	tgah->twenty_four = *d++;
}

//#define RGB24TO16(c)	(((c->b << 4) & 0x0F00) | (c->g & 0x0F0) | (c->r >> 4))
inline	unsigned short	RGB24TO16( RGB24* c )
{
	// conv to 444
	unsigned short	col = (((c->b << 4) & 0x0F00) | (c->g & 0x0F0) | (c->r >> 4));

	// if not key color
	if (col != 0xF0F)
	{
		// get the 5th bits and save it in the unused higher 4b
		col = col | ((((c->b >> 1) & 0x04) | ((c->g >> 2) & 0x02) | ((c->r >> 3) & 0x01)) << 12);
	}
	else
	{
		// PUT F in the unused 4b
		col |= 0xF000;
	}
	return (col);
}

int findColorInPalette(unsigned short color,int currentNbPal,unsigned short *	palette)
{
	int idx;
	for (idx = 0; idx < currentNbPal; idx++)
	{
		if (palette[idx] == color)
			break;
	}
	return idx;
}

////////////////////////////////////////////////////////////////////////////////////////

bool IsColorUsed(int index, unsigned char *data, int size)
{
	unsigned char *d = data;
	while (size--)
	{
		if (*d == index)
			return true;
		++d;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////

void GetFolder(char* fileName, char *folderName)
{
	strcpy(folderName, fileName);	
	char *p = strrchr(folderName, '\\');

	if (p)
	{
		*p = '\0';
	}
	else
	{
		*folderName = '\0';
	}
}

int ConvertFile(char *namein, char *nameout, int autocol, char &mode, bool bPalOnly, char *maskName = NULL);

void ConvertFiles(char *inFile)
{
	char	folderName[256];
	GetFolder(inFile, folderName);

	FILE	*fIn = fopen(inFile, "rt");

	if (fIn == NULL)
	{
		printf("Error: couldn't open %s\n", inFile);
		return;
	}

	char	line[1024];
	char	namein[256];
	char	nameout[256];
	char	tmp[256];
	char	mode = '4';

	while (fgets(line, 1023, fIn))
	{
		sscanf(line, "%s", tmp);
		
		sprintf( namein, "%s\\%s.tga", folderName, tmp );
		mode = '4';

		char *p = line + strlen(tmp);
#ifdef USE_PVR_TEXTURES
		if (strstr(p, "-pvrtc"))
		{
			sprintf( nameout, "%s\\%s.rle", folderName, tmp );
			if (mFlags & FLAG_USE_PVRTC)
				ConvertFileToPVRTC(namein, nameout);
			else
				ConvertFile(namein, nameout, 1, mode, false);
		}
		else
#endif // USE_PVR_TEXTURES
		{
			char *q = strstr(p, "-p");
			char *r = strstr(p, "-m");

			if (!r && !q)
			{
				sprintf( nameout, "%s\\%s.rle", folderName, tmp );
				ConvertFile(namein, nameout, 1, mode, false);
			}
			else if (r && q)
			{
			}
			else if (q)
			{
				p = q + 2;
				int palId = 0;
				char imageName[256];
				sscanf(p, "%d %s", &palId, imageName);
				sprintf( nameout, "%s\\%s.pal%d.p", folderName, imageName, palId);
				ConvertFile(namein, nameout, 1, mode, true);
			}
			else
			{
				p = r + 2;
				char imageMask[256];
				char nameMask[256];
				sscanf(p, "%s", imageMask);
				sprintf( nameout, "%s\\%s.rle", folderName, tmp );
				sprintf( nameMask, "%s\\%s.tga", folderName, imageMask);
				ConvertFile(namein, nameout, 1, mode, false, nameMask);
			}
		}
	}
}

unsigned char *GetMaskData(char *namein, int width, int height)
{
	FILE*	fpin = fopen( namein, "rb" );
	if (fpin == NULL)
	{
		printf("== FILE : %s == ", namein );
		printf("Error opening input file\n\n");
		return NULL;
	}	

	TGA_HEADER	tgah;
	unsigned char data[TGA_HEADER_SIZE];
	fread(data, TGA_HEADER_SIZE, 1, fpin );
	fillTgaHeader(&tgah, data);

	if (tgah.x_res != width || tgah.y_res != height)
	{
		printf("Error: mask size is different from image size (%s)\n", namein);
		return NULL;
	}

	if (tgah.colorMapType == 0)
	{
		printf("Error: mask must be indexed (%s)\n", namein);
		return NULL;
	}

	int n = tgah.palDepth / 8;

	if (n != 3)
	{
		printf("== FILE : %s == ", namein );
		printf("Error TGA format has not 24bits palette depth\n\n");
		return NULL;
	}

	int			size = tgah.x_res * tgah.y_res;
	int			palSize = (tgah.palDepth * tgah.palLength / 8);
	unsigned char *pal = new unsigned char[palSize];
	fread(pal, 1, palSize, fpin);
	unsigned char *b = new unsigned char[size];
	fread(b, 1, size, fpin);

	unsigned char *maskData = new unsigned char[size];

	for (int i=0; i<size; i++)
	{
		int p = b[i];
		p *= 3;
		maskData[i] = (pal[p] + pal[p+1] + pal[p+2]) / 3;
	}

	delete[] b;
	delete[] pal;

	fclose(fpin);

	return maskData;
}

int SaveUncompressedFile(char *nameout, TGA_HEADER	*tgah, RGB24* rgbin, int half_height)
{
	RLE_HEADER_UNCOMPRESSED	rleh;
	rleh.rle = UNCOMPRESSED | half_height;
	rleh.width = tgah->x_res; 
	rleh.height = tgah->y_res; 
	rleh.datasize = tgah->x_res * tgah->y_res * sizeof(RGB24);

	FILE*	fpout = fopen( nameout, "wb" );
	if (fpout == NULL)
	{
		printf("== FILE : %s == ", nameout );
		printf("Error opening output file\n\n");
		return -1;
	}

	fwrite( &rleh, sizeof( RLE_HEADER_UNCOMPRESSED ), 1, fpout );
	fwrite( rgbin, rleh.datasize, 1, fpout );

	return 1;
}

int ConvertFile(char *namein, char *nameout, int autocol, char &mode, bool bPalOnly, char *maskName)
{
// TODO: rewrite
restart:
	FILE*	fpin = fopen( namein, "rb" );
	if (fpin == NULL)
	{
		printf("== FILE : %s == ", namein );
		printf("Error opening input file\n\n");
		return -1;
	}

	TGA_HEADER	tgah;
	unsigned char data[TGA_HEADER_SIZE];
	fread(data, TGA_HEADER_SIZE, 1, fpin );
	fillTgaHeader(&tgah, data);

	if (((tgah.twenty_four != 24) || (tgah.code != 2)) &&
		(tgah.colorMapType == 0))
	{
		printf("== FILE : %s == ", namein );
		printf("Error TGA format is not 24bits uncompressed or paletted\n");
		fclose(fpin);
		return -1;
	}

	int		half_height = 0;
	if (tgah.x_res != tgah.y_res)
	{
		if (tgah.x_res == tgah.y_res*2)
		{
			half_height = COMPRESS_HALF_HEIGHT_F;
		}
	}

	// read input data
	if (tgah.colorMapType != 0)
	{
		// Indexed
		int n = tgah.palDepth / 8;

		if (n != 3)
		{
			printf("== FILE : %s == ", namein );
			printf("Error TGA format has not 24bits palette depth\n\n");
			fclose(fpin);
			return -1;
		}

		RLE_HEADER8	rleh;		

		int			size = tgah.x_res * tgah.y_res;
		int			palSize = (tgah.palDepth * tgah.palLength / 8);
		unsigned char *pal = new unsigned char[palSize];
		fread(pal, 1, palSize, fpin);
		unsigned char *data = new unsigned char[size];
		fread(data, 1, size, fpin);

		fclose(fpin);

		rleh.datasize = size;

		int			nb_pal = tgah.palLength;
		int			nb_out = 0;

		rleh.rle = COMPRESS_PAL8;
		rleh.width = tgah.x_res; 
		rleh.height = tgah.y_res;

		// TODO: compress !!!!

		for (int i=0, j=0; i<palSize && j<tgah.palLength; j++, i+=n)
		{
			//int p = 0;
			//for (int k=0; k<n; k++)
			//{
			//	p |= pal[i++] << (k << 3);
			//}
			RGB24 color;
			color.r = pal[i];
			color.g = pal[i+1];
			color.b = pal[i+2];
			rleh.palette[j] = RGB24TO16(&color);

			if (rleh.palette[j] == 0xFF0F)
			{
				if (!IsColorUsed(j, data, size))
					rleh.palette[j] = 0;
			}
		}

		// write file
		FILE*	fpout = fopen( nameout, "wb" );
		if (fpout == NULL)
		{
			printf("== FILE : %s == ", nameout );
			printf("Error opening output file\n\n");			
			return -1;
		}

		if (bPalOnly) // only palette
		{
			unsigned char col = 0;
			fwrite(&col, 1, 1, fpout );
			fwrite(rleh.palette, 256 * 2, 1, fpout );
		}
		else
		{
			unsigned char *maskData = NULL;
			if (maskName)
			{
				rleh.rle |= IMAGE_MASK;
				maskData = GetMaskData(maskName, rleh.width, rleh.height);
			}

			fwrite( &rleh, sizeof( RLE_HEADER8 ), 1, fpout );
			fwrite( data, rleh.datasize, 1, fpout );

			if (maskData)
			{
				fwrite( maskData, rleh.datasize, 1, fpout );
			}
		}
		fclose(fpout);

		delete[] data;
		delete[] pal;
	}
	else
	{
		int			size = tgah.x_res * tgah.y_res;
		int			uncompressed_size = size;
		RGB24*		rgbin = (RGB24*)malloc( sizeof(RGB24) * size );
		fread( rgbin, sizeof(RGB24) * size, 1, fpin );
		fclose( fpin );

		//----------------------------------------------------------------
		// 4 Bits RLE (or 4Bits uncompressed if size is bigger compressed than uncompressed)
		//----------------------------------------------------------------
		if (mode == '4')
		{
			RLE_HEADER4	rleh;
			memset(rleh.palette, 0x0, 16*2);
			int			nb_pal = 0;
			int			nb_out = 0;

			rleh.rle = COMPRESS_RLE4 | half_height; // default is RLE4
			rleh.width = tgah.x_res; 
			rleh.height = tgah.y_res; 
			rleh.datasize = 0;

			unsigned char*	rleout = (unsigned char*)malloc( size ); // worst case 1 octet by pixel
			unsigned char*	out = rleout;

			// RLE compress
			RGB24*		in = rgbin;
			while (size)
			{
				// conv to 16b  444
				unsigned short	color = RGB24TO16(in);

				// find index in palette
				int idx = findColorInPalette(color, nb_pal, rleh.palette);			

				// not found -> add it
				if (idx == nb_pal) 
				{
					nb_pal++;
					if (nb_pal > 16) // too many colors
					{
						if (autocol)
						{
							free( rleout );
							free( rgbin );

							mode = '6';
							goto restart;
						}

						//printf("== FILE : %s == ", namein );
						//printf( "Error TGA with too many colors -> 16 allowed\n\n" );
						//fclose(fpin); // already closed
						SaveUncompressedFile(nameout, &tgah, rgbin, half_height);
						
						free( rleout );
						free( rgbin );

						return 0;
					}
					rleh.palette[idx] = color;
				}

				// find new matching colors (max 16 / end of file)
				int		repeat = 0;
				while ((repeat < 16) &&  (size))
				{
					if (RGB24TO16(in) != color)
						break;

					in++;
					size--;
					repeat++;
				}

				// add a color/length
				*out++ = ((repeat-1)<<4) | idx;
				rleh.datasize++;
			}

			FILE*	fpout = fopen( nameout, "wb" );
			if (fpout == NULL)
			{
				printf("== FILE : %s == ", namein );
				printf("Error opening output file\n\n");
				fclose(fpin);
				return -1;
			}

			// check that rle was successfull (not bigger than uncompressed)
			if (rleh.datasize >= (uncompressed_size>>1))
			{
				// better uncompressed
				rleh.rle = COMPRESS_PAL4 | half_height;
				rleh.datasize = uncompressed_size>>1;

				// create uncompressed
				size = uncompressed_size;
				out = rleout;
				in = rgbin;
				nb_pal = 0;
				while (size--)
				{
					// conv to 16b  444
					unsigned short	color = RGB24TO16(in);

					// find index in palette
					int idx = findColorInPalette(color, nb_pal, rleh.palette);				
					// not found -> add it
					if (idx == nb_pal) 
					{
						nb_pal++;
						if (nb_pal > 16) // too many colors
						{
							if (autocol)
							{
								free( rleout );
								free( rgbin );

								mode = '6';
								goto restart;
							}

							//printf("== FILE : %s == ", namein );
							//printf( "Error TGA with too many colors -> 16 allowed\n\n" );
							//fclose(fpin);
							SaveUncompressedFile(nameout, &tgah, rgbin, half_height);

							free( rleout );
							free( rgbin );

							return 0;
						}
						rleh.palette[idx] = color;
					}
					// save data
					if (size & 0x01)
					{
						*out = idx;
					}
					else
					{
						*out = *out | (idx << 4);
						out++;
					}
					in++;
				}
			}
			// write file

			if (bPalOnly) // only palette
			{
				//rleh.rle = COMPRESS_PAL4_ONLY;
				//rleh.datasize = 0; // palette only
				//fwrite( &rleh, sizeof( RLE_HEADER4 ), 1, fpout );
				unsigned char col = 16;
				fwrite(&col, 1, 1, fpout );
				fwrite(rleh.palette, 16 * 2, 1, fpout );
			}
			else
			{
				unsigned char *maskData = NULL;
				if (maskName)
				{
					rleh.rle |= IMAGE_MASK;
					maskData = GetMaskData(maskName, rleh.width, rleh.height);
				}

				fwrite( &rleh, sizeof( RLE_HEADER4 ), 1, fpout );
				fwrite( rleout, rleh.datasize, 1, fpout );

				if (maskData)
				{
					fwrite( maskData, rleh.datasize, 1, fpout );
				}
			}
			fclose(fpout);
			free( rleout );
		}
		else
			//----------------------------------------------------------------
			// 6 Bits RLE (or 8Bits uncompressed if size is bigger compressed than uncompressed)
			//----------------------------------------------------------------
			if (mode == '6')
			{
				RLE_HEADER6	rleh;
				memset(rleh.palette, 0x0, 64*2);
				int			nb_pal = 0;
				int			nb_out = 0;

				rleh.rle = COMPRESS_RLE6 | half_height; // default is RLE6
				rleh.width = tgah.x_res; 
				rleh.height = tgah.y_res; 
				rleh.datasize = 0;

				unsigned char*	rleout = (unsigned char*)malloc( size ); // worst case 1 octet by pixel
				unsigned char*	out = rleout;

				// RLE compress
				RGB24*		in = rgbin;
				while (size)
				{
					// conv to 16b  444
					unsigned short	color = RGB24TO16(in);

					// find index in palette
					int idx = findColorInPalette(color, nb_pal, rleh.palette);			
					// not found -> add it
					if (idx == nb_pal) 
					{
						nb_pal++;
						if (nb_pal > 64) // too many colors
						{
							if (autocol)
							{
								free( rleout );
								free( rgbin );

								mode = '8';
								goto restart;
							}

							//printf("== FILE : %s == ", namein );
							//printf( "Error TGA with too many colors -> 64 allowed\n\n" );
							//fclose(fpin);
							SaveUncompressedFile(nameout, &tgah, rgbin, half_height);

							free( rleout );
							free( rgbin );

							return 0;
						}
						rleh.palette[idx] = color;
					}

					// find new matching colors (max 4 / end of file)
					int		repeat = 0;
					while ((repeat < 4) &&  (size))
					{
						if (RGB24TO16(in) != color)
							break;

						in++;
						size--;
						repeat++;
					}

					// add a color/length
					*out++ = ((repeat-1)<<6) | idx;
					rleh.datasize++;
				}

				// check that rle was successfull (not bigger than uncompressed 8b)
				if (rleh.datasize >= (uncompressed_size))
				{
					// better uncompressed 8
					// free input data
					free( rgbin );
					free( rleout );
					mode = '8';
					goto restart;
				}

				// write file
				FILE*	fpout = fopen( nameout, "wb" );
				if (fpout == NULL)
				{
					printf("== FILE : %s == ", namein );
					printf("Error opening output file\n\n");
					fclose(fpin);
					return -1;
				}

				if (bPalOnly) // only palette
				{
					//rleh.rle = COMPRESS_PAL6_ONLY;
					//rleh.datasize = 0; // palette only
					//fwrite( &rleh, sizeof( RLE_HEADER6 ), 1, fpout );	// header
					unsigned char col = 64;
					fwrite(&col, 1, 1, fpout );
					fwrite(rleh.palette, 64 * 2, 1, fpout );
				}
				else
				{
					unsigned char *maskData = NULL;
					if (maskName)
					{
						rleh.rle |= IMAGE_MASK;
						maskData = GetMaskData(maskName, rleh.width, rleh.height);
					}

					fwrite( &rleh, sizeof( RLE_HEADER6 ), 1, fpout );	// header
					fwrite( rleout, rleh.datasize, 1, fpout );			// data

					if (maskData)
					{
						fwrite( maskData, rleh.datasize, 1, fpout );
					}

				}
				fclose(fpout);
				free( rleout );
			} 
			else
				//----------------------------------------------------------------
				// 8 Bits 
				//----------------------------------------------------------------
				if (mode == '8')
				{
					RLE_HEADER8	rleh;
					memset(rleh.palette, 0x0, 256*2);
					int			nb_pal = 0;
					int			nb_out = 0;

					rleh.rle = COMPRESS_PAL8 | half_height; // default is PAL8
					rleh.width = tgah.x_res; 
					rleh.height = tgah.y_res; 

					unsigned char*	rleout = (unsigned char*)malloc( size ); // worst case 1 octet by pixel
					unsigned char*	out = rleout;

					// RLE compress
					RGB24*		in = rgbin;
					rleh.datasize = uncompressed_size;

					// create uncompressed
					size = uncompressed_size;
					out = rleout;
					in = rgbin;
					nb_pal = 0;
					while (size--)
					{
						// conv to 16b  444
						unsigned short	color = RGB24TO16(in);

						// find index in palette
						int idx = findColorInPalette(color, nb_pal, rleh.palette);			

						// not found -> add it
						if (idx == nb_pal) 
						{
							nb_pal++;
							if (nb_pal > 256) // too many colors
							{
								//printf("== FILE : %s == ", namein );
								//printf( "Error TGA with too many colors -> 256 allowed\n\n" );
								//fclose(fpin);
								SaveUncompressedFile(nameout, &tgah, rgbin, half_height);

								free( rleout );
								free( rgbin );

								return 0;
							}
							rleh.palette[idx] = color;
						}
						// save data
						*out = idx;
						out++;
						in++;
					}
					// write file
					FILE*	fpout = fopen( nameout, "wb" );
					if (fpout == NULL)
					{
						printf("== FILE : %s == ", namein );
						printf("Error opening output file\n\n");
						return -1;
					}

					if (bPalOnly) // only palette
					{
						//rleh.rle = COMPRESS_PAL8_ONLY;
						//rleh.datasize = 0; // palette only
						//fwrite( &rleh, sizeof( RLE_HEADER8 ), 1, fpout );
						unsigned char col = 0;
						fwrite(&col, 1, 1, fpout );
						fwrite(rleh.palette, 256 * 2, 1, fpout );
					}
					else
					{
						unsigned char *maskData = NULL;
						if (maskName)
						{
							rleh.rle |= IMAGE_MASK;
							maskData = GetMaskData(maskName, rleh.width, rleh.height);
						}
						fwrite( &rleh, sizeof( RLE_HEADER8 ), 1, fpout );
						fwrite( rleout, rleh.datasize, 1, fpout );

						if (maskData)
						{
							fwrite( maskData, rleh.datasize, 1, fpout );
						}

					}
					fclose(fpout);
					free( rleout );
				}

				// free input data
				free( rgbin );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

int	main( int argc, char** argv )
{	
	if ((argc != 2) && (argc != 3) && (argc != 4) && (argc != 5))
	{
		printf("Usage : TGA2RLE inputTGA24filename outputRLEfilename 4|6|8 [p]\n\n");
		printf("OR    : TGA2RLE inputTGA24directory outputRLEdirectory\n\n");
		printf("OR    : TGA2RLE inputListFile\n\n");
		printf("Compress uncompressed TGA 24 bits to many formats :\n");
		printf("4 : 4bits palette with RLE 4bits (if the file is bigger than not RLEed, then uncompressed 4bits palette\n");
		printf("6 : 6bits palette with RLE 2bits (if the file is bigger than not RLEed, then uncompressed 8bits palette\n");
		printf("8 : 8bits palette\n");
		printf("p : 4/6/8bits palette only (no data)\n");
		printf("\n");
		return (-1);
	}

	GetFolder(argv[0], g_absPathToPVRToolEXE);
	strcat(g_absPathToPVRToolEXE, "\\PVRTexTool.exe");

	char	namein[256];
	char	nameout[256];
	char	mode = '4';
	int		autocol = 0;	

	bool	bFileExists = true;

	mFlags = 0;

	if (argc == 2) // input list
	{		
		ConvertFiles(argv[1]);

		return 0;
	}

	if (argc == 3 && !stricmp(argv[2], "-pvrtc"))
	{
		mFlags |= FLAG_USE_PVRTC;

		ConvertFiles(argv[1]);

		return 0;
	}

	HANDLE	ff;
	WIN32_FIND_DATA		fd;

	if (argc == 4 && !stricmp(argv[3], "-pvrtc"))
		mFlags |= FLAG_USE_PVRTC;
	
	if (argc == 3 ||  // auto
		argc == 4 && (mFlags & FLAG_USE_PVRTC))
	{
		autocol = 1;

		char	txt[256];
		strcpy( txt, argv[1] );
		strcat( txt, "\\*.tga" );
		ff = FindFirstFile( txt, &fd );

		if (ff == INVALID_HANDLE_VALUE)
		{
			printf("TGA2RLE:No .tga files FOUND on the path %s \n",txt);
			return -1;
		}
	}
	else
	{
		strcpy( namein, argv[1] );
		strcpy( nameout, argv[2] );
		mode = argv[3][0];
	}


	while (bFileExists)
	{
		bFileExists = false;

		if (autocol)
		{
			mode = '4';

			strcpy( namein, argv[1] );
			strcat( namein, "\\" );
			strcat( namein, fd.cFileName );

			strcpy( nameout, argv[2] );
			strcat( nameout, "\\" );
			strcat( nameout, fd.cFileName );

			nameout[strlen(nameout)-3] = 0;
			strcat( nameout, "RLE" );
		}

		//printf("name: %s %d\n", namein, mFlags);
#ifdef USE_PVR_TEXTURES
		if ((mFlags & FLAG_USE_PVRTC) && CanConvertFileToPVRTC(namein))
		{
			ConvertFileToPVRTC(namein, nameout);
		}
		else
#endif // USE_PVR_TEXTURES
		{
			ConvertFile(namein, nameout, autocol, mode, argc == 5);
		}

		// next file
		if (autocol)
		{
			//filesearch
			if (FindNextFile( ff, &fd ))
			{
				bFileExists = true;
			}
		}
	}

	return 0;
}