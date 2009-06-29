// to_upper_case.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "ctype.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if( argc == 2 )
	{	
		int i = 0;
		char *text;

		text = argv[1];
		while( text[i] != '\0' )
		{
			text[i] = toupper( text[i] );
			i++;
		}

		printf( "%s", text );
	}
	else if( argc == 4 )
	{
		int i = 0;
		char *text;

		text = argv[2];
		while( text[i] != '\0' )
		{
			text[i] = toupper( text[i] );
			i++;
		}

		printf( "%s %s %s", argv[1], text, argv[3] );

	}

 
	return 0;
}

