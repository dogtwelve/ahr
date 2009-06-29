#include "Parser.h"
#include "Exception.h"

#include <sstream>

const int kOpenBlock = '{';
const int kCloseBlock = '}';
const int kBlockName = '*';
const int kEndOfLine = '\n';


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Parser::Parser(const char* fileName)
{
	m_char=0;
	m_level=0;
	m_line=1;
	m_file = ::fopen(fileName,"rt");
	m_token = TUnknown;

	if(m_file==NULL)
		throw Exception( "Can't open file:",fileName);
	ReadChar();
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
Parser::~Parser()
{
	if(m_file)
		::fclose(m_file);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Parser::ReadToken(const char* c)
{
	ReadToken();
	if(::strcmp(m_buffer,c)!=0)
		Throw("Expecting: ",c);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Parser::ReadToken(EToken t)
{
	ReadToken();
	if(m_token!=t)
		Throw("Wrong Token");
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
EToken Parser::ReadToken()
{
	SkipSpace();
	int i=0;
	while(!IsSpace() && !IsEOF())
	{
		m_buffer[i]=m_char;
		i++;
		ReadChar();
	}
	m_buffer[i] = '\0';

	m_token = TUnknown;

	if(i==0)
		m_token = TEOF;

	if(m_buffer[0]==kBlockName)
		m_token = TBlockName;

	if(m_buffer[0]==kOpenBlock)
		m_token = TOpenBlock;


	if(m_buffer[0]==kCloseBlock)
		m_token = TCloseBlock;
	
	

	return m_token;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Parser::SkipBlock()
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
//
// ---------------------------------------------------------------------------
void Parser::SkipBlockList()
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
//
// ---------------------------------------------------------------------------
int Parser::ReadChar()
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
//
// ---------------------------------------------------------------------------
void Parser::SkipSpace()
{
	while(IsSpace() && !IsEOF())
		ReadChar();
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int Parser::ReadInt()
{
	ReadToken(TUnknown);
	
	int val;
	if(sscanf(m_buffer,"%d",&val)==1)	
		return val;

	Throw("Int Expected");
	return 0;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
float Parser::ReadFloat()
{
	ReadToken(TUnknown);
	
	float val;
	if(sscanf(m_buffer,"%f",&val)==1)	
		return val;

	Throw("Float Expected");

	return 0;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Parser::Throw(const char* c,const char* msg2) const
{
	std::ostringstream str;

	str << c;
	if(msg2)
		str << " " << msg2;

	str << " Line: " << m_line << " Level: " << m_level;
	throw Exception(str.str());
}




