
// unicode utils
int strlen(const unsigned short* str);

int strcpy(unsigned short* dst, const unsigned short* src);

int strcpy(unsigned short* dst, const char* src);

int strcpy(char* dst, const unsigned short* src);

int strcat(unsigned short* dst, const unsigned short* src);

int strcat(unsigned short* dst, const char* src);

int strcmp(unsigned short* str1, unsigned short* str2);

int itoa(int no, unsigned short* strbuff);

unsigned short* toUnicode(char* str);
void CharToUnicode(unsigned short* dst, const char* str);

void sprintf(unsigned short *buff, const unsigned short *text, ...);
void sprintf(unsigned short *buff, const char *text, ...);
