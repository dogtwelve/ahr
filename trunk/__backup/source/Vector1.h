#ifndef		_VECTOR_
#define		_VECTOR_

class Vector1{
	void* pnt[1000];
	int length;

public:
	void removeAllElements();
	int size();
	void* elementAt(int pos);
	void insertElementAt(void*, int pos);
	void* firstElement();
	void removeLastElement();
};

#endif