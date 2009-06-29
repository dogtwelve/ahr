#include <windows.h>
#include <stdio.h>
#include <io.h>

int		main( int argc, char** argv )
{
	if (argc != 3)
	{
		printf("Usage : ARCHIVEMERGE inputname outputname\n\n");
		printf("\n");
		return (-1);
	}

	char	namein1[256];
	char	namein2[256];

	sprintf(namein1, "%s.OFF", argv[1] );
	sprintf(namein2, "%s.DAT", argv[1] );

	FILE*	fpin1 = fopen( namein1, "rb" );
	if (fpin1 == NULL)
	{
		printf("== FILE : %s == ", namein1 );
		printf("Error opening input file\n\n");
		return -1;
	}

	FILE*	fpin2 = fopen( namein2, "rb" );
	if (fpin2 == NULL)
	{
		printf("== FILE : %s == ", namein2 );
		printf("Error opening input file\n\n");
		return -1;
	}
	
	fseek( fpin1, 0, SEEK_END );
	int		size1 = ftell( fpin1 );
	fseek( fpin1, 0, SEEK_SET );

	
	fseek( fpin2, 0, SEEK_END );
	int		size2 = ftell( fpin2 );
	fseek( fpin2, 0, SEEK_SET );

	void*	ptr1 = malloc( size1 );
	void*	ptr2 = malloc( size2 );

	fread( ptr1, size1, 1, fpin1 );
	fread( ptr2, size2, 1, fpin2 );

	fclose( fpin1 );
	fclose( fpin2 );

	char	nameout[256];

	sprintf(nameout, "%s.ARC", argv[2] );

	FILE*	fpout = fopen( nameout, "wb" );
	if (fpout == NULL)
	{
		printf("== FILE : %s == ", nameout );
		printf("Error opening output file\n\n");
		return -1;
	}


	fwrite( &size1, sizeof(int), 1, fpout );
	fwrite( &size2, sizeof(int), 1, fpout );

	fwrite( ptr1, size1, 1, fpout );
	fwrite( ptr2, size2, 1, fpout );

	fclose(fpout);

	return 0;
}
