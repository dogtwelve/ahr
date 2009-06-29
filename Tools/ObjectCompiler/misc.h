#ifndef _MISC_H_
#define _MISC_H_



#include <string>

#include <fstream>



extern std::ofstream gTrace;

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class OutFileException
{
public:
	OutFileException(const char* msg,const char*msg2=NULL);

public:
	std::string m_msg;
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class OutFile
{
public:
	OutFile(const std::string& fileName);
	~OutFile();

	void WriteChar(int x);
	void WriteShort(int x);

	void WriteLong(int x);
	void Write(const void* data,int size);

	template<class T>
	void WriteList(const T& vec) 
	{
		if(gTrace)
			gTrace << T::value_type::ClassName() << " " << vec.size() << "\n";

		int nb = vec.size();
		WriteLong(nb);
		for(T::const_iterator i = vec.begin();i!=vec.end();i++)
		{
			if(gTrace)
			{
				gTrace << "\t"; 
				i->Debug();
				gTrace << "\n";
			}

				
			i->Save(*this);
		}
	}

	template<class T>
	void WriteListPtr(const T& vec) 
	{
		int nb= vec.size();
		WriteLong(nb);

		T::const_iterator i = vec.end();

		//gTrace << (*i)->ClassName() << " " << vec.size() << "\n";

		for(i = vec.begin();i!=vec.end();i++)
		{
			if(gTrace)
			{
				gTrace << "\t"; 
				(*i)->Debug();
				gTrace << "\n";
			}
			(*i)->Save(*this);
		}
	}

	void Throw(const char* i_msg);
private:
	const std::string	m_fileName;
	FILE*				m_file;	
};





void		ToLower(std::string&);
void		StripQuotes(std::string&);
std::string StripFileName(const std::string&);


#endif // _MISC_H_
