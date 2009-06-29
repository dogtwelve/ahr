#include "misc.h"
#include "exception.h"

#include <algorithm>




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ToLower(std::string& str)
{
	std::transform(str.begin(),str.end(),str.begin(),tolower);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void StripQuotes(std::string& str)
{
	const int lastQuote = str.rfind("\"");

	if(lastQuote!=std::string::npos)
		str = str.substr(0,lastQuote);

	const int firstQuote = str.find("\"");
	if(firstQuote!=std::string::npos)
		str = str.substr(firstQuote+1);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
std::string StripFileName(const std::string& fileName)
{
	using namespace std;

	const string::size_type extpos = fileName.rfind(".");
	if(extpos!=string::npos)
	{
		string noExt = fileName.substr(0,extpos);

		const string::size_type nameBegin = noExt.rfind("\\");
		if(nameBegin!= string::npos)
			noExt = noExt.substr(nameBegin+1);

		return noExt;
	}
	throw Exception("Bad File name:",fileName.c_str());
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
OutFileException::OutFileException(const char* msg,const char*msg2)
{
	m_msg = msg;
	if(msg2)
	{
		m_msg +=" ";
		m_msg + msg2;
	}
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
OutFile::OutFile(const std::string& fileName)
		:m_fileName(fileName)
{
	m_file = ::fopen(fileName.c_str(),"wb");
	if(!m_file)		
		Throw("Can't open file");
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
OutFile::~OutFile()
{
	if(m_file)
		::fclose(m_file);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void OutFile::WriteShort(int x)
{	
	if(x> SHRT_MAX || x < SHRT_MIN)
		Throw("Invalid Value");

	const short val = (short)x;
	Write(&val,sizeof(val));
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void OutFile::WriteChar(int x)
{	
	if(x> CHAR_MAX || x < CHAR_MIN)
		Throw("Invalid Value");

	const signed char val = (signed char)x;
	Write(&val,sizeof(val));
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void OutFile::WriteLong(int x)
{
	const long val = x;
	Write(&val,sizeof(val));
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void OutFile::Write(const void* data,int size)
{
	if(m_file)
	{
		if(::fwrite(data,size,1,m_file)!=1)
			Throw("Write Error");
	}
	else
		Throw("File Not Opened");
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void OutFile::Throw(const char* i_msg)
{
	std::string msg = std::string("Output Error: ") + i_msg + " " + m_fileName;
	throw OutFileException(msg.c_str());
}
