// IniExp.cpp : Defines the entry point for the console application.
//

#pragma warning(disable:4786)
#define for if(false) ; else for

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <windows.h>

using namespace std;

string clean(const string& s)
{
	string blanks = " \t\n\r";
	return string(&s[s.find_first_not_of(blanks)], &s[s.find_last_not_of(blanks)] + 1);
}

string clean_quotes(const string& s)
{
	if (s.size() < 2)
		return s;
	if (s[0] == '\"')
		if (s[s.size() - 1] == '\"')
			return string(s, 1, s.size() - 2);
	return s;
}

ostream& write_uchar(ostream& o, unsigned char i)
{
	o.write((const char*)&i, 1);
	return o;
}

ostream& write_ushort(ostream& o, unsigned short i)
{
	o.write((const char*)&i, 2);
	return o;
}

ostream& write_int(ostream& o, int i)
{
	o.write((const char*)&i, sizeof(i));
	return o;
}

ostream& write_uint(ostream& o, unsigned i)
{
	o.write((const char*)&i, sizeof(i));
	return o;
}

ostream& write_ptr(ostream& o, unsigned i)
{
	if (i % 4 != 0)
		cout << "argggg ptr = " << i << endl;
	o.write((const char*)&i, sizeof(i));
	return o;
}

unsigned adjust_size(unsigned i)
{
	unsigned result = i + ((4 - (i % 4)) % 4);
	if (result % 4 != 0)
		cout << "argggg size = " << result << endl;
	return result;
}

void padding(ostream& o, unsigned i)
{
	unsigned len = adjust_size(i);
	len -= i;
	while (len--)
		write_uchar(o, 0);
}


int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "usage: IniExp inFile.ini outFile.rc" << endl;
		return 0;
	}



	//-----------------------------------------------
	// check if the files have the same creation time
	char*	argv1 = argv[1];
	char*	argv2 = argv[2];
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
		//	exit(0);
		}
	}
	//-----------------------------------------------


	{
		ifstream input(argv1);
		if (input.fail())
		{
			cout << "open input file failed : " << argv1 << " : " << strerror(errno) << endl;
			return -1;
		}

		ofstream output(argv2, ofstream::binary);
		if (output.fail())
		{
			cout << "open output file failed : " << argv2 << " : " << strerror(errno) << endl;
			return -1;
		}
		
		map<string, int>	ints;
		map<string, string> strings;

		string s;
		while (getline(input, s))
		{
			s = clean(s);

			// comments
			if (s.empty())
				continue;
			if (string(s, 0, 1) == "[")
				continue;
			if (string(s, 0, 2) == "//")
				continue;

			// key = value
			int pos = s.find('=');
			if (pos == string::npos)
				continue;

			string key = clean(string(s, 0, pos));
			string value = clean(string(s, pos+1, string::npos));
			
			istringstream is(value);
			int v;
			if (is >> v)
				ints[key] = v;
			else
				strings[key] = clean_quotes(value);
		}

		write_ushort(output, ints.size());
		write_ushort(output, strings.size());

		unsigned current = 4 + 2 * sizeof(void*) * (ints.size() + strings.size());

		if (current % 4 != 0)
			cout << "failed at the beginning = " << current << endl;
 
		for (map<string, int>::iterator it = ints.begin();
			 it != ints.end(); ++it)
		{
			write_ptr(output, current);
			current += adjust_size(it->first.size() + 1);
			write_int(output, it->second);
		}

		for (map<string, string>::iterator it = strings.begin();
			 it != strings.end(); ++it)
		{
			write_ptr(output, current);
			current += adjust_size(it->first.size() + 1);
			write_ptr(output, current);
			current += adjust_size(it->second.size() + 1);
		}

		for (map<string, int>::iterator it = ints.begin();
			 it != ints.end(); ++it)
		{
			output << it->first;
			write_uchar(output, 0);
			padding(output, it->first.size() + 1);
		}

		for (map<string, string>::iterator it = strings.begin();
			 it != strings.end(); ++it)
		{
			output << it->first;
			write_uchar(output, 0);
			padding(output, it->first.size() + 1);

			output << it->second;
			write_uchar(output, 0);
			padding(output, it->second.size() + 1);
		}
	}


	//--------------------------------------------------------
	// set the same creation time for the 2 files (out = in)
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
	//---------------------------------------------------------




	return 0;
}

