#include <stdio.h>
#include <windows.h>

#define	WIDTH		256
#define SCR_WIDTH	240

typedef struct
{
	unsigned char	r, g, b;

} RGB_COL;

typedef struct
{
	unsigned short		index;//char	x, y;

} XY_COL;

int		main( int argc, char** argv)
{
	if ((argc != 1) && (argc != 3) && (argc != 4))
	{
		printf("Usage : EnvmapTable                  (to generate the 176x176 envmaptable)\n");
		printf("        EnvmapTable in.raw out.tab [cab]  (to transcode the 128x128 envmap)\n");

		return -1;
	}

	if (argc == 1)
	{
		FILE*	fp = fopen( "envmaptable.raw", "wb" );

		for (int y = 0; y < SCR_WIDTH; y++)
			for (int x = 0; x < SCR_WIDTH; x++)
			{
				RGB_COL	col;

				col.r = x;
				col.b = y;
				col.g = 0;

				fwrite( &col, sizeof(RGB_COL), 1, fp );
			}


		fclose( fp );
	}


	FILE*	fi = fopen( argv[1], "rb" );
	if (!fi)
	{
		printf("Error can't open %s\n", argv[1] );
		return -1;
	}

	
	RGB_COL		in[WIDTH*WIDTH];

	fread( in, WIDTH*WIDTH*3, 1, fi );

	fclose(fi);

	XY_COL		out[WIDTH*WIDTH];

	int	i;
	for (i = 0; i < WIDTH*WIDTH; i++)
	{
		if ((in[i].r == 0) && (in[i].b == 0))
		{
			out[i].index = 0;
		}
		else
		{
			/**/
			int a = (int)in[i].r * 240 / 176;
			in[i].r = a;
			a = (int)in[i].b * 240 / 176;
			in[i].b = a;
			/**/

			if (argc == 4)	// cab flip
			{
				if (i < WIDTH*(WIDTH/2))
					out[i].index = (in[i].r) + in[i].b*SCR_WIDTH;
				else
					out[i].index = ((SCR_WIDTH-1)-in[i].r) + in[i].b*SCR_WIDTH;
			}
			else
			{	if (i < WIDTH*(WIDTH/2))
					out[i].index = ((SCR_WIDTH-1)-in[i].r) + in[i].b*SCR_WIDTH;
				else
					out[i].index = (in[i].r) + in[i].b*SCR_WIDTH;
			}

			if (out[i].index == 0)
				out[i].index = 1;
			//out[i].x = in[i].r;
			//out[i].y = in[i].b;
		}
	}


	fi = fopen( argv[2], "wb" );
	if (!fi)
	{
		printf("Error can't open %s\n", argv[2] );
		return -1;
	}

//	fwrite( out, WIDTH*WIDTH*2, 1, fi );

	// write only the half to save space we will regenerate the other half in the game !
//	fwrite( out, WIDTH*(WIDTH/2)*2, 1, fi );

	// save only one pixel out of two
	//for (int y = 0; y < WIDTH/2; y+=2)
	for (int y = 0; y < WIDTH/2; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		//for (int x = 0; x < WIDTH; x+=2)
		{
			fwrite( out + (y*WIDTH + x), 2, 1, fi );
		}
	}


	fclose( fi );


	return 0;
}