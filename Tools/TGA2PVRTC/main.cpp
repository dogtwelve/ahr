    #include <windows.h>
#include <stdio.h>
#include <io.h>
#include <process.h>

// Warning:
// Do not use sizeof(TGA_HEADER), the size may be bigger due to data alignment
#define TGA_HEADER_SIZE		18
#define	MATH_MIN(a, b)	((a) < (b) ? (a) : (b))			

//use this to take the PVRTexTool.exe absolute path
char g_absPathToPVRToolEXE[1024];

typedef unsigned char u8;
typedef unsigned int  u32;


typedef struct
{
	unsigned char idLength;
	unsigned char colorMapType;
	unsigned char code;
	unsigned short palStart;       /* 03h  Color map origin */
	unsigned short palLength;      /* 05h  Color map length */
	unsigned char palDepth;       /* 07h  Depth of color map entries */
	unsigned char dummy[4];		 // 9 unused bytes
	unsigned short x_res;
	unsigned short y_res;
	unsigned char twenty_four;
	unsigned char space;
} TGA_HEADER;
    
typedef struct
{
	unsigned char	r, g, b;

} RGB24;

#define COMPRESS_PAL4			0
#define COMPRESS_RLE4			1
#define COMPRESS_RLE6			2
#define COMPRESS_PAL8			3
#define COMPRESS_PAL4_ONLY		4
#define COMPRESS_PAL6_ONLY		5
#define COMPRESS_PAL8_ONLY		6
#define COMPRESS_PVRTC			7	// PowerVR MBX specific
#define COMPRESS_HALF_HEIGHT_F	0x80
#define IMAGE_MASK				0x40

typedef struct
{
	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width
	unsigned short	height;
	unsigned long	datasize;		// file size
} RLE_HEADER_WITHOUT_PALETTE;


typedef struct
{
	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width
	unsigned short	height;
	unsigned long	datasize;		// file size
	unsigned short	palette[256];	// palette	// 16 (PAL4/RLE4) or 64 (RLE6) or 256 (PAL8)

} RLE_HEADER8;

typedef struct
{
	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width=heigh <=256 
	unsigned short	height;
	unsigned long	datasize;		// file size
	unsigned short	palette[64];	// palette	// 16 (PAL4/RLE4) or 64 (RLE6) or 256 (PAL8)

} RLE_HEADER6;

typedef struct
{
	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width=heigh <=256 
	unsigned short	height;
	unsigned long	datasize;		// file size
	unsigned short	palette[16];	// palette	// 16 (PAL4/RLE4) or 64 (RLE6) or 256 (PAL8)

} RLE_HEADER4;

void fillTgaHeader(TGA_HEADER *tgah, u8 *data)
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


void getDataFromTga( u8 *data, const TGA_HEADER &tgah )
{
	unsigned char *d = data;

	*d++ = tgah.idLength & 0xFF;
	*d++ = tgah.colorMapType & 0xFF;
	*d++ = tgah.code & 0xFF;

	*d++ = tgah.palStart & 0xFF;
	*d++ = ( tgah.palStart >> 8 ) & 0xFF;

	*d++ = tgah.palLength & 0xFF;
	*d++ = ( tgah.palLength >> 8 )& 0xFF;

	*d++ = tgah.palDepth;

	d+=4;

	*d++ = tgah.x_res & 0xFF;
	*d++ = ( tgah.x_res >> 8 ) & 0xFF;

	*d++ = tgah.y_res & 0xFF;
	*d++ = ( tgah.y_res >> 8 ) & 0xFF;

	*d++ = tgah.twenty_four & 0xFF;
	*d++ = 0;
}


int GetPow2Size(int size)
{
	int pow2Size = 1;

	while(pow2Size < size)
		pow2Size <<=1;

	return pow2Size;
}

//0xFF00FF
bool checkDataForFullyTranspColor24bpp( u8* tgaData, int imageSize )
{
	
	for(int pixel = 0; pixel < imageSize; pixel++)
	{
		u8 r, g, b;

		b = *tgaData++;
		g = *tgaData++;
		r = *tgaData++;

		if( r == 0xFF && g == 0x0 && b == 0xFF )
			return true; 
	}

	return false;
}

bool checkPaletteForFullyTranspColor24bpp( u8* tgaPalette, int palSize)
{
	for(int idx = 0; idx < palSize; idx++)
	{
		u8 r, g, b;

		b = *tgaPalette++;
		g = *tgaPalette++;
		r = *tgaPalette++;

		if( r == 0xFF && g == 0x0 && b == 0xFF )
			return true; 
	}

	return false;
}

#define MAX_WIDTH		1024
#define MAX_HEIGHT		1024
#define MAX_BYTES_PP	4		//if 

u8 s_decodedBuffer[MAX_WIDTH * MAX_HEIGHT * MAX_BYTES_PP]; 

//srcData will be 24bpp ... 
int ConvertToTGASpecificForPVR( char* fileNameConverted_TGA, u8* srcData, const TGA_HEADER &tgah )
{	
	int tgaSize;
	u8* convertedTGA = s_decodedBuffer;
	bool hasFullyTranspColor = false;
	int width = tgah.x_res;
	int height = tgah.y_res;

	TGA_HEADER convertedTGA_header;

	//no palette
	u8* palData = 0;

	
	//check for fully transp color
	if (tgah.colorMapType != 0)
	{
		hasFullyTranspColor = checkPaletteForFullyTranspColor24bpp(srcData, tgah.palLength);
		palData = srcData;
		srcData += (tgah.palLength * 3);
	}
	else
	{
		hasFullyTranspColor = checkDataForFullyTranspColor24bpp(srcData, width * height);
	}

	u8* dst = convertedTGA;
	//skip header ... will fill this later
	dst+=TGA_HEADER_SIZE; 
		

	int widthPow2	= GetPow2Size(width);
	int heightPow2	= GetPow2Size(height);


	convertedTGA_header.idLength		= 0;
	convertedTGA_header.colorMapType	= 0;
	convertedTGA_header.code			= 2;
	convertedTGA_header.palStart		= 0;
	convertedTGA_header.palLength		= 0;
	convertedTGA_header.palDepth		= 0;
	convertedTGA_header.x_res			= widthPow2;
	convertedTGA_header.y_res			= heightPow2;
	convertedTGA_header.twenty_four		= 24;


	volatile u8 r = 0;
	volatile u8 g = 0;
	volatile u8 b = 0;

	if(hasFullyTranspColor)
	{
		memset(dst, 0x0, widthPow2 * heightPow2 * 4);
		tgaSize = TGA_HEADER_SIZE + (widthPow2 * heightPow2 * 4);

		for(int j=0; j< height; j++)
		{
			for( int i=0; i< width; i++ )
			{
				//get color 				
				if(palData != 0)
				{
					//get color from palette
					int idx = *srcData++;
					b = palData[idx * 3];
					g = palData[idx * 3 + 1];
					r = palData[idx * 3 + 2];
				}
				else
				{
					//get the color from image data
					b = *srcData++;
					g = *srcData++;
					r = *srcData++;
				}

				//little endian
				*dst++ = b; 
				*dst++ = g;
				*dst++ = r;
				*dst++ = ( r == 0xFF && g == 0x0 && b == 0xFF ) ? 0x0 : 0xFF; //alpha				
			}

			//skip remaining pixels for the current line
			dst += ( ( widthPow2 - width ) * 4 );		
		}

		//has alpha channel
		convertedTGA_header.twenty_four		= 32;	
	}
	else
	{
		memset(dst, 0x0, widthPow2 * heightPow2 * 3);
		tgaSize = TGA_HEADER_SIZE + (widthPow2 * heightPow2 * 3);

		for(int j=0; j< height; j++)
		{
			for( int i=0; i< width; i++ )
			{
				//get color 				
				if(palData != 0)
				{
					//get color from palette
					int idx = *srcData++;
					b = palData[idx * 3];
					g = palData[idx * 3 + 1];
					r = palData[idx * 3 + 2];
				}
				else
				{
					//get the color from image data
					b = *srcData++;
					g = *srcData++;
					r = *srcData++;
				}

				//little endian
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
			}

			//skip remaining pixels for the current line
			dst += ( ( widthPow2 - width ) * 3 );		
		}
	}

	getDataFromTga( convertedTGA, convertedTGA_header);

	//create file converted
	FILE * f = fopen(fileNameConverted_TGA, "wb");
	
	if(f == NULL )
	{
		printf("ERROR CREATE FILE [%s]\n", fileNameConverted_TGA);
		return -1;
	}
	fwrite(convertedTGA, 1, tgaSize, f);
	fclose(f);
	

	return 0;
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
		*folderName++='.';
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

int ConvertFile(char *namein, char *nameout, int autocol, char &mode, bool bPalOnly, char *maskName)
{
// TODO: rewrite
	bool checkPVRCompress = true;

restart:
	//parse tga ... also check if exist
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
	else if( tgah.colorMapType != 0 )
	{
		int bytesCount = tgah.palDepth / 8;

		if (bytesCount != 3)
		{
			printf("== FILE : %s == ", namein );
			printf("Error TGA format has not 24bits palette depth\n\n");
			fclose(fpin);
			return -1;
		}
	}

	int		half_height = 0;
	if (tgah.x_res != tgah.y_res)
	{
		if (tgah.x_res == tgah.y_res*2)
		{
			half_height = COMPRESS_HALF_HEIGHT_F;
		}
	}	

	if( checkPVRCompress )
	{
		fseek(fpin, 0, SEEK_END);
		int fileTGASize = ftell(fpin);
		rewind(fpin);

		//converte tga specific to pvr
		u8* tempBuffer = new u8[fileTGASize];
		fread(tempBuffer, 1, fileTGASize, fpin);
		fclose(fpin);

		char convertedTGAName[1024];

		strcpy(convertedTGAName, namein);
		char *startExtension = strrchr(convertedTGAName, '.');
		*startExtension = '\0';
		strcat(convertedTGAName, ".CONV.tga");

		//make the tga pow2, and also use alpha channel for those with 0xFF00FF pixels
		ConvertToTGASpecificForPVR(convertedTGAName, tempBuffer + TGA_HEADER_SIZE, tgah);

		
		delete[] tempBuffer;		


		//CONSTRUCT THE COMMAND LINE for PVRTexTool.exe
		char pvrtc_args[5][1024];
		strcpy(pvrtc_args[0], "-p");
		strcpy(pvrtc_args[1], "-f");
		
		//also OGLPVRTC2 could be used ... low quality
		strcat(pvrtc_args[1], "OGLPVRTC4");

		//specify input file (tga in this case)
		strcpy(pvrtc_args[2], "-i");
		strcat(pvrtc_args[2], convertedTGAName);
		

		//generate the name of the output file to pass to PVRTexTool.exe
		char transformedFile[1024];
		strcpy(transformedFile, nameout);
		strcat(transformedFile, ".pvr");

		//specify the output file ( pvr file )
		strcpy(pvrtc_args[3], "-o");
		strcat(pvrtc_args[3], transformedFile);

		//specify mask if needed
		strcpy(pvrtc_args[4],"");
		if( maskName != NULL )
		{
			//try to see if mask file exists
			FILE *fpmask = fopen( maskName, "rb" );
			if (fpmask == NULL)
			{
				printf("== FILE : %s == ", maskName );
				printf("Error opening mask file\n\n");		
				return -1;
			}	
			fclose(fpmask);

			strcat(pvrtc_args[4], "-a");
			strcat(pvrtc_args[4], maskName);	
		}

		//spawn new process to export the texture -> result will be a file with the extension .pvr
		int ret = _spawnl( _P_WAIT, g_absPathToPVRToolEXE, g_absPathToPVRToolEXE, pvrtc_args[0], pvrtc_args[1], pvrtc_args[2], pvrtc_args[3], pvrtc_args[4], NULL);
		
		if( ret != 0)
		{
			printf("\n\n ERROR COMPRESSING TO *.PVRTC [%s] \n TRY COMPRESS TO *.RLE\n\n\n\n", namein );
			checkPVRCompress = false;			
		}
		else
		{

			//find *.pvr size
			FILE* pvrtcFile = fopen(transformedFile, "rb");
			fseek(pvrtcFile, 0, SEEK_END);
			int fileSize = ftell(pvrtcFile);
			rewind(pvrtcFile);


			//save here information about what type of compress is used
			//needed when the texture is loaded
			RLE_HEADER_WITHOUT_PALETTE rleh;	

			rleh.rle = COMPRESS_PVRTC;
			rleh.width = tgah.x_res; 
			rleh.height = tgah.y_res;
			rleh.datasize = fileSize;
			

			FILE*	fpout = fopen( nameout, "wb" );
			
			if (fpout == NULL)
			{
				printf("== FILE : %s == ", nameout );
				printf("Error opening output file\n\n");
				fclose(fpout);
				return -1;
			}

			fwrite( &rleh, sizeof( RLE_HEADER_WITHOUT_PALETTE ), 1, fpout );

			//copy contents to the output file
			const int CHUNK_SIZE = (1024);
			char buffer[CHUNK_SIZE];

			//copy pvrtc file content in the output file
			if(pvrtcFile != NULL) 
			{
				while(fileSize > 0)
				{
					size_t actuallyRead = fread(buffer, 1, MATH_MIN(CHUNK_SIZE, fileSize), pvrtcFile);
					fwrite(buffer, 1, actuallyRead, fpout);
					fileSize -= actuallyRead;
				}
				fclose(pvrtcFile);
			}

			//we're done
			fclose(fpout);

			//printf("CREATED PVRTC [%s] USING MASK [%s] \n", transformedFile, maskName);
			
			//succes
			return 0;
		}
	}//checkPVRCompress

	// read input data
	if (tgah.colorMapType != 0)
	{
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
			fclose(fpin);
			return -1;
		}

		if (bPalOnly) // only palette
		{
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
						free( rleout );
						free( rgbin );

						if (autocol)
						{
							mode = '6';
							goto restart;
						}

						printf("== FILE : %s == ", namein );
						printf( "Error TGA with too many colors -> 16 allowed\n\n" );
						fclose(fpin);
						return -1;
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
							free( rleout );
							free( rgbin );

							if (autocol)
							{
								mode = '6';
								goto restart;
							}

							printf("== FILE : %s == ", namein );
							printf( "Error TGA with too many colors -> 16 allowed\n\n" );
							fclose(fpin);
							return -1;
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
				fwrite(rleh.palette, 16 * 2, 1, fpout );
			}
			else
			{
				fwrite( &rleh, sizeof( RLE_HEADER4 ), 1, fpout );
				fwrite( rleout, rleh.datasize, 1, fpout );
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
							free( rleout );
							free( rgbin );

							if (autocol)
							{
								mode = '8';
								goto restart;
							}

							printf("== FILE : %s == ", namein );
							printf( "Error TGA with too many colors -> 64 allowed\n\n" );
							fclose(fpin);
							return -1;
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
					fwrite(rleh.palette, 64 * 2, 1, fpout );
				}
				else
				{
					fwrite( &rleh, sizeof( RLE_HEADER6 ), 1, fpout );	// header
					fwrite( rleout, rleh.datasize, 1, fpout );			// data
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
								free( rleout );
								free( rgbin );

								printf("== FILE : %s == ", namein );
								printf( "Error TGA with too many colors -> 256 allowed\n\n" );
								fclose(fpin);
								return -1;
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
						fclose(fpin);
						return -1;
					}

					if (bPalOnly) // only palette
					{
						//rleh.rle = COMPRESS_PAL8_ONLY;
						//rleh.datasize = 0; // palette only
						//fwrite( &rleh, sizeof( RLE_HEADER8 ), 1, fpout );
						fwrite(rleh.palette, 256 * 2, 1, fpout );
					}
					else
					{
						fwrite( &rleh, sizeof( RLE_HEADER8 ), 1, fpout );
						fwrite( rleout, rleh.datasize, 1, fpout );
					}
					fclose(fpout);
					free( rleout );
				}

				// free input data
				free( rgbin );
	}

	fclose(fpin);

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

	printf("\n\n !!!PVRTEXTOOL SHOULD BE HERE [%s]!!! \n\n", g_absPathToPVRToolEXE);


	char	namein[256];
	char	nameout[256];
	char	mode = '4';
	int		autocol = 0;	

	printf("%s\n", argv[0]);

	bool	bFileExists = true;

	if (argc == 2) // input list
	{		
		ConvertFiles(argv[1]);

		return 0;
	}
	
	HANDLE	ff;
	WIN32_FIND_DATA		fd;	
	
	if (argc == 3) // auto
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

		ConvertFile(namein, nameout, autocol, mode, argc == 5);

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