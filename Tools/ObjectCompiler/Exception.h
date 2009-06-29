#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <string>



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Exception
{
public:
	Exception(const std::string& str):m_msg(str){}

	Exception(const char* c,const char* c2=NULL)
	{
		m_msg = c;
		if(c2)
		{
			m_msg +=" "; 
			m_msg += c2;
		}
	}	
public:
	std::string m_msg;
};



#endif // _EXCEPTION_H_
