#ifndef _ASEMAP_H_
#define _ASEMAP_H_

#include <string>

class ASEParser;

// ---------------------------------------------------------------------------
//	Contains information about one texture map
// ---------------------------------------------------------------------------
class ASEMap
{
public:
	ASEMap(ASEParser&);


	const std::string& GetName() const {return m_fileName;}

private:
	std::string	m_fileName;
};



#endif // _ASEMAP_H_
