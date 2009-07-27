#include "Vector1.h"

void Vector1::removeAllElements()
{
	length = 0;
}

int Vector1::size(){
	return length;
}

void* Vector1::elementAt(int pos)
{
	return pnt[pos];
}
void Vector1::insertElementAt(void* pt, int pos)
{
	if( pos>=length )
	{
		pnt[length] = pt;
	}else{
		for( int pp=length; pp>pos; pp-- )
		{
			pnt[pp] = pnt[pp-1];
		}
		pnt[pos] = pt;
	}
	length++;
}

void* Vector1::firstElement()
{
	if( length > 0 ) return 0;
	return pnt[0];
}
void Vector1::removeLastElement()
{
	if( length > 0 ) length--;
	
}