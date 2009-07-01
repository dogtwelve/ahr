#include "gendef.h"
#include "Intl.h"
#include "File.h"

void unload_language(unsigned short**& intl_strings)
{
	if (intl_strings)
	{
		MM_DELETE intl_strings;
		intl_strings = 0;
	}
}

void load_language(const char* lang, unsigned short**& intl_strings)
{
	//bool isUnicode = (strcmp(lang, "jp") == 0) || (strcmp(lang, "ar") == 0);
	//use unicode for all languages
	bool isUnicode = true;
	
	unload_language(intl_strings);

	A_IFile* input = A_IFile::Open((std::string("Game\\lang") + lang + ".txt").c_str(), A_IFile::OPEN_READ | A_IFile::OPEN_BINARY, false);
	A_ASSERT(input);
	
	unsigned int nb_strings;
	input->Read(&nb_strings, 4);
	
	int dataSize = input->GetSize() - 4 * (1 + nb_strings);
	if (!isUnicode) // we keep non unicode strings on 2 bytes also
		dataSize *= 2;
		
	char* fileData = STATIC_NEW char[nb_strings * 4 + dataSize];
	unsigned int fileSize = input->GetSize() - 4;
	input->Read(fileData, fileSize); // reading the rest of file
	
	intl_strings = (unsigned short**)fileData;
	
	if (isUnicode)
	{
		for (unsigned int i = 0; i < nb_strings; ++i)
			intl_strings[i] = (unsigned short*)((char*)intl_strings + (unsigned int)intl_strings[i] - 4);
	}
	else
	{
		// converting from byte* to short* data
		char* s = fileData + nb_strings * 4; // stop pointer
		char* p = fileData + fileSize; // src
		short* q = (short*)(fileData + nb_strings * 4 + dataSize); // dest
		
		p--;
		q--;
		
		while (s <= p)
			*(q--) = (short)*(p--) & 0xFF;
		
		for (unsigned int i = 0; i < nb_strings; ++i)
			intl_strings[i] = (unsigned short*)((char*)intl_strings + (((unsigned int)intl_strings[i] << 1) - (nb_strings << 2) - 8));
	}

	A_IFile::Close(input);
}

void load_language(int L, unsigned short**& intl_strings)
{
	const char* langs[] = { "en", "fr", "de", "it", "sp", "jp", "ar" };
	load_language(langs[L], intl_strings);
}
