#pragma warning (disable:4786)
#include "unifyreader.h"
#include "unify.h"


namespace Unify {


Unify::CNode* CUnifyReader::Read(const char * filename){
	try {
		m_File = new std::ifstream(filename,std::ios::in);
		m_Line=1;
		c=0;
		m_NodeLevel=0;
		
		Unify::CNode * root = 0;
		
		if(m_File->is_open())
		{
			NextChar();
			// Skip white spaces
			while(::isspace(c))
				NextChar();

			if(c=='('){
				root = new Unify::CNode;
				NextChar();
				ReadNode(root);

			}else if(c=='<'){
				// New comment
				NextChar();
				//Advance cursor to begining of next token
				ReadComments();
			}

			m_File->close();
		}		
		
		delete m_File;
		return root;
	}

	catch(CUnifyException e){
		m_Exception=e;
		return 0;
	}
}


void CUnifyReader::ReadNode(Unify::CNode* node){
	m_NodeLevel++;

	// Cursor is on the first character of node

	while(!m_File->eof()) {
		bool skip=false;
	
		// Skip white spaces
		while(::isspace(c))
			NextChar();

		
		if(c=='('){
			// New node
			NextChar();
			//Advance cursor to begining of next token
			ReadNode(node->NewChild());


		}else if(c=='<'){
			// New comment
			NextChar();
			//Advance cursor to begining of next token
			ReadComments();


		}else if(IsRegularChar(c)||c=='\"'){
			// New parameter
			ReadParameter(node);

		}else if(c==')'){
			// End of node
			// Skip the node closing
			m_NodeLevel--;
			NextChar();
			return;

		}else{
			// Unknown character
			throw CUnifyException("Syntax error",m_Line);
		}


	}


	throw;
}


void CUnifyReader::ReadComments(){

	// Cursor is on the first character of comment node
	while(!m_File->eof()){
		if(c=='<'){
			// New comment
			NextChar();
			//Advance cursor to begining of next token
			ReadComments();

		}else if(c=='>'){
			// End of comments
			NextChar();
			return;
		} else{
			NextChar();
		}
	}


	throw;
}


void CUnifyReader::ReadParameter(Unify::CNode * node){
	Unify::CParameter * param;
	
	// A new parameter name is on cursor position
	// Read string at once
	
	if(IsRegularChar(c)){
		param=node->NewParameter(ReadString());
	}
	if(c=='\"'){
		param=node->NewParameter(ReadLiteralString());
	}

	// Cursor is after the string, No need to advance
	
	while(!m_File->eof()){
		
		// Skip white spaces
		while(::isspace(c))
			NextChar();

		if(IsRegularChar(c)){
			// Regular string
			param->NewValue()->SetString(ReadString());
			// Cursor is after string, No need to advance

		}else if(c=='\"'){
			// Literal string
			param->NewValue()->SetString(ReadLiteralString());
			// Cursor is after string, No need to advance

		}else{
			// Unknown character
			throw CUnifyException("Syntax error",m_Line);

		}

		
		// Skip white spaces
		while(::isspace(c))
			NextChar();

		// Are there more parameters?
		if(c!=',')
			return;

		// Cursor is on the comma, skip it
		NextChar();

	}


}



std::string CUnifyReader::ReadString(){

	// Cursor is on first letter, this is good
	std::string str;
	while(IsRegularChar(c)){
		str+=c;
		NextChar();
	}

	// Cursor is after the last letter, this is good

	return str;
}

std::string CUnifyReader::ReadLiteralString(){
	std::string str;

	// Cursor is on first quote, skip the quote
	NextChar();

	// Read string

	bool escape= c=='\\';

	if(escape){
		NextChar();
	}

	while(c!='\"' || escape){
		if(escape){
			if(c=='n'){
				str+="\r\n";
			}else if(c=='r'){
			}else if(c=='\\'){
				str+='\\';
			}else if(c=='t'){
				str+='\t';
			}else if(c=='"'){
				str+='\"';
			}else{
				// Unknown escape
				throw CUnifyException("Unknown escape code in literal string",m_Line);
			}
		}else{
			if(c!='\\'){
				str+=c;
			}
		}
		escape=c=='\\'&&!escape;
		NextChar();
		
		// Make sure there is no new line in literal
		if(c=='\n' || c=='\r') throw CUnifyException("End of line in literal string",m_Line);

	}

	// Skip last character so the cursor is after last quote
	NextChar();


	return str;
}


bool CUnifyReader::IsRegularChar(char c){
	return IsIn(c,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-+");
}


bool CUnifyReader::IsIn(char c,const char* in){
	/*bool ret=false;
	
	int i=0;
	const char * p=in;
	while(i<strlen(in) && c != *p){p++; i++;}

	return *p!=0;*/

	std::string str(1,c);
	return str.find_first_of(in)!=std::string::npos;
}

char CUnifyReader::NextChar(){
	m_File->get(c);

	if(c=='\n'){
		m_Line++;
	}

	if(m_File->eof() && m_NodeLevel>0){
		throw CUnifyException("Unexpected end of file",m_Line);
	}
	return c;
}


}