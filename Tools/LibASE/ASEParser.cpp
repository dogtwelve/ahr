#include "ASEParser.h"
#include "ASEException.h"

#include <sstream>
#include <assert.h>

const int kOpenBlock	= '{';
const int kCloseBlock	= '}';
const int kBlockName	= '*';
const int kEndOfLine	= '\n';
const int kStringDelimiter = '"';

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEParser::ASEParser(const char* fileName)
{
	m_char=0;
	m_level=0;
	m_line=1;
	m_file = ::fopen(fileName,"rt");
	m_token = TUnknown;

	if(m_file==NULL)
		throw ASEException( "Can't open file:",fileName);

	ReadChar();
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEParser::~ASEParser()
{
	if(m_file)
		::fclose(m_file);
}


// ---------------------------------------------------------------------------
// Read the next token, throw if the value of the token is not s
// ---------------------------------------------------------------------------
void ASEParser::ReadToken(const char* c)
{
	ReadToken();
	if(::strcmp(m_buffer,c)!=0)
		Throw("Expecting: ",c);
}


// ---------------------------------------------------------------------------
// Read the next token, throw if the token is not of the specified type
// ---------------------------------------------------------------------------
void ASEParser::ReadToken(EToken t)
{
	const EToken readToken = ReadToken();
	if(readToken != t)
		Throw("Wrong Token");
}


// ---------------------------------------------------------------------------
// read a token and return it's type
// ---------------------------------------------------------------------------
ASEParser::EToken ASEParser::ReadToken()
{
	SkipSpace();

	if(m_char==kStringDelimiter)
	{
		ReadToEndOfString();		
	}
	else
	{
		int length=0;
		while(!IsSpace() && !IsEOF())
		{
			if(length>sizeof(m_buffer))
				Throw("Token Size too long");

			m_buffer[length]=m_char;
			length++;
			ReadChar();
		}
		m_buffer[length] = '\0';

		// Identify the token
		m_token = TUnknown;

		if(length==0)
			m_token = TEOF;

		if(m_buffer[0]==kBlockName)
			m_token = TBlockName;

		if(m_buffer[0]==kOpenBlock)
			m_token = TOpenBlock;


		if(m_buffer[0]==kCloseBlock)
			m_token = TCloseBlock;
		
	}
	return m_token;
}

// ---------------------------------------------------------------------------
// private; read a token until the string delimiter (") is encoutered
// ---------------------------------------------------------------------------
void ASEParser::ReadToEndOfString()
{
	assert(m_char==kStringDelimiter);
	
	const int currentLine = m_line;
	int length = 0;

	ReadChar();
	while( m_char != kStringDelimiter )
	{
		if(length >= sizeof(m_buffer))
			Throw("String too long");

		if(IsEOF() || m_line != currentLine)
			Throw("String not closed");

		m_buffer[length] = m_char;
		length++;
		ReadChar();
	}
	m_buffer[length]='\0';

	ReadChar();// skip the last string delimiter

	m_token = TString;
}

// ---------------------------------------------------------------------------
// read a quote-delimited string; the quotes are removed; throw if not a valid string
// String cannot span two lines
// ---------------------------------------------------------------------------
const char*	ASEParser::ReadString()
{
	ReadToken(TString);	
	return m_buffer;
}


// ---------------------------------------------------------------------------
//	Skip the current block;
// ---------------------------------------------------------------------------
void ASEParser::SkipBlock()
{
	const int level = m_level;

	const int currentLine = m_line;
	while(ReadToken()!=TEOF)
	{
		if(m_token==TOpenBlock)
		{
			SkipBlockList();
			if(m_token!=TCloseBlock)
				Throw("CloseBlock Expected");
			break;
		}
		if(currentLine != m_line)
			break;
	}
	if(m_level!=level)
		Throw("Level mismatch");
}

// ---------------------------------------------------------------------------
// private; skip all of block contained inside a {} block
// ---------------------------------------------------------------------------
void ASEParser::SkipBlockList()
{
	while(ReadToken()!=TEOF)
	{
		if(m_token == TBlockName)
			SkipBlock();
		else
			return;
	}
}


// ---------------------------------------------------------------------------
// private; all char reading must be done in this function 
// ---------------------------------------------------------------------------
int ASEParser::ReadChar()
{
	m_char = ::fgetc(m_file);
	if(m_char==kEndOfLine)
		m_line++;
	if(m_char==kOpenBlock)
		m_level++;
	if(m_char==kCloseBlock)
		m_level--;
	return m_char;
}



// ---------------------------------------------------------------------------
//	private
// ---------------------------------------------------------------------------
void ASEParser::SkipSpace()
{
	while(IsSpace() && !IsEOF())
		ReadChar();
}


// ---------------------------------------------------------------------------
// read an int, throw if the next token is not a valid int
// ---------------------------------------------------------------------------
int ASEParser::ReadInt()
{
	ReadToken(TUnknown);
	
	int val;
	if(::sscanf(m_buffer,"%d",&val)!=1)	
		Throw("Int Expected");

	return val;
}

// ---------------------------------------------------------------------------
// read a floating-point value, throw if not a valid float
// ---------------------------------------------------------------------------
float ASEParser::ReadFloat()
{
	ReadToken(TUnknown);
	
	float val;

	if(::sscanf(m_buffer,"%f",&val)!=1)	
		Throw("Float Expected");

	return val;
}


// ---------------------------------------------------------------------------
// Check if the last block read have the specified name, throw if the last token read is not a block
// ---------------------------------------------------------------------------
bool ASEParser::IsBlock(const std::string& str) const 
{
	if(m_token!=TBlockName)
		Throw("Should Be block Name");
	return str==m_buffer;
}

// ---------------------------------------------------------------------------
// Make the parser throw an ASEException (including the current file name and line number)
// ---------------------------------------------------------------------------
void ASEParser::Throw(const char* c,const char* msg2) const
{
	std::ostringstream str;

	str << c;
	if(msg2)
		str << " " << msg2;

	str << " Line: " << m_line << " Level: " << m_level;

	throw ASEException(str.str());
}




