#ifndef _ASEEXCEPTION_H_
#define _ASEEXCEPTION_H_

#include <string>



// ---------------------------------------------------------------------------
//	
// ---------------------------------------------------------------------------
class ASEException
{
public:
	ASEException(const std::string& str):m_msg(str)
	{
		#ifndef NDEBUG
			_asm int 3
		#endif
	}

	ASEException(const char* c,const char* c2=NULL)
	{
		#ifndef NDEBUG
			_asm int 3
		#endif

		m_msg = c;
		if(c2)
		{
			m_msg +=" "; 
			m_msg += c2;
		}
	}


	const std::string& GetMessage() const {return m_msg;}

private:
	std::string m_msg;
};





#endif // _ASEEXCEPTION_H_
