
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

string blanks = " \t\n\r";

string clean(const string& s)
{
	return string(&s[s.find_first_not_of(blanks)], &s[s.find_last_not_of(blanks)] + 1);
}

// debug pouet
unsigned current_champ_line = 0;
unsigned current_champ_epreuve = 0;

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
			cerr << "line : " << current_champ_line << " epreuve : " << current_champ_epreuve << endl;
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

struct category
{
	unsigned id;
	string	 name;
	vector<unsigned> cars;
};

struct ChampionshipGroup
{
	unsigned id;
	string	 name;
	vector<unsigned> championshipIds;
};

struct epreuve
{
	enum race_type
	{
		normal_race = 1,
		duel_race	= 2,
		golden_race	= 3,
		radar_race	= 4,
	} type;

	unsigned	track;
	bool		reverse;
	bool		with_traffic;

	unsigned	nb_laps;		// 0 == infinite (for radar)
	unsigned	nb_opponents;
	unsigned	bonus;

	unsigned	difficulty;
	unsigned	amount_bet;
	unsigned	section_begin;
	unsigned	section_end;
	unsigned	speed_begin;
	unsigned	speed_end;

	friend istream& operator>>(istream& i, epreuve& e)
	{
		string s;
		if (!(i >> s))
			return i;
		vector<unsigned> params = split_ints(s);
		if (params.size() != 8)
			cerr << "line : " << current_champ_line << " epreuve : " << current_champ_epreuve << endl;
		assert(params.size() == 8);

		e.track   = params[0];
		e.reverse = params[1] ? true : false;
		e.type	  = static_cast<race_type>(params[2]);
		switch (e.type)
		{
		case normal_race:
			e.with_traffic	= params[4] ? true : false;

			e.nb_laps		= params[3];
			e.nb_opponents	= params[6];
			e.bonus			= params[7];

			e.difficulty	= params[5];

			break;

		case duel_race:
			e.with_traffic	= params[4] ? true : false;

			e.nb_laps		= params[3];
			e.nb_opponents	= 1;
			e.bonus			= params[7];

			e.amount_bet	= params[6];
			e.difficulty	= params[5];
			break;

		case golden_race:
			e.with_traffic	= params[4] ? true : false;

			e.nb_laps		= params[3];
			e.nb_opponents	= params[6];
			e.bonus			= params[7];

			e.difficulty	= params[5];
			break;

		case radar_race:
			e.with_traffic	= false;

			e.nb_laps		= 0;
			e.nb_opponents	= 0;
			e.bonus			= params[7];

			e.section_begin	= params[3];
			e.section_end	= params[4];
			e.speed_begin	= params[5];
			e.speed_end		= params[6];
			break;
		}
		return i;
	}

	friend ostream& operator<<(ostream& o, epreuve& e)
	{
		switch (e.type)
		{
		case normal_race:
			cout << "normal race : "
				 << "track " << e.track << ", "
				 << (e.reverse ? "reverse" : "normal") << ", "
				 << e.nb_laps << " laps, "
				 << (e.with_traffic ? "with" : "without") << " traffic"
				 << ", difficulty " << e.difficulty
				 << ", " << e.nb_opponents << " players"
				 ;
			break;
		case duel_race:
			cout << "duel race : "
				 << "track " << e.track << ", "
				 << (e.reverse ? "reverse" : "normal") << ", "
				 << e.nb_laps << " laps, "
				 << (e.with_traffic ? "with" : "without") << " traffic"
				 << ", difficulty " << e.difficulty
				 << ", bet = " << e.amount_bet
				 << ", won = " << e.bonus
				 ;
			break;
		case golden_race:
			cout << "golden race : "
				 << "track " << e.track << ", "
				 << (e.reverse ? "reverse" : "normal") << ", "
				 << e.nb_laps << " laps, "
				 << (e.with_traffic ? "with" : "without") << " traffic"
				 << ", difficulty " << e.difficulty
				 << ", " << e.nb_opponents << " players"
				 ;
			break;
		case radar_race:
			cout << "radar race : "
				 << "track " << e.track << ", "
				 << (e.reverse ? "reverse" : "normal") << ", "
				 << "from " << e.section_begin << " to " << e.section_end
				 << ", from " << e.speed_begin << " to " << e.speed_end
				 << ", cash " << e.bonus
				 ;
			break;
		default:
			assert(!"unknown race type");
		}
		return o;
	}

	unsigned get_size()
	{
		switch (type)
		{
		case normal_race:	return 8 + 4;
		case duel_race:		return 8 + 4;
		case golden_race:	return 8 + 4;
		case radar_race:	return 8 + 8;
		default:
			assert(!"unknown race type");
		}
		return 0;
	}

	void dump(ostream& o)
	{
		write_uchar (o, type);
		write_uchar (o, track);
		write_uchar (o, reverse);
		write_uchar (o, with_traffic);

		write_uchar (o, nb_laps);
		write_uchar (o, nb_opponents);
		write_ushort(o, bonus);

		switch (type)
		{
		case normal_race:
			write_uchar (o, difficulty);
			write_uchar (o, 0);
			write_uchar (o, 0);
			write_uchar (o, 0);
			break;
		case duel_race:
			write_ushort(o, amount_bet);
			write_uchar (o, difficulty);
			write_uchar (o, 0);
			break;
		case golden_race:
			write_uchar (o, difficulty);
			write_uchar (o, 0);
			write_uchar (o, 0);
			write_uchar (o, 0);
			break;
		case radar_race:
			write_ushort(o, section_begin);
			write_ushort(o, section_end);
			write_ushort(o, speed_begin);
			write_ushort(o, speed_end);
			break;
		default:
			cout << "weird : " << type << endl;
			assert(!"unknown race type");
		}
	}
};

struct championship
{
	unsigned id;
	string	 name;
	vector<unsigned> categories;
	vector<unsigned> locks;
	vector<epreuve>	 epreuves;
	unsigned		 reward_money[4];
	unsigned		 reward_car[4];
	unsigned		 reward_kit[4];
};

struct multichamp
{
	unsigned	id;
	string		name;
	unsigned	accepted_cars[6];
	struct race
	{
		unsigned track;
		bool	 reverse;
	} races[3];
};

struct CarCategory;
struct ChampionshipGroup;
struct Championship;
struct MultiplayerChampionship;

// [NOTE] this next struct must match the struct in the game
struct ChampionshipSettings
{
#define NB_ARCADE_CATEGORY	11
	unsigned short	arcade_categories_id[NB_ARCADE_CATEGORY];
	unsigned short	nb_multiplayer_championships;
	unsigned char*	arcade_categories[NB_ARCADE_CATEGORY];
	//Note: the first char of the pointed array is the number of elements in the category

	unsigned char				nb_categories;
	unsigned char				nb_championshipGroups;
	unsigned short				nb_championship;
	CarCategory*				categories;
	ChampionshipGroup*			championshipGroups;
	Championship*				championships;
	MultiplayerChampionship*	multiplayer_championship;
};

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

	unsigned max_trans;

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
			s = clean(s);
			if (s == "BEGIN_STRINGS")
				got_header = true;
		}
		if (!got_header)
		{
			cout << DEFINITION_FILE " : invalid definition file (no header)." << endl;
			return -1;
		}

		max_trans = 0;

		bool got_footer = false;
		while (!got_footer && getline(defs, s))	
		{
			s = clean(s);
			if (s.find("END_STRINGS") == 0)
				got_footer = true;
			else
				if (s.find("STRING_(") == 0)
					++max_trans;
		}

	}

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

		vector<category>	 categories;
		vector<ChampionshipGroup>	 groups;
		vector<championship> championships;
		typedef pair<string, vector<unsigned> > arcade_category_type;
		vector<arcade_category_type>	arcade_categories;
		vector<multichamp>		multiplayer_championships;

		bool in_categories	 = false;
		bool in_championship = false;
		bool in_rewards		 = false;
		bool in_arcade		 = false;
		bool in_multichamp	 = false;
		bool in_groups		 = false;

		string s;
		while (getline(input, s))
		{
			s = clean(s);

			// skip comments
			if (s.empty())
				continue;
			if (string(s, 0, 2) == "//")
				continue;

//			cout << "state:" << in_categories << "|" << in_championship << " got:" << s << endl;

			if (s == "[CATEGORY]")
			{
				in_categories	= true;
				in_championship = false;
				in_rewards		= false;
				in_arcade		= false;
				in_multichamp	= false;
				in_groups		= false;
				continue;
			}
			if (s == "[CHAMPIONSHIP]")
			{
				in_categories	= false;
				in_championship = true;
				in_rewards		= false;
				in_arcade		= false;
				in_multichamp	= false;
				in_groups		= false;
				continue;
			}
			if (s == "[CHAMPIONSHIP_MULTI]")
			{
				in_categories	= false;
				in_championship = false;
				in_rewards		= false;
				in_arcade		= false;
				in_multichamp	= true;
				in_groups		= false;
				continue;
			}			
			if (s == "[REWARDS]")
			{
				in_categories	= false;
				in_championship = false;
				in_rewards		= true;
				in_arcade		= false;
				in_multichamp	= false;
				in_groups		= false;
				continue;
			}
			if (s == "[ARCADE]")
			{
				in_categories	= false;
				in_championship = false;
				in_rewards		= false;
				in_arcade		= true;
				in_multichamp	= false;
				in_groups		= false;
				continue;
			}
			if (s == "[CHAMPIONSHIPGROUP]")
			{
				in_categories	= false;
				in_championship = false;
				in_rewards		= false;
				in_arcade		= true;
				in_multichamp	= false;
				in_groups		= true;
				continue;
			}
			if (s == "[END]")
			{
				in_categories	= false;
				in_championship = false;
				in_rewards		= false;
				in_arcade		= false;
				in_multichamp	= false;
				in_groups		= false;
				continue;
			}

			if (in_categories)
			{
				category cat;

				istringstream is(s);
				is >> cat.id;

				string rest = clean(string(s, s.find(' '), string::npos));
				
				cat.name = string(rest, 1, rest.find('\"', 1) - 1);
				cat.cars = split_ints(clean(string(rest, rest.find('\"', 1) + 1, string::npos)));

				categories.push_back(cat);
				continue;
			}
						
			if( in_groups) 
			{
				ChampionshipGroup group;

				istringstream is(s);
				is >> group.id;
				
				string rest = clean(string(s, s.find('\"'), string::npos));
				
				group.name = string(rest, 1, rest.find('\"', 1) - 1);
				group.championshipIds = split_ints(clean(string(rest, rest.find('\"', 1) + 1, string::npos)));

				groups.push_back(group);
				continue;
			}

			if (in_championship)
			{
				championship champ;
				{
					istringstream is(s);
					is >> champ.id;
					current_champ_line = champ.id;
				}

				string rest = string(s, s.find_first_not_of(blanks, 2), string::npos);
				champ.name = string(rest, 1, rest.find('\"', 1) - 1);

				string rest2 = string(rest, rest.find('\"', 1) + 1, string::npos);
				istringstream is(rest2);

				string cats;
				is >> cats;
				champ.categories = split_ints(cats);

				string locks;
				is >> locks;
				champ.locks = split_ints(locks);

//				cout << "=-=-=-=-=-=-=-> " << champ.name << " <==" << endl;
				epreuve ep;
				current_champ_epreuve = 1;
				while (is >> ep)
				{
//					cout << "got epreuve " << current_champ_epreuve << " -> " << ep << endl;
					champ.epreuves.push_back(ep);
					++current_champ_epreuve;
				}

				championships.push_back(champ);
				continue;
			}

			if (in_multichamp)
			{
				multichamp	  champ;

				istringstream is(s);
				string cars;
				string races[3];

				is >> champ.id;
				champ.name = read_string(is);
				is >> cars >> races[0] >> races[1] >> races[2];

				vector<unsigned> pouet = split_ints(cars);
				assert(pouet.size() <= 6 && "too many cars in multiplayer championship");

				unsigned i = 0;
				for (; i < pouet.size(); ++i)
					champ.accepted_cars[i] = pouet[i];
				for (; i < 6; ++i)
					champ.accepted_cars[i] = 0;

				for (i = 0; i < 3; ++i)
				{
					pouet = split_ints(races[i]);
					assert(pouet.size() == 2 && "race format incorrect");

					champ.races[i].track   = pouet[0];
					champ.races[i].reverse = pouet[1] == 1;
				}

				multiplayer_championships.push_back(champ);
				continue;
			}

			if (in_rewards)
			{
				istringstream is(s);

				unsigned champ_nb;
				string reward[4];

				is >> champ_nb >> reward[0] >> reward[1] >> reward[2] >> reward[3];

				for (unsigned i = 0; i < 4; ++i)
				{
					vector<unsigned> rewards = split_ints(reward[i]);
					assert(rewards.size() == 3);
					championships[champ_nb].reward_money[i] = rewards[0];
					championships[champ_nb].reward_car[i]	= rewards[1];
					championships[champ_nb].reward_kit[i]	= rewards[2];
				}
				continue;
			}

			if (in_arcade)
			{
				istringstream is(s);

				arcade_category_type pouet;
				pouet.first	 = read_string(is);
				string a; is >> a;
				pouet.second = split_ints(a);
				arcade_categories.push_back(pouet);
				continue;
			}


		}


		assert(arcade_categories.size() == NB_ARCADE_CATEGORY);

		// dumping
		
		// header
		unsigned current = sizeof(ChampionshipSettings);

		for (unsigned i = 0; i < arcade_categories.size(); ++i)
		{
			write_ushort(output, max_trans++);
		}

		write_ushort(output, multiplayer_championships.size());

		// padding
		if (arcade_categories.size() % 2 == 0)
		{
			write_ushort(output, 0);
		}

		for (unsigned i = 0; i < arcade_categories.size(); ++i)
		{
			write_uint(output, current); // 
			current += arcade_categories[i].second.size() + 1;
		}

		current += padding(current);

		write_uchar(output, categories.size());
		write_uchar(output, groups.size());
		write_ushort(output, championships.size());

		write_uint(output, current); // categories*
		current += categories.size() * 8;

		write_uint(output, current); // groups*
		current += groups.size() * 8;

		write_uint(output, current); // championships*
		current += championships.size() * (12 + 4 * (2+1+1) + 8);

		write_uint(output, current); // multiplayer championships*
		current += multiplayer_championships.size() * 20;


		// arcade categories

		long pos = output.tellp();

		{
			unsigned count = 0;
			for (unsigned i = 0; i < arcade_categories.size(); ++i)
			{
				write_uchar (output, arcade_categories[i].second.size());
				++count;
				for (unsigned j = 0; j < arcade_categories[i].second.size(); ++j)
				{
					write_uchar (output, arcade_categories[i].second[j]);
					++count;
				}
			}
			for (unsigned i = 0; i < padding(count); ++i)
				write_uchar(output, 0);
		}

		// categories*
		for (unsigned i = 0; i < categories.size(); ++i)
		{
			write_ushort(output, max_trans++); // category txt id
			write_ushort(output, categories[i].cars.size());
			write_uint  (output, current); // cars*
			current += categories[i].cars.size() + padding(categories[i].cars.size());
		}
		// groups*
		for (unsigned i = 0; i < groups.size(); ++i)
		{
			write_ushort(output, i); // group txt id
			write_ushort(output, groups[i].championshipIds.size());
			write_uint  (output, current); // ids*
			current += groups[i].championshipIds.size() + padding(groups[i].championshipIds.size());
		}

		// championships*
		for (unsigned i = 0; i < championships.size(); ++i)
		{
			write_ushort(output, max_trans++); // championship txt id			
			write_uchar (output, championships[i].categories.size());
			write_uchar (output, championships[i].epreuves.size());
			write_uint	(output, championships[i].locks.size());

			write_uint  (output, current); // cats*
			current += championships[i].categories.size() + padding(championships[i].categories.size());

			write_uint  (output, current); // epreuves**
			current += championships[i].epreuves.size() * 4;

			write_uint	(output, current); // locks*
			current += championships[i].locks.size() + padding(championships[i].locks.size());

			write_ushort(output, championships[i].reward_money[0]);
			write_ushort(output, championships[i].reward_money[1]);
			write_ushort(output, championships[i].reward_money[2]);
			write_ushort(output, championships[i].reward_money[3]);
			write_uchar (output, championships[i].reward_car[0]);
			write_uchar (output, championships[i].reward_car[1]);
			write_uchar (output, championships[i].reward_car[2]);
			write_uchar (output, championships[i].reward_car[3]);
			write_uchar (output, championships[i].reward_kit[0]);
			write_uchar (output, championships[i].reward_kit[1]);
			write_uchar (output, championships[i].reward_kit[2]);
			write_uchar (output, championships[i].reward_kit[3]);
		}

		// multiplayer championships
		for (unsigned i = 0; i < multiplayer_championships.size(); ++i)
		{
			write_ushort(output, max_trans++); // multichamp txt id
			for (unsigned j = 0; j < 6; ++j)
				write_uchar(output, multiplayer_championships[i].accepted_cars[j]);

			for (unsigned j = 0; j < 3; ++j)
			{
				write_uchar (output, multiplayer_championships[i].races[j].track);
				write_uchar (output, multiplayer_championships[i].races[j].reverse);
				write_ushort(output, 0); // padding
			}			
		}
        

		//categories
		for (unsigned i = 0; i < categories.size(); ++i)
		{
			for (unsigned j = 0; j < categories[i].cars.size(); ++j)
				write_uchar (output, categories[i].cars[j]);
			for (unsigned j = 0; j < padding(categories[i].cars.size()); ++j)
				write_uchar (output, 0);
		}
		
		///groups
		for (unsigned i = 0; i < groups.size(); ++i)
		{
			for (unsigned j = 0; j < groups[i].championshipIds.size(); ++j)
				write_uchar (output, groups[i].championshipIds[j]);
			for (unsigned j = 0; j < padding(groups[i].championshipIds.size()); ++j)
				write_uchar (output, 0);
		}


		for (unsigned i = 0; i < championships.size(); ++i)
		{
			for (unsigned j = 0; j < championships[i].categories.size(); ++j)
				write_uchar (output, championships[i].categories[j] - 1);
			for (unsigned j = 0; j < padding(championships[i].categories.size()); ++j)
				write_uchar (output, 0);

			for (unsigned j = 0; j < championships[i].epreuves.size(); ++j)
			{
				write_uint  (output, current); // epreuves*
				current += championships[i].epreuves[j].get_size();
			}

			for (unsigned j = 0; j < championships[i].locks.size(); ++j)
				write_uchar(output, championships[i].locks[j]);
			for (unsigned j = 0; j < padding(championships[i].locks.size()); ++j)
				write_uchar(output, 0);
		}

		for (unsigned i = 0; i < championships.size(); ++i)
			for (unsigned j = 0; j < championships[i].epreuves.size(); ++j)
				championships[i].epreuves[j].dump(output);


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

