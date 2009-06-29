#ifndef _OUTFILE_H_
#define _OUTFILE_H_

#include <stdio.h>
#include <vector>

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class OutFile
{
public:
	OutFile(const char* name)
	{
		m_file = fopen(name,"wb");
	}
	~OutFile()	{if(m_file)fclose(m_file);}

	void Write(const void*d,int size){fwrite(d,1,size,m_file);}
	void Write(int v) {Write(&v,sizeof(int));}


	template<class T>
	void Write(const std::vector<T>& v)
	{
		Write(v.size());
		Write(&v[0], sizeof(T) * v.size());
	}


	template<class T>
	void SaveElements(const T& v)
	{
		Write(v.size());
		for(T::const_iterator i = v.begin();i!=v.end();++i)
			(*i)->Save(*this);
	}

private:
	FILE*	m_file;
};
#endif // _OUTFILE_H_
