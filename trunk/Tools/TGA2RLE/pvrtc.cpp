#include <windows.h>
#include <stdio.h>
#include <process.h>

#include "defines.h"
#include "pvrtc.h"

//use this to take the PVRTexTool.exe absolute path
char g_absPathToPVRToolEXE[1024];

void getDataFromTga( unsigned char *data, const TGA_HEADER &tgah )
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

bool checkDataForFullyTranspColor24bpp( unsigned char* tgaData, int imageSize )
{
	
	for(int pixel = 0; pixel < imageSize; pixel++)
	{
		unsigned char r, g, b;

		b = *tgaData++;
		g = *tgaData++;
		r = *tgaData++;

		unsigned short color = (((b << 4) & 0x0F00) | (g & 0x0F0) | (r >> 4));
		if (color == 0xF0F)
			return true; 
	}

	return false;
}

bool checkPaletteForFullyTranspColor24bpp( unsigned char* tgaPalette, int palSize)
{
	for(int idx = 0; idx < palSize; idx++)
	{
		unsigned char r, g, b;

		b = *tgaPalette++;
		g = *tgaPalette++;
		r = *tgaPalette++;

		unsigned short color = (((b << 4) & 0x0F00) | (g & 0x0F0) | (r >> 4));
		if (color == 0xF0F)
		{
			return true; 
		}
	}

	return false;
}

#define MAX_WIDTH		1024
#define MAX_HEIGHT		1024
#define MAX_BYTES_PP	4		//if 

unsigned char s_decodedBuffer[MAX_WIDTH * MAX_HEIGHT * MAX_BYTES_PP]; 

//srcData will be 24bpp ... 
int ConvertToTGASpecificForPVR( char* fileNameConverted_TGA, unsigned char* srcData, const TGA_HEADER &tgah )
{	
	int tgaSize;
	unsigned char* convertedTGA = s_decodedBuffer;
	bool hasFullyTranspColor = false;
	int width = tgah.x_res;
	int height = tgah.y_res;

	TGA_HEADER convertedTGA_header;

	//no palette
	unsigned char* palData = 0;
	
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

	unsigned char* dst = convertedTGA;
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


	volatile unsigned char r = 0;
	volatile unsigned char g = 0;
	volatile unsigned char b = 0;

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

				unsigned short color = (((b << 4) & 0x0F00) | (g & 0x0F0) | (r >> 4));
				int alphaColor = (color == 0xF0F); // ( r == 0xFF && g == 0x0 && b == 0xFF );
				if (alphaColor)				
				{
					// Replace with black
					r = g = b = 0;
				}

				//little endian
				*dst++ = b; 
				*dst++ = g;
				*dst++ = r;
				*dst++ = alphaColor ? 0x0 : 0xFF; //alpha				
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


bool CanConvertFileToPVRTC(char *namein)
{
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

	fclose(fpin);

#ifdef IPHONE_BUG_SQUARE_PVR_TEXTURES
	if (tgah.x_res != tgah.y_res)
		return false;
#endif // IPHONE_BUG_SQUARE_PVR_TEXTURES

	return true;
}

int ConvertFileToPVRTC(char *namein, char *nameout)
{
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

	fseek(fpin, 0, SEEK_END);
	int fileTGASize = ftell(fpin);
	rewind(fpin);

	//converte tga specific to pvr
	unsigned char* tempBuffer = new unsigned char[fileTGASize];
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
	// Mask not supported yet
	//if( maskName != NULL )
	//{
	//	//try to see if mask file exists
	//	FILE *fpmask = fopen( maskName, "rb" );
	//	if (fpmask == NULL)
	//	{
	//		printf("== FILE : %s == ", maskName );
	//		printf("Error opening mask file\n\n");		
	//		return -1;
	//	}	
	//	fclose(fpmask);

	//	strcat(pvrtc_args[4], "-a");
	//	strcat(pvrtc_args[4], maskName);	
	//}

	//spawn new process to export the texture -> result will be a file with the extension .pvr
	int ret = _spawnl( _P_WAIT, g_absPathToPVRToolEXE, g_absPathToPVRToolEXE, pvrtc_args[0], pvrtc_args[1], pvrtc_args[2], pvrtc_args[3], pvrtc_args[4], NULL);

	if( ret != 0)
	{
		printf("\n\n ERROR COMPRESSING TO *.PVRTC [%s] \n TRY COMPRESS TO *.RLE\n\n\n\n", namein );

		return 99;
	}

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
			int size = CHUNK_SIZE;
			if (fileSize < size)
				size = fileSize;
			size_t actuallyRead = fread(buffer, 1, size, pvrtcFile);
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