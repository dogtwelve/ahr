#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>

enum EToken
{
	TBlockName,
	TOpenBlock,
	TCloseBlock,

	TEOF,
	TUnknown,	
};






// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Parser
{
public:
	Parser(const char* fileName);
	~Parser();

	void		ReadToken(const char*);
	void		ReadToken(EToken);
	EToken		ReadToken();
	int			ReadInt();
	float		ReadFloat();
	void		SkipBlock();
	void		SkipBlockList();
	

	const char* Get()			const {return m_buffer;}

	int			LineNumber()	const {return m_line;}
	int			Level()			const {return m_level;}

	bool		operator==(EToken e) const {return m_token==e;}
	bool		operator!=(EToken e) const {return !operator==(e);}



	bool		IsBlock(const std::string& str) const {if(m_token!=TBlockName) Throw("Should Be block Name");return str==m_buffer;}

	bool		operator==(const std::string& str) const {return IsBlock(str);}
	bool		operator!=(const std::string& str) const {return !IsBlock(str);}

	void		Throw(const char*,const char* msg2=NULL)const;
	

private:
	void		SkipSpace();

	int			ReadChar();
	
	bool		IsSpace() const	{return ::isspace(m_char)!=0;}
	bool		IsEOF() const		{return m_char==EOF;}



private:
	FILE*	m_file;
	int		m_char;
	int		m_level;
	int		m_line;
	EToken	m_token;
	char	m_buffer[1024];
};
















#endif // _PARSER_H_
