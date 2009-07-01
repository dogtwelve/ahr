#ifndef _UTIL_H_
#define _UTIL_H_

#include "GenDef.h"

//#ifdef __BREW__
//	#include "memoryallocation.h"
//#endif

#include "DevUtil.h"
#include <string>
#include <stdarg.h>

class IGenericFile;	//for CTimeCounter's save.

class Util
{
public:

	template<class T>
	static inline void swap(T& v1, T&  v2){
		T const inter = v1;
		v1=v2;
		v2=inter;
	}

	template<class Iter>
	static inline void DeleteArray(Iter begin,Iter end)
	{
		for(Iter i = begin;i!=end;i++)
			MM_DELETE *i;		
	}


	template<class T>
	static inline void DeleteArray(T& v)
	{		
		DeleteArray(v.begin(),v.end());
		v.clear();
	}


	template <class T>
	static inline void Swap(T&A, T&B){
		T const I=A;
		A=B;
		B=I;
	}

	
	template <class T>
	static inline void TestAndSwap(T&A, T&B){
		if(A>B){
			Swap(A,B);
		}
	}

	template <class T>
	static inline int Middle(T& a,T& b){
		return a + ((b-a)>>1);
	}

	static int Lerp16(int m, int M, int f16){
		return m + (( f16 * (M-m) ) >>16);
	}

	static int LerpColor16(int m, int M, int f16){
		int ret=0;
		ret |= Lerp16( (m>>12)&0xf,	(M>>12)&0xf,f16 ) << 12; // A
		ret |= Lerp16( (m>>8)&0xf,	(M>>8)&0xf,	f16 ) << 8; // R
		ret |= Lerp16( (m>>4)&0xf,	(M>>4)&0xf,	f16 ) << 4; // G
		ret |= Lerp16(  m&0xf,		M&0xf,		f16 ) ; // B
		return ret;
	}

	static std::string ConvertIntToTime(int time) // in ms
	{
		char txt[16];
		char *ptxt = txt;
		time *= 5;
		if (time < 0)
		{
			*ptxt++ = '-';
			time = -time;
		}
		sprintf(ptxt, "%d%d:%d%d:%d%d",
				(time / 60000) % 10,
				(time / 6000) % 10,
				(time / 1000) % 6,
				(time / 100)% 10,
				(time / 10) % 10,
				time % 10);

		return std::string( txt );
	}

	static std::string ConvertIntToString(int intVal)
	{
		bool blnNegative = false;
		char chrpBuffer[16];
		int i = 1;

		if(intVal < 0)
		{
			blnNegative = true;
			intVal *= -1;
			i++;
			chrpBuffer[0] = '-';
		}

		int intOriginalVal = intVal;

		while(intVal / 10 > 0)
		{
			intVal /= 10;
			i++;
		}

		chrpBuffer[i] = 0;

		intVal = intOriginalVal;
		while(i > 1 || (i > 0 && !blnNegative))
		{
			i--;
			chrpBuffer[i] = (intVal % 10) + 48;
			intVal /= 10;
		}

		return std::string( chrpBuffer );
	}


	static std::string& ToUpper(std::string& str);
	static std::string& Trim(std::string& str);
	static std::string& Replace(std::string& str, std::string key, std::string rep);
	static void SplitLeft(std::string str, std::string& left, std::string& right,const char * tok=" \t\n\r");

	//SEFU 8
#ifdef USE_CHINESE
	static bool bChineseLanguage;
#endif
	static void SetChineseLanguage(bool chinese);

//	static int my_sprintf(char * dest, const char * format, ...);
//	static int my_vwsprintf(unsigned short * dest, const char * format, va_list &va);

	static void StrFindAndReplace(char * dest, const char * source, const char * str_find, const char * str_replace);
	static void StrFindAndReplace(unsigned short * dest, const unsigned short * source, const unsigned short * str_find, const unsigned short * str_replace);

	static const unsigned short* FindCharRight(const unsigned short* s, const unsigned short c, bool langJP = false);

	static int Interp( int valMin, int valMax, int stepMin, int step, int stepMax);
#ifdef USE_OGL
	static f32 Interp( f32 valMin, f32 valMax, f32 stepMin, f32 step, f32 stepMax);
#endif /* USE_OGL */

	template <class T>
	static const T& Max (const T& v1, const T& v2){
		return (v1>v2)?(v1):(v2);
	}
	template <class T>
	static const T& Min (const T& v1, const T& v2){
		return (v1<v2)?(v1):(v2);
	}

	static std::string GetCompleteFileName(const std::string& in_strFileName, const char* in_pExtension);

}; //class Util

#endif
