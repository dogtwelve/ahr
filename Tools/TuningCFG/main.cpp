
// vcc sux
#define for if (false); else for
#pragma warning(disable:4786)

#include <windows.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define DEFINITION_FILE		"Game\\intl_Lang_arm.hpp"
#define CHAMPIONSHIP_FILE	"Game\\champion.txt"


/////////////////////////////////////////////////////////////////////
// helpers

string blanks = " \t\n\r";

string clean(const string& s)
{
	return string(&s[s.find_first_not_of(blanks)], &s[s.find_last_not_of(blanks)] + 1);
}

vector<unsigned> split_ints(string s)
{
	vector<unsigned> result;

	int start = 0;
	int end = -1;
	do
	{
		start = end + 1;
		end = s.find(',', start);
		string su(s, start, end - start);
		istringstream is(su);
		unsigned v;
		if (!(is >> v))
		{
			assert(!"expecting an integer");
		}
		result.push_back(v);
	}
	while (!(end == string::npos || end >= s.size()));

	return result;
}

string read_string(istream& o)
{
	string s;
	o >> s;
	if (!s.empty() && s[0] == '\"')
	{
		if (s[s.size() - 1] != '\"')
		{
			do
			{
				string s2;
				o >> s2;
				s2 = clean(s2);
				if (!s2.empty())
					s += " " + s2;
			}
			while (s[s.size() - 1] != '\"');
		}
		return string(s, 1, s.size() - 2);
	}
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

ostream& write_uint(ostream& o, unsigned i)
{
	o.write((const char*)&i, sizeof(i));
	return o;
}

unsigned padding(unsigned n)
{
	unsigned pad = 4 - n % 4;
	return pad == 4 ? 0 : pad;
}

unsigned padded(unsigned n)
{
	return n + padding(n);
}

ostream& write_padding(ostream& o, unsigned n)
{
	unsigned len = padding(n);
	while (len--)
		write_uchar(o, 0);
	return o;
}

/////////////////////////////////////////////////////////////////////
// structures

struct Piece
{
	string		code;
	string		txt;
	int	old_unlock_idx;
	int	unlock_idx;
	int	modifier1;
	int	modifier2;
	int	modifier3;
	int	modifier4;
};

struct Prize
{
	string			 txt;
	vector<unsigned> pieces;
};

struct Category
{
	string			 txt;
	string			 description;
	vector<unsigned> pieces;
};

struct Dealer
{
	string			 name;
	vector<unsigned> cars;
};

/////////////////////////////////////////////////////////////////////
// expected structures

struct TuningPiece
{
	unsigned		id;			// txt id
	const char*		code;		// code in tuning.ini
	char	old_unlock_idx;
	char	unlock_idx;
	char	modifier1_type;
	char	modifier1_value;
	char	modifier2_type;
	char	modifier2_value;
};

struct TuningKit
{
	unsigned short	id;			// txt id
	unsigned short	pieces_nb;
	unsigned char*	pieces;		// idx in TuningPiece
};

struct TuningCategory
{
	unsigned short  id;				// txt id
	unsigned short  description_id;	// description txt id
	unsigned		pieces_nb;
	unsigned char*	pieces;			// idx in TuningPiece
};

struct CarDealer
{
	const char*		name;		// txt id
	unsigned		cars_nb;
	unsigned char*	cars;		// idx in Cars
};

struct TuningSettings
{
	unsigned short	pieces_nb;
	unsigned short	kits_nb;
	unsigned short	categories_nb;
	unsigned short	dealers_nb;

	TuningPiece*	pieces;
	TuningKit*		kits;
	TuningCategory*	categories;
	CarDealer*		dealers;
};
/////////////////////////////////////////////////////////////////////
// 

int		main( int argc, char** argv )
{
	if (argc != 3)
	{
		cout << "usage: " << argv[0] << " input.txt output.cfg" << endl;
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
//			exit(0);
		}
	}
	//-----------------------------------------------

	unsigned max_trans = 0;

	// find translation max id
	{
		ifstream defs(DEFINITION_FILE);
		if (defs.fail())
		{
			cout << "open definition file failed : " DEFINITION_FILE " : " << strerror(errno) << endl;
			return -1;
		}

		// find BEGIN_STRINGS
		string s;
		bool got_header = false;
		while (!got_header && getline(defs, s))
		{
			if (s.size() > 0)
			{
				s = clean(s);
				if (s == "BEGIN_STRINGS")
					got_header = true;
			}
		}
		if (!got_header)
		{
			cout << DEFINITION_FILE " : invalid definition file (no header)." << endl;
			return -1;
		}

		bool got_footer = false;
		while (!got_footer && getline(defs, s))	
		{
			if (s.size() > 0)
			{
				s = clean(s);
				if (s.find("END_STRINGS") == 0)
					got_footer = true;
				else
					if (s.find("STRING_(") == 0)
						++max_trans;
			}
		}
	}
	{
		ifstream champs(CHAMPIONSHIP_FILE);
		if (champs.fail())
		{
			cout << "open championship file failed : " CHAMPIONSHIP_FILE " : " << strerror(errno) << endl;
			return -1;
		}

		bool in_categories   = false;
		bool in_championship = false;
		bool in_multichamp	 = false;
		bool in_ARCADE	 = false;
		string s;

		while (getline(champs, s))
		{
			if (s.size() > 0)
				s = clean(s);

			// comments
			if (s.empty())
				continue;
			if (string(s, 0, 2) == "//")
				continue;

			if (s == "[CATEGORY]")
			{
				in_categories	= true;
				in_championship = false;
				in_multichamp	= false;
				in_ARCADE = false;
				continue;
			}
			if (s == "[CHAMPIONSHIP]")
			{
				in_categories	= false;
				in_championship = true;
				in_ARCADE = false;
				in_multichamp	= false;
				continue;
			}
			if (s == "[CHAMPIONSHIP_MULTI]")
			{
				in_categories	= false;
				in_championship = false;
				in_ARCADE = false;
				in_multichamp	= true;
				continue;
			}
			if (s == "[ARCADE]")
			{
				in_categories	= false;
				in_championship = false;
				in_ARCADE = true;
				in_multichamp	= false;
				continue;
			}
			if (s == "[END]")
			{
				in_categories	= false;
				in_championship = false;
				in_ARCADE = false;
				in_multichamp	= false;
				continue;
			}

			if (	in_categories
				||	in_championship
				||	in_multichamp
				|| in_ARCADE
				)
			{
				++max_trans;
				continue;
			}
		}
	}

	--max_trans;

	{

		ifstream input(argv[1]);
		if (input.fail())
		{
			cout << "open input file failed : " << argv[1] << " : " << strerror(errno) << endl;
			return -1;
		}

		ofstream output(argv[2], ofstream::binary);
		if (output.fail())
		{
			cout << "open output file failed : " << argv[2] << " : " << strerror(errno) << endl;
			return -1;
		}

		// read

		vector<Piece>	 pieces;
		vector<Prize>	 prizes;
		vector<Category> categories;
		vector<Dealer>	 dealers;

		bool in_pieces	 = false;
		bool in_prize	 = false;
		bool in_category = false;
		bool in_dealers	 = false;

		string s;
		while (getline(input, s))
		{
			if (s.size() > 0)
				s = clean(s);

			// skip comments
			if (s.empty())
				continue;
			if (string(s, 0, 2) == "//")
				continue;

			if (s == "[PIECES]")
			{
				in_pieces	= true;
				in_prize	= false;
				in_category	= false;
				in_dealers	= false;
				continue;
			}
			if (s == "[PRIZE]")
			{
				in_pieces	= false;
				in_prize	= true;
				in_category	= false;
				in_dealers	= false;
				continue;
			}
			if (s == "[CATEGORY]")
			{
				in_pieces	= false;
				in_prize	= false;
				in_category	= true;
				in_dealers	= false;
				continue;
			}
			if (s == "[DEALERS]")
			{
				in_pieces	= false;
				in_prize	= false;
				in_category	= false;
				in_dealers	= true;
				continue;
			}
			if (s == "[END]")
			{
				in_pieces	= false;
				in_prize	= false;
				in_category	= false;
				in_dealers	= false;
				continue;
			}


			if (in_pieces)
			{
				istringstream is(s);
				
				// dummy
				unsigned i; is >> i;

				Piece p;
				is >> p.code;
				p.txt = read_string(is);

				is >> p.old_unlock_idx;
				is >> p.unlock_idx;
				is >> p.modifier1;
				is >> p.modifier2;
				is >> p.modifier3;
				is >> p.modifier4;
				
				pieces.push_back(p);
				continue ;
			}

			if (in_prize)
			{
				istringstream is(s);

				// dummy
				unsigned i; is >> i;

				Prize p;
				p.txt = read_string(is);
				string ps; is >> ps;
				p.pieces = split_ints(ps);

				prizes.push_back(p);
				continue ;
			}

			if (in_category)
			{
				istringstream is(s);

				// dummy
				unsigned i; is >> i;
				
				Category cat;
				cat.txt = read_string(is);
				cat.description = read_string(is);
				string ps; is >> ps;
				cat.pieces = split_ints(ps);

				if (cat.pieces.size() == 1 && cat.pieces[0] == 0)
					cat.pieces.clear();

				categories.push_back(cat);
				continue ;
			}

			if (in_dealers)
			{
				istringstream is(s);

				// dummy
				unsigned i; is >> i;
				
				Dealer d;
				d.name = read_string(is);
				string ps; is >> ps;
				d.cars = split_ints(ps);
				
				dealers.push_back(d);
				continue ;
			}
		}


		// dumping

		unsigned current = sizeof(TuningSettings);

		// TuningSettings
		{
			write_ushort(output, pieces.size());
			write_ushort(output, prizes.size());
			write_ushort(output, categories.size());
			write_ushort(output, dealers.size());

			write_uint(output, current);
			current += pieces.size() * sizeof(TuningPiece);

			write_uint(output, current);
			current += prizes.size() * sizeof(TuningKit);

			write_uint(output, current);
			current += categories.size() * sizeof(TuningCategory);

			write_uint(output, current);
			current += dealers.size() * sizeof(CarDealer);
		}

		for (unsigned i = 0; i < pieces.size(); ++i)
		{
			write_uint(output, ++max_trans);
			
			write_uint(output, current);
			current += padded(pieces[i].code.size() + 1);

			write_uchar(output, pieces[i].old_unlock_idx);
			write_uchar(output, pieces[i].unlock_idx);
			write_uchar(output, pieces[i].modifier1);
			write_uchar(output, pieces[i].modifier2);
			write_uchar(output, pieces[i].modifier3);
			write_uchar(output, pieces[i].modifier4);
			// alignment
			write_uchar(output, 0);
			write_uchar(output, 0);
			//write_uchar(output, 0);
		}

		for (unsigned i = 0; i < prizes.size(); ++i)
		{
			write_ushort(output, ++max_trans);
			write_ushort(output, prizes[i].pieces.size());
			write_uint(output, current);

			current += padded(prizes[i].pieces.size());
		}

		for (unsigned i = 0; i < categories.size(); ++i)
		{
			write_ushort(output, ++max_trans);
			write_ushort(output, ++max_trans);

			write_uint(output, categories[i].pieces.size());
			write_uint(output, current);
			
			current += padded(categories[i].pieces.size());
		}

		for (unsigned i = 0; i < dealers.size(); ++i)
		{
			write_uint(output, current);
			current += padded(dealers[i].name.size() + 1);
			write_uint(output, dealers[i].cars.size());
			write_uint(output, current);
			
			current += padded(dealers[i].cars.size());
		}

		for (unsigned i = 0; i < pieces.size(); ++i)
		{
			output << pieces[i].code;
			write_uchar(output, 0);
			write_padding(output, pieces[i].code.size() + 1);
		}

		for (unsigned i = 0; i < prizes.size(); ++i)
		{
			for (unsigned j = 0; j < prizes[i].pieces.size(); ++j)
				write_uchar(output, prizes[i].pieces[j] - 1);
			write_padding(output, prizes[i].pieces.size());
		}

		for (unsigned i = 0; i < categories.size(); ++i)
		{
			for (unsigned j = 0; j < categories[i].pieces.size(); ++j)
				write_uchar(output, categories[i].pieces[j] - 1);
			write_padding(output, categories[i].pieces.size());
		}

		for (unsigned i = 0; i < dealers.size(); ++i)
		{
			output << dealers[i].name;
			write_uchar(output, 0);
			write_padding(output, dealers[i].name.size() + 1);
			for (unsigned j = 0; j < dealers[i].cars.size(); ++j)
				write_uchar(output, dealers[i].cars[j] - 1);
			write_padding(output, dealers[i].cars.size());
		}

/*
		for (unsigned i = 0; i < pieces.size(); ++i)
		{
			cout << "got piece : " << pieces[i].code << " -> {" << pieces[i].txt << "}" << endl;
		}

		for (unsigned i = 0; i < prizes.size(); ++i)
		{
			cout << "got prize : " << prizes[i].txt << endl;
			for (unsigned j = 0; j < prizes[i].pieces.size(); ++j)
				cout << "   piece: " << pieces[prizes[i].pieces[j] - 1].code << endl;
		}

		for (unsigned i = 0; i < categories.size(); ++i)
		{
			cout << "got category : " << categories[i].txt << " -> " << categories[i].description << endl;
			for (unsigned j = 0; j < categories[i].pieces.size(); ++j)
				cout << "   piece: " << pieces[categories[i].pieces[j] - 1].code << endl;
		}

		for (unsigned i = 0; i < dealers.size(); ++i)
		{
			cout << "got dealer : " << dealers[i].name << endl;
			for (unsigned j = 0; j < dealers[i].cars.size(); ++j)
				cout << "   car: " << dealers[i].cars[j] << endl;
		}
*/		
	}

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