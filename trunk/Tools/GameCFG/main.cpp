#include <windows.h>
#include <stdio.h>
#include <io.h>

#include "parser.h"

#pragma warning(disable : 4996)

#define MAX_CARS			128
#define MAX_OPPONENTS		8

int		main( int argc, char** argv )
{
	if (argc != 3)
	{
		printf("Usage : GAMECFG inputfilename outputCFGfilename\n\n");
		return -1;
	}		


	////-----------------------------------------------
	//// check if the files have the same creation time
	//{
	//	bool	doit1 = true;

	//	HANDLE	hfile = CreateFile( argv[1], GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	//	if (hfile != INVALID_HANDLE_VALUE)
	//	{
	//		BY_HANDLE_FILE_INFORMATION	fileinfo1;
	//		GetFileInformationByHandle( hfile, &fileinfo1 );
	//		CloseHandle( hfile );

	//		hfile = CreateFile( argv[2], GENERIC_READ, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	//		if (hfile != INVALID_HANDLE_VALUE)
	//		{
	//			BY_HANDLE_FILE_INFORMATION	fileinfo2;
	//			GetFileInformationByHandle( hfile, &fileinfo2 );
	//			CloseHandle( hfile );

	//			if ((fileinfo1.ftLastWriteTime.dwLowDateTime == fileinfo2.ftCreationTime.dwLowDateTime) &&
	//				(fileinfo1.ftLastWriteTime.dwHighDateTime == fileinfo2.ftCreationTime.dwHighDateTime))
	//				doit1 = false;
	//		}
	//	}	

	//	if (doit1 == false)
	//	{
	//		//printf( "%s : Skipped (no changes)\n", argv[1] );
	//		exit(0);
	//	}
	//}
	//-----------------------------------------------



	// ===============================================
	// CONFIG
	// ===============================================

	// cars

	FILE*	fin = fopen( argv[1], "rt" );

	if (fin == NULL)
	{
		printf("== FILE : %s == ", argv[4] );
		printf("Input File Error!\n\n");
		return -1;
	}

	char	line[1024];

	char	txt[256];
	txt[0] = 0;

	// find tracks
	while (strcmp( txt, "[TRACKS]" ) )
		fscanf( fin, "%s\n", txt );
	
	int		track_nb = 0;
	char	track[128][128];
	int		track_unlock[128];
	while (1)
	{
		fscanf( fin, "%s\n", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;
		// get car list
		int	d;
		sscanf( txt, "%d", &d );

		fscanf( fin, "%s %i\n", track[track_nb], &(track_unlock[track_nb]) );
		if (strcmp( track[track_nb], "[END]" ) == 0)
			break;
		track_nb++;
	}

	// find cars
	while (strcmp( txt, "[CARS]" ) )
		fscanf( fin, "%s\n", txt );
	
	int		cars_nb = 0;
	char	cars[128][128];
	int		cars_unlock[128];
	int		cars_old_unlock[128];
	int		cars_price[128];

	char	nitroPos[128][2];
	memset(nitroPos[0], 0, 128);
	memset(nitroPos[1], 0, 128);

	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		if (strcmp(p, "[END]" ) == 0)
			break;

		parserBegin(p, " \t,;");

		int index = 0;
		while (parserGetString(txt))
		{
			switch (index)
			{
			case 0:
				break; // skip car id
			case 1:
				strcpy(cars[cars_nb], txt);
				break;
			case 2:
				sscanf(txt, "%d", &cars_old_unlock[cars_nb]);
				break;
			case 3:
				sscanf(txt, "%d", &cars_unlock[cars_nb]);
				break;
			case 4:
				sscanf(txt, "%d", &cars_price[cars_nb]);
				break;
			case 5:
			case 6:
				sscanf(txt, "%d", &nitroPos[cars_nb][index - 5]);
				break;
			}

			index++;
		}

		cars_nb++;
	}

	// find cars opponents
	while (strcmp( txt, "[CARS_OPPONENTS]" ) )
		fscanf( fin, "%s\n", txt );
	
	int		carId;

	unsigned char opponents[MAX_CARS][MAX_OPPONENTS];
	for (int i=0; i<cars_nb; i++)
		memset(opponents[i], 0, MAX_OPPONENTS);

	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		if (strcmp(p, "[END]" ) == 0)
			break;

		parserBegin(p, " \t,;");

		int nOpponents = 0;
		int index = 0;
		while (parserGetString(txt))
		{
			switch (index)
			{
			// car index
			case 0:
				sscanf(txt, "%d", &carId);
				--carId; // indexed from 0
				if (carId < 0 || carId >= cars_nb)
				{
					printf("Error: car index is out of bounds %d\n", carId);
					carId = 0;
				}
				opponents[carId][0] = 1; // 1 opponent for now				
				opponents[carId][1] = carId; // we have at least 1 opponent of the same type
				nOpponents++;
				break;
			default:
				int opIndex = -1;
				for (int i=0; i<cars_nb; i++)
					if (!strcmp(cars[i], txt))
					{
						opIndex = i;
						break;
					}

				if (opIndex < 0)
				{
					printf("Error: opponent car %s not found in cars list\n", txt);
					opIndex = 0;
				}

				if (1 + nOpponents >= MAX_OPPONENTS)
				{
					printf("Warning: reaching max number of opponents for car %s\n", cars[carId]);
				}
				else
				{
					opponents[carId][1 + nOpponents] = opIndex;
					nOpponents++;
				}
			}

			++index;
		}

		opponents[carId][0] = nOpponents;
	}

	// find points
	while (strcmp( txt, "[POINTS]" ) )
		fscanf( fin, "%s\n", txt );
	
	int	points_nb = 0;
	int points_table[128*4];
	while (1)
	{
		fscanf( fin, "%s\n", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;

		sscanf( txt, "%d", &(points_table[points_nb*4+0]) );
		fscanf( fin, "%d", &(points_table[points_nb*4+1]) );
		fscanf( fin, "%d", &(points_table[points_nb*4+2]) );
		fscanf( fin, "%d", &(points_table[points_nb*4+3]) );

		char comment[128];
		fscanf( fin, "%s\n", comment );
		if (strcmp( comment, "[END]" ) == 0)
			break;

		points_nb++;
	}

	// find points
	while (strcmp( txt, "[CRIME]" ) )
		fscanf( fin, "%s\n", txt );
	
	int	crime_levels_nb = 0;
	int crime_levels_table[128];
	while (1)
	{
		fscanf( fin, "%s\n", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;

		sscanf( txt, "%d", &(crime_levels_table[crime_levels_nb]) );

		crime_levels_nb++;
	}


	// find singlerace rewards
	while (strcmp( txt, "[SINGLERACE_REWARDS]" ) )
		fscanf( fin, "%s\n", txt );
	
	int	SINGLERACE_REWARDS_num_rank = 0;
	int SINGLERACE_REWARDS[128];
	while (1)
	{
		fscanf( fin, "%s\n", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;

		sscanf( txt, "%d", &(SINGLERACE_REWARDS[SINGLERACE_REWARDS_num_rank]) );

		SINGLERACE_REWARDS_num_rank++;
	}

	// find event type
	while (strcmp( txt, "[EVENT_TYPE]" ) )
		fscanf( fin, "%s\n", txt );

	char szEventTypes[16][128];
	int nEventTypes = 0;
	while (1)
	{
		fscanf( fin, "%s\n", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;

		strcpy(szEventTypes[nEventTypes], txt);

		++nEventTypes;
	}
	
	while (strcmp( txt, "[CITY_EVENTS]" ) )
		fscanf( fin, "%s\n", txt );

	int cityEvents[64][8];
	int nCityEvents = 0;
	
	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		if (strcmp(p, "[END]" ) == 0)
			break;

		parserBegin(p, " \t,;");

		int index = 0;
		while (parserGetString(txt))
		{
			switch (index)
			{
			// city event
			case 0:
				// Find event
				cityEvents[nCityEvents][0] = 0;
				for (int k=0; k<nEventTypes; k++)
				{
					if (!strcmp(szEventTypes[k], txt))
					{
						cityEvents[nCityEvents][1] = k;
						cityEvents[nCityEvents][0]++;
						break;
					}
				}				
				break;

			default:
				if (cityEvents[nCityEvents][0] == 0)
					break;

				sscanf(txt, "%d", &cityEvents[nCityEvents][1 + index]);
				cityEvents[nCityEvents][0]++;

				break;
			}
			++index;
		}

		++nCityEvents;
	}

	while (strcmp( txt, "[GIRLS]" ) )
		fscanf( fin, "%s\n", txt );

	int nGirls = 0;
	int girls[64];
	int old_girls_unlock[64];
	int tmp;

	while (1)
	{
		fgets(line, 1023, fin);
		if (!line)
		{
			printf("Unexpected end of file\n");
			break;
		}

		char *p = trimLine(line);		
		if (!p || !*p)
			continue;

		if (strcmp(p, "[END]" ) == 0)
			break;

		sscanf(p, "%d %d %d", &tmp, &old_girls_unlock[nGirls], &girls[nGirls]);

		++nGirls;
	}


	// find Unlock Table
	while (strcmp( txt, "[UNLOCK_TABLE]" ) )
		fscanf( fin, "%s\n", txt );
	
	int	UNLOCK_TABLE_size = 0;
	int UNLOCK_TABLE[128];
	while (1)
	{
		fscanf( fin, "%s\n", txt );
		if (strcmp( txt, "[END]" ) == 0)
			break;

		sscanf( txt, "%d", &(UNLOCK_TABLE[UNLOCK_TABLE_size]) );

		UNLOCK_TABLE_size++;
	}

	fclose( fin );

	FILE*	fout = fopen( argv[2], "wb" );
	if (fout == NULL)
	{
		printf("== FILE : %s == ", argv[4] );
		printf("Error opening output file\n\n");
		return -1;
	}

	unsigned char	idx;
	int i;

	idx = track_nb;
	fwrite( &idx, 1, 1, fout );
	for (i = 0; i < track_nb; i++)
	{
		idx = strlen( track[i] );
		fwrite( &idx, 1, 1, fout );
		fwrite( track[i], idx, 1, fout );
		fwrite( &(track_unlock[i]), sizeof(track_unlock[i]), 1, fout );
	}

	idx = cars_nb;
	fwrite( &idx, 1, 1, fout );
	for (i = 0; i < cars_nb; i++)
	{
		idx = strlen( cars[i] );
		fwrite( &idx, 1, 1, fout );
		fwrite( cars[i], idx, 1, fout );		
		fwrite( &(cars_old_unlock[i]), sizeof(cars_old_unlock[i]), 1, fout );
		fwrite( &(cars_unlock[i]), sizeof(cars_unlock[i]), 1, fout );
		fwrite( &(cars_price[i]), sizeof(cars_price[i]), 1, fout );
		// nitro pos
		fwrite( &nitroPos[i], 1, 2, fout );
	}

	for (i = 0; i < cars_nb; i++)
	{
		fwrite( opponents[i], (opponents[i][0] + 1), 1, fout );
	}

	idx = points_nb;
	fwrite( &idx, 1, 1, fout );
	for (i = 0; i < points_nb; i++)
	{
		short money = 	points_table[i*4+0];
		short nitro = 	points_table[i*4+1];
		short crime = 	points_table[i*4+2];
		short prior =	points_table[i*4+3];
		fwrite( &money, sizeof(short), 1, fout );
		fwrite( &nitro, sizeof(short), 1, fout );
		fwrite( &crime, sizeof(short), 1, fout );
		fwrite( &prior, sizeof(short), 1, fout );
	}

	idx = crime_levels_nb;
	fwrite( &idx, 1, 1, fout );
	for (i = 0; i < crime_levels_nb; i++)
	{
		short crime_level = crime_levels_table[i];
		fwrite( &crime_level, sizeof(short), 1, fout );
	}

	idx = SINGLERACE_REWARDS_num_rank;
	fwrite( &idx, 1, 1, fout );
	for (i = 0; i < SINGLERACE_REWARDS_num_rank; i++)
	{
		int reward = SINGLERACE_REWARDS[i];
		fwrite( &reward, sizeof(int), 1, fout );
	}

	// City events
	idx = nCityEvents;
	fwrite( &idx, 1, 1, fout );
	for (i=0; i<nCityEvents; i++)
	{
		idx = cityEvents[i][0];
		fwrite( &idx, 1, 1, fout );
		for (int k=0; k < idx; k++)
		{
			int evt = cityEvents[i][k + 1];
			fwrite( &evt, sizeof(int), 1, fout );
		}
	}

	// Girls
	idx = nGirls;
	fwrite( &idx, 1, 1, fout );
	for (i=0; i<nGirls; i++)
	{
		idx = old_girls_unlock[i];
		fwrite( &idx, 1, 1, fout );
		idx = girls[i];
		fwrite( &idx, 1, 1, fout );
	}

	idx = UNLOCK_TABLE_size;
	fwrite( &idx, 1, 1, fout );
	for (i = 0; i < UNLOCK_TABLE_size; i++)
	{
		int reward = UNLOCK_TABLE[i];
		fwrite( &reward, sizeof(int), 1, fout );
	}

	fclose( fout );



	////--------------------------------------------------------
	//// set the same creation time for the 2 files (out = in)
	//{
	//	HANDLE	hfile = CreateFile( argv[1], GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	//	if (hfile != INVALID_HANDLE_VALUE)
	//	{
	//		FILETIME	time;
	//		GetFileTime( hfile, NULL, NULL, &time );
	//		CloseHandle( hfile );

	//		hfile = CreateFile( argv[2], GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	//		if (hfile != INVALID_HANDLE_VALUE)
	//		{
	//			SetFileTime( hfile, &time, &time, &time );
	//			CloseHandle( hfile );
	//		}
	//	}	
	//}
	////---------------------------------------------------------

	return 0;
}