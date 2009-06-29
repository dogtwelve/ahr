#ifndef _UNIFYREADER_H_
#define _UNIFYREADER_H_
#include <fstream>
#include "unify.h"

namespace Unify {

	
	
class CNode;
class CParameter;



class CUnifyReader {
public:
	Unify::CNode* Read(const char *);

	const CUnifyException& GetException()const{return m_Exception;}
protected:

	void ReadNode(Unify::CNode* node);
	void ReadComments();
	void ReadParameter(Unify::CNode* node);
	bool IsRegularChar(char c);
	bool IsIn(char,const char*);
	char NextChar();
	std::string ReadString();
	std::string ReadLiteralString();


protected:
	
	std::ifstream * m_File;
	int m_Line;
	int m_NodeLevel;
	char c;

	CUnifyException m_Exception;

};


}
#endif
