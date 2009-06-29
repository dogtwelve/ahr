#include <windows.h>
#include <stdio.h>
#include <io.h>

    
typedef struct
{
	unsigned char	r, g, b;

} RGB24;

typedef struct
{
	unsigned char	b, g, r, pad;

} RGB32;

typedef struct
{
#define COMPRESS_PAL4			0
#define COMPRESS_RLE4			1
#define COMPRESS_RLE6			2
#define COMPRESS_PAL8			3
#define COMPRESS_PAL4_ONLY		4
#define COMPRESS_PAL6_ONLY		5
#define COMPRESS_PAL8_ONLY		6
#define COMPRESS_HALF_HEIGHT_F	0x80
	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width
	unsigned short	height;
	unsigned long	datasize;		// file size
	unsigned short	palette[256];	// palette	// 16 (PAL4/RLE4) or 64 (RLE6) or 256 (PAL8)

} RLE_HEADER8;

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

inline	unsigned short	RGB32TO16( RGB32* c )
{
	// conv to 444
	unsigned short	col = (((c->r << 4) & 0x0F00) | (c->g & 0x0F0) | (c->b >> 4));

	// if not key color
	if (col != 0xF0F)
	{
		// get the 5th bits and save it in the unused higher 4b
		col = col | ((((c->r >> 1) & 0x04) | ((c->g >> 2) & 0x02) | ((c->b >> 3) & 0x01)) << 12);
	}
	else
	{
		// PUT F in the unused 4b
		col |= 0xF000;
	}
	return (col);
}

inline	unsigned short	RGB32TO16_2( RGB32* c )
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

int	main( int argc, char** argv )
{
	if (argc != 3)
	{
		printf("Usage   : BMP2RLE inputBMP8directory outputRLEdirectory\n\n");
		printf("Compress uncompressed BMP 8 bits AND palettes *.pal.??\n");
		printf("\n");
		return (-1);
	}

	char	namein[256];
	char	nameout[256];
	int		autocol = 0;
	HANDLE	ff;
	WIN32_FIND_DATA		fd;
//	FILE*	fplog;

	RGB32	pal[256];

	{
		autocol = 1;

		char	txt[256];
		strcpy( txt, argv[1] );
		strcat( txt, "\\*.bmp" );
		ff = FindFirstFile( txt, &fd );

		if (ff == INVALID_HANDLE_VALUE)
		{
			printf("No file FOUND\n");
			return -1;
		}

		//fplog = fopen( "texturelog.txt", "at" );
	}


loopautobmp:
	if (autocol)
	{
		strcpy( namein, argv[1] );
		strcat( namein, "\\" );
		strcat( namein, fd.cFileName );

		strcpy( nameout, argv[2] );
		strcat( nameout, "\\" );
		strcat( nameout, fd.cFileName );

		nameout[strlen(nameout)-3] = 0;
		strcat( nameout, "RLE" );
	}


	//-----------------------------------------------
	// check if the files have the same creation time
	{
		bool	doit1 = true;

		/*HANDLE	hfile = CreateFile( namein, GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (hfile != INVALID_HANDLE_VALUE)
		{
			BY_HANDLE_FILE_INFORMATION	fileinfo1;
			GetFileInformationByHandle( hfile, &fileinfo1 );
			CloseHandle( hfile );

			hfile = CreateFile( nameout, GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
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
			// next file
			if (autocol)
			{
				//fprintf( fplog, "%s : SKIPPED\n", nameout );

				//filesearch
				if (FindNextFile( ff, &fd ))
				{
					goto loopautobmp;
				}

				//fclose( fplog );

				goto dopalette;
			}
		}
	}
	//-----------------------------------------------

	{
		FILE*	fpin = fopen( namein, "rb" );
		if (fpin == NULL)
		{
			printf("== FILE : %s == ", namein );
			printf("Error opening input file\n\n");
			goto nextfile;
		}

		BITMAPFILEHEADER	bmpfh;
		fread( &bmpfh, sizeof( BITMAPFILEHEADER ), 1, fpin );

		BITMAPINFOHEADER	bmpih;
		fread( &bmpih, sizeof( BITMAPINFOHEADER ), 1, fpin );

		int		x_res = bmpih.biWidth;
		int		y_res = bmpih.biHeight;
		bool	bFlip = false;

		if (y_res < 0)
		{
			printf("The bitmap is top-bottom, will be flipped\n");
			y_res = -y_res;
			bFlip = true;
		}

		if ((bmpih.biBitCount != 8) || (bmpih.biCompression != 0) || (bmpih.biPlanes != 1))
		{
			printf("== FILE : %s == ", namein );
			printf("Error BMP format is not 8bits uncompressed\n\n");
			fclose(fpin);

			goto nextfile;
		}

		int		half_height = 0;
		if (x_res != y_res)
		{
			if (x_res == y_res*2)
			{
				half_height = COMPRESS_HALF_HEIGHT_F;
			}
			else
			{
	//			printf("== FILE : %s == ", namein );
	//			printf("Error TGA resolution is not square NOR half height\n\n");
	//			return -1;
			}
		}

	//	if ((tgah.x_res != 8) && (tgah.x_res != 16) && (tgah.x_res != 32) && (tgah.x_res != 64) && (tgah.x_res != 128)&& (tgah.x_res != 256))
	//	{
	//		printf("== FILE : %s == ", namein );
	//		printf("Error TGA resolution is not a power of 2 or is > 256\n\n");
	//		return -1;
	//	}


		// read palette
		fread( pal, 256/*bmpih.biClrUsed*/*4, 1, fpin );
		fseek( fpin, bmpfh.bfOffBits, SEEK_SET );

		// read input data
		int			size = x_res * y_res;
		int			uncompressed_size = size;
		unsigned char*	rgbin = (unsigned char*)malloc( size );
		fread( rgbin, size, 1, fpin );
		fclose( fpin );

		if (bFlip)
		{
			unsigned char tmp;
			int half_height = y_res >> 1;
			for (int i=0; i<half_height; i++)
				for (int j=0; j<x_res; j++)
				{
					int t = i * x_res + j;
					int k = (y_res - i - 1) * x_res + j;
					tmp = rgbin[t];
					rgbin[t] = rgbin[k];
					rgbin[k] = tmp;
				}
		}

	//	FILE* t = fopen( nameout, "wb" );
	//	fwrite( rgbin, size, 1, t );
	//	fclose( t );

		// 8 Bits 
		{
			RLE_HEADER8	rleh;
			int			nb_pal = 0;
			int			nb_out = 0;

			rleh.rle = COMPRESS_PAL8 | half_height; // default is PAL8
			rleh.datasize = size;
			rleh.width = x_res; 
			rleh.height = y_res; 

			// create palette
			for (unsigned int i = 0; i < 256/*bmpih.biClrUsed*/; i++)
			{
				rleh.palette[i] = RGB32TO16( pal + i );
			}
	//		for (i = bmpih.biClrUsed; i < 256; i++)
	//			rleh.palette[i] = 0;	// set to 0 for compression

			// write file
			FILE*	fpout = fopen( nameout, "wb" );
			if (fpout == NULL)
			{
				printf("== FILE : %s == ", namein );
				printf("Error opening output file\n\n");

				fclose(fpout);
				free( rgbin );

				goto nextfile;
			}
			
	//			rleh.rle = COMPRESS_PAL8_ONLY;
	//			rleh.datasize = 0; // palette only

			fwrite( &rleh, sizeof( RLE_HEADER8 ), 1, fpout );
			fwrite( rgbin, rleh.datasize, 1, fpout );

			fclose(fpout);
		}

		// free input data
		free( rgbin );



		//--------------------------------------------------------
		// set the same creation time for the 2 files (out = in)
		/*{
			HANDLE	hfile = CreateFile( namein, GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hfile != INVALID_HANDLE_VALUE)
			{
				FILETIME	time;
				GetFileTime( hfile, NULL, NULL, &time );
				CloseHandle( hfile );

				hfile = CreateFile( nameout, GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
				if (hfile != INVALID_HANDLE_VALUE)
				{
					SetFileTime( hfile, &time, &time, &time );
					CloseHandle( hfile );
				}
			}	
		}
		*/
		//-------------------------------------------------------
nextfile:
		// next file
		if (autocol)
		{
			//fprintf( fplog, "%s : 8 BMP (%d)\n", nameout, half_height );

			//filesearch
			if (FindNextFile( ff, &fd ))
			{
				goto loopautobmp;
			}

			//fclose( fplog );
		}
	}

	//========================================================================

	// PALETTE EXPORT (*.p??)
dopalette:
	{
		{
			autocol = 1;

			char	txt[256];
			strcpy( txt, argv[1] );
			strcat( txt, "\\*.pal" );
			ff = FindFirstFile( txt, &fd );

			if (ff == INVALID_HANDLE_VALUE)
			{
				printf("No palette files found in %s\n",argv[1]);
				return -1;
			}

			//fplog = fopen( "texturelog.txt", "at" );
		}


	loopautopal:
		if (autocol)
		{
			strcpy( namein, argv[1] );
			strcat( namein, "\\" );
			strcat( namein, fd.cFileName );

			strcpy( nameout, argv[2] );
			strcat( nameout, "\\" );
			strcat( nameout, fd.cFileName );

			nameout[strlen(nameout)-2] = 0;

// 			strcat( nameout, "P" );
// 			
// 			strcat( nameout, fd.cFileName + strlen(fd.cFileName) - 2 );
		}


		//-----------------------------------------------
		// check if the files have the same creation time
		{
			bool	doit1 = true;

			/*HANDLE	hfile = CreateFile( namein, GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hfile != INVALID_HANDLE_VALUE)
			{
				BY_HANDLE_FILE_INFORMATION	fileinfo1;
				GetFileInformationByHandle( hfile, &fileinfo1 );
				CloseHandle( hfile );

				hfile = CreateFile( nameout, GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
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
				// next file
				if (autocol)
				{
					//fprintf( fplog, "%s : SKIPPED\n", nameout );

					//filesearch
					if (FindNextFile( ff, &fd ))
					{
						goto loopautopal;
					}

					//fclose( fplog );

					exit(0);
				}
			}
		}
		//-----------------------------------------------



		FILE* fpin = fopen( namein, "rb" );
		if (fpin == NULL)
		{
			printf("== FILE : %s == ", namein );
			printf("Error opening input file\n\n");
			return -1;
		}

		fseek( fpin, 22, SEEK_SET );
		unsigned short	colused = 0;		
		fread( &colused, 2, 1, fpin );
		//printf("Palette color used: %d\n", colused);

		// read palette
		fread( pal, colused*4, 1, fpin );
		fclose( fpin );

		unsigned short	pal16[256];

		// create palette
		for (unsigned int i = 0; i < colused; i++)
		{
			pal16[i] = RGB32TO16_2( pal + i );
		}

		// write file
		FILE*	fpout = fopen( nameout, "wb" );
		if (fpout == NULL)
		{
			printf("== FILE : %s == ", namein );
			printf("Error opening output file\n\n");
			return -1;
		}

		unsigned char col = colused == 256 ? 0 : colused;
		fwrite( &col, 1, 1, fpout );
		fwrite( pal16, colused*2, 1, fpout );

		fclose(fpout);



		//--------------------------------------------------------
		// set the same creation time for the 2 files (out = in)
		/*{
			HANDLE	hfile = CreateFile( namein, GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hfile != INVALID_HANDLE_VALUE)
			{
				FILETIME	time;
				GetFileTime( hfile, NULL, NULL, &time );
				CloseHandle( hfile );

				hfile = CreateFile( nameout, GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
				if (hfile != INVALID_HANDLE_VALUE)
				{
					SetFileTime( hfile, &time, &time, &time );
					CloseHandle( hfile );
				}
			}	
		}
		*/
		//-------------------------------------------------------



		// next file
		if (autocol)
		{
			//fprintf( fplog, "%s : 8 PAL\n", nameout );

			//filesearch
			if (FindNextFile( ff, &fd ))
			{
				goto loopautopal;
			}

			//fclose( fplog );
		}

	}
	//------------------------------------------------------------------------
	
	return 0;
}