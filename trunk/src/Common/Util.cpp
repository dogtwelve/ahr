
#include "config.h"
#include "util.h"
#include "str_utils.h"

std::string& Util::ToUpper(std::string& str)
{
	char* src = (char*) str.c_str();
	while(*src!=0)
	{
		*src = toupper(*src);
		src++;
	}

	return str;
}

std::string& Util::Trim(std::string& str){

	long bpos;
	long epos;
	bpos=str.find_first_not_of(" \t\n\r");
	if(bpos==str.npos){
		str="";
		return str;
	}
	epos=str.find_last_not_of(" \t\n\r");
	if(epos==str.npos){
		str="";
		return str;
	}

	return str=str.substr(bpos,epos-bpos+1);
}

std::string& Util::Replace(std::string& str, std::string key, std::string rep){
	long bpos;
	
	bpos=str.find(key);
	while(bpos!=str.npos){
		str=str.replace(bpos,key.size(),rep);
		bpos=str.find(key,bpos+rep.size());
	}

	return str;
}


void Util::SplitLeft(std::string str, std::string& left, std::string& right,const char * tok){

	long pos;
	str=Trim(str);
	pos=str.find_first_of(tok);

	if(pos!=str.npos){
#if defined(__BREW__)
		std::string strBlockName = str.substr(0,pos);

		left=Trim(strBlockName);
		strBlockName = str.substr(pos+1,str.size()-pos);
		right=Trim(strBlockName);

#else
		left = str.substr(0,pos);
		Trim(left);
		right = str.substr(pos+1,str.size()-pos);
		Trim(right);
#endif //__BREW__

	}else{
		left=str;
		right="";
	}
}


std::string Util::GetCompleteFileName(const std::string& in_strFileName, const char* in_pExtension)
{
	std::string strFileName = in_strFileName;
	for(std::string::iterator it = strFileName.begin(); it != strFileName.end(); ++it)


	{
		*it = tolower(*it);
	}

	if (strFileName.find(in_pExtension) == std::string::npos)

	{
		strFileName.append(in_pExtension);
	}
	return strFileName;
}

//SEFU 8
#ifdef USE_CHINESE
bool Util::bChineseLanguage = false;
void Util::SetChineseLanguage(bool chinese)
{
	bChineseLanguage = chinese;
}
#endif

//int Util::my_sprintf(char * dest, const char * format, ...)
//{
//	va_list va;
//	va_start(va, format);
//
//	int result;
////SEFU 8
//#ifdef USE_CHINESE
//	if (bChineseLanguage)
//	{
//		result = my_vwsprintf((unsigned short*)dest, format, va);
//	}
//	else
//#endif
//	{
//		result = vsprintf(dest, format, va);
//	}
//
//	va_end(va);
//	return result;
//}
//
//int Util::my_vwsprintf(unsigned short * dest, const char * format, va_list &va)
//{
//	int len = 0;
//
//	unsigned short tmp[10];
//	int tmp_len = 0;
//
//	while( *format != '\0' ) 
//	{
//		if (*format == '%')
//		{
//			format++;
//
//			switch (*format)
//			{
//			case '%':
//				{
//					dest[len++] = TO_UNICODE('%');
//				}
//				break;
//			case 'c':
//				{
//					char c = va_arg( va, char );
//					dest[len++] = TO_UNICODE(c);
//				}
//				break;
//			case 's':
//				{
//					unsigned short * s = va_arg( va, unsigned short * );
//					while ((*s) & 0xFF00 && (*s) & 0x00FF)
//					{
//						dest[len++] = *s;
//						s++;
//					}
//				}
//				break;
//			case 'z':
//				{
//					unsigned char * s = va_arg( va, unsigned char * );
//					while (*s)
//					{
//						dest[len++] = TO_UNICODE(*s);
//						s++;
//					}
//				}
//				break;
//			case 'd':
//				{
//					int d = va_arg( va, int );
//					if (d < 0)
//					{
//						dest[len++] = TO_UNICODE('-');
//						d = -d;
//					}
//					if (d == 0)
//					{
//						dest[len++] = TO_UNICODE('0');
//						break;
//					}
//					tmp_len = 0;
//					while (d)
//					{
//						tmp[tmp_len++] = TO_UNICODE('0' + d % 10);
//						d /= 10;
//					}
//					while (tmp_len--)
//					{
//						dest[len++] = tmp[tmp_len];
//					}
//				}
//				break;
//			default:
//				{
//					A_ASSERT(!"FORMAT NOT SUPPORTED");
//				}
//				break;
//			} 
//		}
//		else
//		{
//			dest[len++] = TO_UNICODE(*format);
//		}
//		format++;
//	}
//
//	dest[len] = 0;
//
//	return len;
//}

void Util::StrFindAndReplace(char * dest, const char * source, const char * str_find, const char * str_replace)
{
	const char * ptr_source;
	const char * ptr_find;

	const int str_find_len = strlen(str_find);
	const int str_replace_len = strlen(str_replace);

	while (*source)
	{
		ptr_source = source;
		ptr_find = str_find;

		while (*ptr_source == *ptr_find && *ptr_source)
		{
			ptr_source++;
			ptr_find++;
		}

		if (ptr_source - source == str_find_len)
		{
			memcpy(dest, str_replace, str_replace_len);
			dest += str_replace_len;
			source = ptr_source;
		}
		else
		{
			*dest++ = *source;
			source++;
		}
	}

	*dest = 0;
}

void Util::StrFindAndReplace(unsigned short * dest, const unsigned short * source, const unsigned short * str_find, const unsigned short * str_replace)
{
	const unsigned short * ptr_source;
	const unsigned short * ptr_find;
	const unsigned short * tmp;

	int str_find_len = 0; 
	int str_replace_len = 0;

	tmp = str_find;
	while (*tmp++)
		str_find_len++;

	tmp = str_replace;
	while (*tmp++)
		str_replace_len++;

	while (*source)
	{
		ptr_source = source;
		ptr_find = str_find;

		while (*ptr_source == *ptr_find && *ptr_source)
		{
			ptr_source++;
			ptr_find++;
		}

		if (ptr_source - source == str_find_len)
		{
			memcpy(dest, str_replace, 2 * str_replace_len);
			dest += str_replace_len;
			source = ptr_source;
		}
		else
		{
			*dest++ = *source;
			source++;
		}
	}

	*dest = 0;
}

	const unsigned short* Util::FindCharRight(const unsigned short* s, const unsigned short c, bool langJP)
	{
#ifdef USE_CHINESE
		if (bChineseLanguage)
		{
			#pragma REMINDER ("Implement FindCharRight for Unicode strings")
			unsigned short* tmp = (unsigned short*)s;
			unsigned short* tmp2 = (unsigned short*)s;
			int len = 0;;
			while(*(tmp++))
				len++;
			while(len--)
			{
				unsigned short a = TO_UNICODE(c);
				if(tmp2[len] ==65281 )//rc to do 
					return (char*)(tmp2+len);
			}
			return NULL;
		}
		
#endif
		if(langJP)
		{
			
			int n = strlen(s);
			while (n--)
				if (s[n] == 65281)// quick fix for "!" 
					return s + n;
			return NULL;
		}
		else
		{
			int n = strlen(s);
			while (n--)
				if (s[n] == c)
					return s + n;
			return NULL;
		}
	}

	int Util::Interp( int valMin, int valMax, int stepMin, int step, int stepMax)
	{
		return valMin + ( valMax - valMin ) * ( step - stepMin ) / ( stepMax - stepMin );
	}

#ifdef USE_OGL

	f32 Util::Interp( f32 valMin, f32 valMax, f32 stepMin, f32 step, f32 stepMax)
	{
		return valMin + ( valMax - valMin ) * ( step - stepMin ) / ( stepMax - stepMin );
	}	

#endif /* USE_OGL */

