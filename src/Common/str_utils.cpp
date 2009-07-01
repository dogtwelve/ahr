
#include <stdio.h>
#include <stdarg.h>
//#include <varargs.h>
#include <string.h>
#include "str_utils.h"
#include "DevUtil.h"


// unicode utils
int strlen(const unsigned short* str)
{
	unsigned int len = 0;
	
	while (str[len] != 0)
		len++;
	
	return len;
}

int strcpy(unsigned short* dst, const unsigned short* src)
{
	unsigned int i = 0;
	
	do
	{
		dst[i] = src[i];
		i++;
	}
	while (src[i] != 0);

	dst[i] = 0;
	
	return i;
}

int strcpy(unsigned short* dst, const char* src)
{
	unsigned int i = 0;
	
	do
	{
		dst[i] = src[i];
		i++;
	}
	while (src[i] != 0);

	dst[i] = 0;
	
	return i;
}

int strcpy(char* dst, const unsigned short* src)
{
	unsigned int i = 0;
	
	do
	{
		dst[i] = src[i];
		i++;
	}
	while (src[i] != 0);

	dst[i] = 0;
	
	return i;
}

int strcat(unsigned short* dst, const unsigned short* src)
{
	return strcpy(dst + strlen(dst), src);
}

int strcat(unsigned short* dst, const char* src)
{
	return strcpy(dst + strlen(dst), src);
}

int strcmp(unsigned short* str1, unsigned short* str2)
{
	unsigned int i = 0;
	
	while (str1[i] == str2[i] && str1[i] != 0)
		i++;
	
	return str1[i] - str2[i];
}

int itoa(int no, unsigned short* strbuff)
{
	int l = 0;
	char tmp[10];
	bool neg = false;
	
	if (no < 0)
	{
		no = -no;
		neg = true;
	}
	
	while (no > 0)
	{
		A_ASSERT(l < 10);
		
		tmp[l++] = no % 10;
		no /= 10;
	}
	
	if (l == 0)
	{
		strbuff[0] = '0';
		strbuff[1] = 0;
		
		return 1;
	}
	else
	{
		if (neg)
			*(strbuff++) = '-';
		
		int i = l;
		while (i > 0)
		{
			i--;
			
			*(strbuff++) = '0' + tmp[i];
		}

		*strbuff = 0;
		
		return l + (neg ? 1 : 0);
	}
}

void sprintf(unsigned short *buff, const unsigned short *text, ...)
{
	int i = 0, j = 0;
	va_list	argptr;
    va_start (argptr, text);
    
    while (text[i] != 0)
    {
    	if (text[i] == '%')
    	{
    		i++;
    		switch(text[i])
    		{
    			case '%':
    			{
					buff[j++] = '%';
					// i++; // rax - bugfix
					break;
				}
				
				case 'd':
				{
					j += itoa(va_arg(argptr, int), buff + j);
					break;
				}
				
				case 's':
				{
					unsigned short* str = va_arg(argptr, unsigned short*);
					j += strcpy(buff + j, str);
					break;
				}
    		}
			i++;
    	}
    	else
    	{
    		buff[j++] = text[i++];
    	}
    }
    
    buff[j] = 0; // terminator
	
    va_end (argptr);
}

void sprintf(unsigned short *buff, const char *text, ...)
{
	int i = 0, j = 0;
	va_list	argptr;
    va_start (argptr, text);
    
    while (text[i] != 0)
    {
    	if (text[i] == '%')
    	{
    		i++;
    		switch(text[i])
    		{
    			case '%':
    			{
					buff[j++] = '%';
					break;
    			}
				
				case 'd':
				{
					int dec = va_arg(argptr, int);
					j += itoa(dec, buff + j);
					break;
				}
				
				case 's':
				{
					unsigned short* str = va_arg(argptr, unsigned short*);
					j += strcpy(buff + j, str);
					break;
				}
				
				default:
					A_ASSERT(false);
    		}
			i++;
    	}
    	else
    	{
    		buff[j++] = text[i++];
    	}
    }
    
    buff[j] = 0; // terminator
	
    va_end (argptr);
}

unsigned short* toUnicode(char* str)
{
	unsigned short* uniStr = (unsigned short*)str;
	unsigned int len = strlen(str);

	// no memory alloc here
	for (int i = len-1; i >= 0; i--)
		uniStr[i] = str[i];
	
	uniStr[len] = 0; // terminator
	
	return uniStr;
}

void CharToUnicode(unsigned short* dst, const char* str)
{	
	int k = 0;
	for(k = 0; k<strlen(str);k++)
	{
		dst[k] = str[k];
	}
	dst[strlen(str)] = 0;
}