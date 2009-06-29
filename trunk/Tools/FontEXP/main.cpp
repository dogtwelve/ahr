
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;
#define for if (false) ; else for

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
    
#define COMPRESS_HALF_HEIGHT_F	0x80

struct RGB24
{
	RGB24(unsigned char	r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
	
	unsigned char b, g, r;

	bool operator!=(const RGB24& rhs) const
	{
		return !(r == rhs.r && g == rhs.g && b == rhs.b);
	}
};

ostream& operator<<(ostream& o, const RGB24& rhs)
{
	o << (unsigned)rhs.r << ":" << (unsigned)rhs.g << ":" << (unsigned)rhs.b;
	return o;
}


struct FontDescription
{
	unsigned size_y;
	struct Character
	{
		unsigned short pos_x;
		unsigned short pos_y;
		unsigned size_x;
	} characters[256];
};

bool	is_empty( RGB24* ptr, int width, int height, int x, int y, int sx, int sy )
{
	for (int j = 0; j < sy; j++)
	{
		for (int i = 0; i < sx; i++)
		{
			if (ptr[ x + i + (height - 1 - (j + y)) * width ] != RGB24( 255,0,255))
				return false;
		}
	}
	return true;
}

int		main( int argc, char** argv )
{
	if (argc != 3)
	{
		printf("Usage : FontEXP input.tga output.fnt\n");
		return 0;
	}

	char* argv1 = argv[1];
	char* argv2 = argv[2];

	FILE*	fpin = fopen( argv1, "rb" );
	if (fpin == NULL)
	{
		printf("== FILE : %s == ", argv1 );
		printf("Error opening input file\n\n");
		return -1;
	}

	TGA_HEADER	tgah;
	
	fread( &tgah, sizeof( TGA_HEADER ), 1, fpin );

	if ((tgah.twenty_four != 24) || (tgah.code != 2))
	{
		printf("== FILE : %s == ", argv1 );
		printf("Error TGA format is not 24bits uncompressed\n\n");
		return -1;
	}

	// read input data
	int			size = tgah.x_res * tgah.y_res;
	int			uncompressed_size = size;
	RGB24*		rgbin = (RGB24*)malloc( sizeof(RGB24) * size );
	fread( rgbin, sizeof(RGB24) * size, 1, fpin );
	fclose( fpin );

	ofstream output(argv2, ofstream::binary);
	if (output.fail())
	{
		cout << "open output file failed : " << argv2 << " : " << strerror(errno) << endl;
		return -1;
	}

	FontDescription font;
	font.size_y = tgah.y_res / 16 - 1;

	RGB24*		rgbout = NULL;
	int			rgboutx = 0;

	for (unsigned j = 0; j < 16; ++j)
	{
		unsigned cur_x = 0;
		unsigned cur_y = j * (font.size_y + 1);
		for (unsigned i = 0; i < 16; ++i, ++cur_x)
		{
			FontDescription::Character& car = font.characters[j * 16 + i];
			car.pos_x = cur_x;
			car.pos_y = cur_y;
			while (rgbin[(tgah.y_res - cur_y - 1) * tgah.x_res + cur_x] != RGB24(0,255,255)
					&& cur_x < tgah.x_res)
				++cur_x;
			car.size_x = cur_x - car.pos_x;

			if (!(
					(j == 2 && i == 0)   // leave space character
				 || (j == 1 && i == 15)  // leave dummy character for track names
				)
				&& is_empty( rgbin, tgah.x_res, tgah.y_res, car.pos_x, car.pos_y, car.size_x, font.size_y ))
			{
				// remove it
				car.pos_x = car.pos_y =	car.size_x = 0;
			}
			else
			{
				// add it

				int		rgboutx2 = rgboutx + car.size_x;
				RGB24*	out2 = (RGB24*)malloc( sizeof( RGB24 ) * font.size_y * rgboutx2 );

				// copy old data
				for (int j = 0; j < font.size_y; j++)
				{
					for (int i = 0; i < rgboutx; i++)
					{
						out2[ i + j * rgboutx2 ] = rgbout[ i + j * rgboutx ];
					}
				}


				// add data
				for (int j = 0; j < font.size_y; j++)
				{
					for (int i = 0; i < car.size_x; i++)
					{
						out2[ rgboutx + i + (font.size_y - 1 - j) * rgboutx2 ] = rgbin[ car.pos_x + i + (tgah.y_res - 1 - (j + car.pos_y)) * tgah.x_res ];
					}
				}

				// change car data
				car.pos_x = rgboutx;
				car.pos_y = 0;

				// swap
				if (rgbout)	free(rgbout);
				rgbout = out2;
				rgboutx = rgboutx2;
			}
		}
	}
	
	output.write((const char*)&font, sizeof(FontDescription));

	// write new tga
	{
		TGA_HEADER	header;

		header = tgah;

		header.x_res = rgboutx;
		header.y_res = font.size_y;

		char	name[256];

		strcpy( name, argv1 );
		strcpy( name + strlen(name)-4, "2.tga" );
		FILE*	fp = fopen( name, "wb" );

		fwrite( &header, sizeof(TGA_HEADER), 1, fp );

		fwrite( rgbout, sizeof(RGB24)*font.size_y*rgboutx, 1, fp );

		fclose( fp );
	}


	return 0;
}