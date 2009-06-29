
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <windows.h>

using namespace std;

#define for if(false); else for

struct PositionSample
{
	unsigned frame;
	float	 x, y, z;
};

struct Camera
{
	vector<PositionSample> positions[2];
};

ostream& write_uint(ostream& o, unsigned i)
{
	o.write((const char*)&i, sizeof(i));
	return o;
}

float read_uint(istream& in)
{
	unsigned i;
	in.read((char*)&i, sizeof(i));
	return i;
}

ostream& write_fixed(ostream& o, float f)
{
	int i = f * 4096;
	o.write((const char*)&i, sizeof(i));
	return o;
}

float read_fixed(istream& in)
{
	int i;
	in.read((char*)&i, sizeof(i));
	return i / 4096.;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "usage: CameraExp inFile_1.ase [inFile_2.ase] ... [inFile_N.ase] outFile.bin" << endl;
		return 0;
	}

	//-----------------------------------------------
	// check if the files have the same creation time
	/*
	char*	argv1 = argv[1];
	char*	argv2 = argv[2];
	if (strcmp( argv[1], "-d") != 0)
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
	*/
	//-----------------------------------------------

	{
		int ticksPerFrame = 0;

		ofstream output(argv[argc-1], ofstream::binary);
		if (output.fail())
		{
			cout << "open output file failed : " << strerror(errno) << endl;
			return -1;
		}

		write_uint(output, argc - 2);
		cout << argc - 2 << endl;

		for (int j = 1; j < argc - 1; j++)
		{
			ifstream input(argv[j]);

			if (input.fail())
			{
				cout << "open input file failed \"" << argv[j] << "\" : " << strerror(errno) << endl;
				return -1;
			}

			Camera cam;

			string s;

			int dummy_count = 0;

			while (getline(input, s))
			{
				if (s.find("*SCENE_TICKSPERFRAME") != string::npos)
				{
					string prefix = "*SCENE_TICKSPERFRAME ";
					string s2 = s.substr(s.find(prefix) + prefix.size());
					istringstream is(s2);
					is >> ticksPerFrame;
					continue;
				}

				if (s.find("*CONTROL_POS_TRACK {") != string::npos)
				{
					string s;
					while (getline(input, s))
					{
						if (s.find("}") != string::npos)
							break;
						string prefix = "*CONTROL_POS_SAMPLE ";
						string s2 = s.substr(s.find(prefix) + prefix.size());
						istringstream is(s2);
						PositionSample p;
						is >> p.frame >> p.x >> p.z >> p.y;
						cam.positions[dummy_count].push_back(p);
					}
					dummy_count++;
				}
			}

			write_uint(output, cam.positions[0].size());
			cout << cam.positions[0].size() << endl;

			if (dummy_count == 2)
			{
				for (int i = 0; i < cam.positions[0].size(); i++)
				{
					int tx = (cam.positions[0][i].x - cam.positions[1][i].x)/2;
					int ty = (cam.positions[0][i].y - cam.positions[1][i].y)/2;
					int tz = -(cam.positions[0][i].z - cam.positions[1][i].z)/2;
					int t = cam.positions[0][i].frame / ticksPerFrame;

					int x = tx;
					int y = ty;
					int z = tz;
					
					write_uint(output, x);
					write_uint(output, y);
					write_uint(output, z);
					write_uint(output, t);

					cout << x << "\t" << y << "\t" << z << "\t" << t << endl;
				}
			}
			else if (dummy_count == 1)
			{
				for (int i = 0; i < cam.positions[0].size(); i++)
				{
					int tx = (cam.positions[0][i].x - 0)/2;
					int ty = (cam.positions[0][i].y - 0)/2;
					int tz = -(cam.positions[0][i].z - 0)/2;
					int t = cam.positions[0][i].frame / ticksPerFrame;

					int x = tx;
					int y = ty;
					int z = tz;

				
					write_uint(output, x);
					write_uint(output, y);
					write_uint(output, z);
					write_uint(output, t);

					cout << x << "\t" << y << "\t" << z << "\t" << t << endl;
				}
			}
			else
			{
				cout << "Error" << endl;
			}
		}

		output.close();
	}

	//--------------------------------------------------------
	// set the same creation time for the 2 files (out = in)
	/*
	if (strcmp( argv1, "-d") != 0)
	{
		HANDLE	hfile = CreateFile( argv1, GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if (hfile != INVALID_HANDLE_VALUE)
		{
			FILETIME	time;
			GetFileTime( hfile, NULL, NULL, &time );
			CloseHandle( hfile );

			hfile = CreateFile( argv2, GENERIC_WRITE, 0, NULL,  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hfile != INVALID_HANDLE_VALUE)
			{
				SetFileTime( hfile, &time, &time, &time );
				CloseHandle( hfile );
			}
		}	
	}
	*/
	//---------------------------------------------------------

	return 0;
}