#ifndef _UNIFYWRITER_H_
#define _UNIFYWRITER_H_
#include <fstream>

namespace Unify {

class CUnifyWriter {
public:
	void Write(const char *, Unify::CNode*, bool brief=true);

protected:
	void WriteNode(Unify::CNode*);
	void WriteParameter(const std::string&  name,Unify::CParameter* param);
	void WriteToken(const std::string& token);

	
	
	int m_Tab;
	std::ofstream m_File;

	bool m_Brief;

	void EscapeString(std::string&);

};

}

#endif
