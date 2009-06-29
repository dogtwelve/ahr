#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

// strip white spaces (left, right); returns NULL if it's a blanc line or a comment (begins with "//")
char *trimLine(char *line)
{
	char *p;
	char *q;

	if (line == NULL || line[0] == 0 || line[0] == '\n')
		return NULL;

	p = line + strlen(line) - 1;

	while (*p == ' ' || *p == '\t' || *p == '\n')
	{
		*p = 0;
		p--;
	}

	p = line;

	while (*p == ' ' || *p == '\t')
		p++;

	// check if comment line
	if (strlen(p) > 1 && p[0] == '/' && p[1] == '/')
		return NULL;

	//check if the line contains a comment; if so, right trim line	
	if((q = strstr(p, "//")) != NULL)
		*q = 0;

	return p;
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

// Parse a string and retrieves the integers
char *parserStr;
char *parserDelim;
//char *parserPtr;

void parserBegin(char *s, char *delims)
{
	parserStr = s;
	parserDelim = delims;
}


// return false at end of string
int parserGetInt(int *value)
{
	int bIntFound = 0;
	int n;

	while (!bIntFound && *parserStr != 0)
	{
		/*if((*parserStr >= '0' && *parserStr <= '9') || *parserStr == '-' || *parserStr == '+')
		{
			long l = strtol(parserStr, NULL, 0);
			value = l;
			
			// skip till the end of the number
			int n = strspn(parserStr, "0123456789abcdefABCDEF+-xX");
			parserStr += n;
			
			// prepare the next token, skip the dellimitators
			while(*parserStr != 0 && strchr(parserDelim, *parserStr))
				parserStr++;

			bIntFound = true;
		}
		else
		{
			parserStr++;
		}
		*/

		while (*parserStr && 
			(*parserStr < '0' || *parserStr > '9') &&
			*parserStr != '-' && *parserStr != '+')
			parserStr++;

		if (strToInt(parserStr, value))
			bIntFound = 1;

		// skip till the end of the number
		n = strspn(parserStr, "0123456789abcdefABCDEF+-xX");
		parserStr += n;
	}

	return bIntFound;
}

int strToInt(char *s, int *value)
{
	int bNumber;
	int sgn;
	int base;
	int bFigure;

	if (!s)
		return NULL;

	bNumber = 0;
	*value = 0;

	sgn = 1;
	if (*s == '-')
	{
		sgn = -1;
		s++;
	}
	else if (*s == '+')
	{
		s++;
	}

	base = 10;
	if (*s == '0' && (*(s+1) == 'x' || *(s+1) == 'X'))
	{
		s += 2;
		base = 16;
	}

	bFigure = 1;
 
	do 
	{
		int c = toupper(*s);
		bFigure = 0;

		if (c >= '0' && c <= '9' || c >= 'A' && c <= 'F')
		{
			bFigure = 1;
			bNumber = 1;
			*value *= base;
			if (c >= '0' && c <= '9')
				*value += c - '0';
			else *value += 10 + c - 'A';

			s++;
		}

	} while (bFigure && *s != 0);

	return bNumber;
}

// return false at end of string
int parserGetString(char *string)
{
	int res = 0;

	char *p = string;

	//trim left
	while(*parserStr != 0 && strchr(parserDelim, *parserStr))
			parserStr++;

	while(*parserStr != 0 && !strchr(parserDelim, *parserStr))
	{
		*p = *parserStr;
		parserStr++;
		p++;

		res = 1;
	}

	*p = 0;

	return res;
}


int parserSkip(char *string)
{
	char *p;
	if ((p = strstr(parserStr, string)) != NULL)
	{
		parserStr = p + strlen(string);

		//trim
		while(*parserStr != 0 && strchr(parserDelim, *parserStr))
			parserStr++;

		return 1;
	}
	return 0;
}