#ifndef _ARRAY_H_
#define _ARRAY_H_


#include "defines.h"
#include <string.h>


namespace notstl
{
	template<class T>	inline void swap(T& a,T& b)	{	const T c = a;a = b;b=c;}


// Should not be used with complex type 
//	(ie: classes with constructor/destructor)
// If you want to do that, use a real STL
template<class T>
class	vector
{
	enum 
	{
		kMin = 5
	};

	public:
		typedef T*				v_iterator;
		typedef const T*	const_iterator;
		typedef T*				Ptr;

		vector():m_ptr(0),m_last(0),m_reserved(0){};

		~vector()	{delete m_ptr;};

		void reserve(int newsize)
		{			
			if(newsize > m_reserved)
			{
				const int usedSize = size();
				Ptr newvec = NEW T[newsize];
				if(m_ptr)
					memcpy(newvec,m_ptr,usedSize*sizeof(T));
				
                notstl::swap(newvec,m_ptr);
				delete[] newvec;
				m_last = m_ptr + usedSize;
				m_reserved = newsize;
			}
		}

		void SetSize(int i)	{ reserve(i);m_last = m_ptr + i;}

		void push_back(const T& t)
		{
			if(size()==m_reserved)
			{
				if(size()<kMin)
					reserve(kMin);
				else
					reserve(size()*size());
			}
				*m_last = t;
			m_last++;
		}

		T&				operator[](int x)				{A_ASSERT(x>=0 && x<size());return m_ptr[x];}
		const T&	operator[](int x)const	{A_ASSERT(x>=0 && x<size());return m_ptr[x];}

		void									clear(){	m_last = m_ptr;}

		inline int						size() const {return m_last - m_ptr;}

		inline v_iterator				begin()	{return m_ptr;}
		inline v_iterator				end()		{return m_last;}

		inline const_iterator begin()	const {return m_ptr;}
		inline const_iterator end()		const {return m_last;}

		T&										back()				{A_ASSERT(size()>0);return *(m_last-1);}
		const T&							back() const	{A_ASSERT(size()>0);return *(m_last-1);}

	private:
		Ptr	m_ptr;		
		Ptr	m_last;
		int	m_reserved;
};


template<class T>
class owning_vector
	:public vector<T>
{
	typedef vector<T> inherited;
	public:
	~owning_vector()
		{
			v_iterator i;

			for(v_iterator i = begin();i!=end();++i)
				delete *i;
		}

	void clear() 
	{ 
		for(v_iterator i = begin();i!=end();++i)
				delete *i;
		inherited::clear();
	}


};

}// namespace


#endif // _ARRAY_H_
