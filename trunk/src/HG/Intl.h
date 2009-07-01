
#ifndef HG_INTL_H
# define HG_INTL_H

#include "str_utils.h"

// internal
# define BEGIN_STRINGS	
# define END_STRINGS															

//#define TMP_HARCODED_STRINGS

enum
{
	LANGUAGE_EN = 0,
	LANGUAGE_FR,
	LANGUAGE_DE,
	LANGUAGE_IT,
	LANGUAGE_SP,
	LANGUAGE_JP,
	LANGUAGE_AR
};

#define IS_LANGUAGE_JP	(CHighGear::GetInstance()->m_Profile.language == LANGUAGE_JP)

struct intl_string_ids	
{
	// include definition
	// this is what you are looking for :)
	# ifndef __CW32__
		# define STRING_(VALUE, ID, DEFAULT)	enum { ID = VALUE };				
		# include "../../data/languages/intl_lang_arm.hpp" 
	#else
	enum 
	{
		# include "../../data/languages/intl_lang.hpp" 
	};
	#endif

#ifdef TMP_HARCODED_STRINGS
	enum 
	{
		TXT_FIRST_HARCODED = 10000,
		#undef STRING_
		#define STRING_(VALUE, ID, DEFAULT)  ID,	
		//#include "Intl_hardcoded.hpp"
		# include "../data/game/intl_lang.hpp" 
	};
#endif

};


/*
#ifdef __CW32__
		#undef STRING_
		# define STRING_(VALUE, ID, DEFAULT)	const int intl_string_ids::ID = VALUE;
		# include "../data/game/intl_lang.hpp" 		
		#undef STRING_
#endif
*/

// user interface
# ifdef __CW32__
	# define txt_(ID)	(get_text(0))
	# define txt_id(ID)	(0)	
	# define txt_int(ID) (get_text(ID))
# else
	# define txt_(ID)	(get_text(intl_string_ids::ID))
	# define txt_id(ID)	(intl_string_ids::ID)
	# define txt_int(ID) (get_text(ID))
#endif


// internal
#include "HighGear.h"
inline const unsigned short* get_text(unsigned int id)
{
#ifdef TMP_HARCODED_STRINGS
	if( id > intl_string_ids::TXT_FIRST_HARCODED)
	{
		switch( id )
		{
		#undef STRING_
		#define STRING_(ID,DEFAULT)  case intl_string_ids::ID: return DEFAULT; 
		#include "Intl_hardcoded.hpp"

		
		default: return "???";
		}
	}
#endif
	//TEST
	//return CHighGear::GetInstance()->intl_strings[id]; 
}

// house keeping
void load_language(const char* lang, unsigned short**& intl_strings);
void unload_language(unsigned short**& intl_strings);
void load_language(int L, unsigned short**& intl_strings);

#endif // !HG_INTL_H
