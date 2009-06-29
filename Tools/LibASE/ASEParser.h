#ifndef _ASEPARSER_H_
#define _ASEPARSER_H_

#include <string>

// ---------------------------------------------------------------------------
// Open an ASE File and parse it one token at a time
//
//	All error throw an ASEException object; all operation with an ASEParser 
//	(including construction) must be in a try-catch block
//
//	Sould not be used directly by the application; the parser should be
//	hidden in the ASEReader object.
//
//
//	Note:	This file was developed for clarity and ease of use, 
//			not performance; please keep it that way
// ---------------------------------------------------------------------------
class ASEParser
{
public:
	enum EToken
	{
		TBlockName,		// The first token of a block ( in the form "*BLOCKNAME")
		TOpenBlock,		// {
		TCloseBlock,	// }
		TString,		// a quote-delimited string, with the quotes ommited
		
		TEOF,			// not exactly a token, but the End-Of-File indicator
		TUnknown,		// Any other type of block (int, float etc)
	};

	ASEParser(const char* fileName);		// note: Throw if the file can't be opened
	virtual ~ASEParser();
	
	void		ReadToken(const char* s);	// Read the next token, throw if the value of the token is not s
	void		ReadToken(EToken);			// Read the next token, throw if the token is not of the specified type
	EToken		ReadToken();				// read a token and return it's type
	int			ReadInt();					// read an int, throw if the next token is not a valid int
	float		ReadFloat();				// read a floating-point value, 
	const char*	ReadString();				// read a quote-delimited string; the quotes are removed; throw if not a valid string
	void		SkipBlock();				// read and discard token until the current block is closed


	const char* Get()			const {return m_buffer;}	// return the textual value of the last token read

	int			LineNumber()	const {return m_line;}		// current file line number (mostly for debugging purpose)
	int			Level()			const {return m_level;}		// Level of block inclusion

	bool		operator==(EToken e) const {return m_token==e;}		// check if the last token was of the specified type
	bool		operator!=(EToken e) const {return !operator==(e);}

	bool		IsBlock(const std::string& blockName) const;	// Check if the last block read have the specified name, throw if the last token read is not a block

	bool		operator==(const std::string& str) const {return IsBlock(str);}
	bool		operator!=(const std::string& str) const {return !IsBlock(str);}

	void		Throw(const char*,const char* msg2=NULL)const;	// Make the parser throw an ASEException (including the current file name and line number)

private:
	void		SkipBlockList();
	void		SkipSpace();
	void		ReadToEndOfString();

	int			ReadChar();
	
	bool		IsSpace() const	{return ::isspace(m_char)!=0;}
	bool		IsEOF() const		{return m_char==EOF;}

private:
	FILE*	m_file;
	int		m_char;				// Last character read
	int		m_level;			// Current block imbeding level
	int		m_line;				// Line number in the file, mostly for issuing readeable error messages
	EToken	m_token;			// Type of the last token read
	char	m_buffer[1024];		// Literal value of the last token read
};

#endif // _ASEPARSER_H_
