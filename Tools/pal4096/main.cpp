#include <io.h>
#include <stdio.h>

void	main()
{
	FILE*	fp = fopen( "Pal4096.raw", "wb" );
/*	for (int f = 0; f < 16; f++)
	{
		unsigned char	cf = f<<4;
		fwrite( &cf, 1, 1, fp );
		fwrite( &cf, 1, 1, fp );
		fwrite( &cf, 1, 1, fp );
	}

	for ( f = 0; f < 16; f++)
	{
		unsigned char	cf = f<<4;
		cf = f<<4;
		fwrite( &cf, 1, 1, fp );
		cf = 0;
		fwrite( &cf, 1, 1, fp );
		fwrite( &cf, 1, 1, fp );
	}
	for ( f = 0; f < 16; f++)
	{
		unsigned char	cf = f<<4;
		cf = 0;
		fwrite( &cf, 1, 1, fp );
		cf = f<<4;
		fwrite( &cf, 1, 1, fp );
		cf = 0;
		fwrite( &cf, 1, 1, fp );
	}
	for ( f = 0; f < 16; f++)
	{
		unsigned char	cf = f<<4;
		cf = 0;
		fwrite( &cf, 1, 1, fp );
		fwrite( &cf, 1, 1, fp );
		cf = f<<4;
		fwrite( &cf, 1, 1, fp );
	}

	for (int r = 0; r < 16; r++)
		for (int g = 0; g < 16; g++)
			for (int b = 0; b < 16; b++)
			{
				unsigned char	cr = r<<4;
				unsigned char	cg = g<<4;
				unsigned char	cb = b<<4;
				fwrite( &cr, 1, 1, fp );
				fwrite( &cg, 1, 1, fp );
				fwrite( &cb, 1, 1, fp );
			}*/


	for (int y = 0; y < 256; y++)
		for (int x = 0; x < 256; x++)
		{
			unsigned char	cr = x&0xF0;
			unsigned char	cg = ((x+y)/2)&0xF0;
			unsigned char	cb = y&0xF0;
			fwrite( &cr, 1, 1, fp );
			fwrite( &cg, 1, 1, fp );
			fwrite( &cb, 1, 1, fp );
		}
	
	fclose( fp );
}