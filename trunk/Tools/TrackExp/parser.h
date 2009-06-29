
#ifndef MY_TINY_PARSER
#define MY_TINY_PARSER

#include <stdio.h>

char *trimLine(char *line);

void parserBegin(char *s, char *delims);

int strToInt(char *s, int *value);

// return false at end of string
int parserGetInt(int *value);

// return false at end of string
int parserGetString(char *string);

int parserSkip(char *string);

#endif