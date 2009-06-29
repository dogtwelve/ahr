#include <windows.h>
#include <stdio.h>
#include <io.h>


int		main( int argc, char** argv )
{
	if (argc != 3)
	{
		printf("Usage : IACFG inputfilename outputCFGfilename\n\n");
		return -1;
	}		


	//-----------------------------------------------
	// check if the files have the same creation time
	{
		bool	doit1 = true;

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

		if (doit1 == false)
		{
			//printf( "%s : Skipped (no changes)\n", argv[1] );
			exit(0);
		}
	}
	//-----------------------------------------------


	// ===============================================
	// CONFIG
	// ===============================================

	FILE*	fin = fopen( argv[1], "rt" );

	if (fin == NULL)
	{
		printf("== FILE : %s == ", argv[4] );
		printf("Input File Error!\n\n");
		return -1;
	}

	FILE*	fout = fopen( argv[2], "wb" );
	if (fout == NULL)
	{
		printf("== FILE : %s == ", argv[4] );
		printf("Error opening output file\n\n");
		return -1;
	}

	char	txt[128];
	txt[0] = 0;

	unsigned i;

	for (unsigned j = 0; j < 10; ++j)
	{
		char buffer[16];
		strcpy(buffer, "[IA0]");
		buffer[3] += j;

		// find IA
		while (strcmp( txt, buffer ) )
			fscanf( fin, "%s\n", txt );
		int	speed[5];
		fscanf( fin, "%d %d %d %d %d\n", &speed[0], &speed[1], &speed[2], &speed[3], &speed[4] );
		int	errorfac[8];
		fscanf( fin, "%d %d %d %d %d %d %d %d\n", &errorfac[0], &errorfac[1], &errorfac[2], &errorfac[3], &errorfac[4], &errorfac[5], &errorfac[6], &errorfac[7] );
		int	erroradd[6];
		fscanf( fin, "%d %d %d %d %d %d\n", &erroradd[0], &erroradd[1], &erroradd[2], &erroradd[3], &erroradd[4], &erroradd[5] );
		int	turbo[6];
		fscanf( fin, "%d %d %d\n", &turbo[0], &turbo[1], &turbo[2] );

		for (i = 0; i < 5; i++)
		{
			signed short	s = speed[i];
			fwrite( &s, 2, 1, fout );
		}
		for (i = 0; i < 8; i++)
		{
			signed short	s = errorfac[i];
			fwrite( &s, 2, 1, fout );
		}
		for (i = 0; i < 6; i++)
		{
			signed short	s = erroradd[i];
			fwrite( &s, 2, 1, fout );
		}
		for (i = 0; i < 3; i++)
		{
			signed short	s = turbo[i];
			fwrite( &s, 2, 1, fout );
		}
	}


	fclose( fin );

	fclose( fout );



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






	return 0;
}