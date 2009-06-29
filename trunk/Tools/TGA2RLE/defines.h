#ifndef MY_DEFINES_H__

// Warning:
// Do not use sizeof(TGA_HEADER), the size may be bigger due to data alignment
#define TGA_HEADER_SIZE		18

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
#define COMPRESS_PVRTC			7       //// PowerVR MBX specific

#define UNCOMPRESSED			0x0010
#define ALPHA_CHANNEL			0x0020	// goes with UNCOMPRESSED type; if set => image has 32 bpp, if not set => image has 24 bpp
#define IMAGE_MASK				0x0040
#define COMPRESS_HALF_HEIGHT_F	0x0080

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

typedef struct
{
	unsigned short	rle;			// 1 = yes, 0 = no rle (depends on final compression ratio)
	unsigned short	width;			// width
	unsigned short	height;
	unsigned long	datasize;		// file size
} RLE_HEADER_UNCOMPRESSED;

#endif // MY_DEFINES_H__