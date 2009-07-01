#ifndef LZMA_UNCOMPRESS__H__INCLUDED__
#define LZMA_UNCOMPRESS__H__INCLUDED__

int uncompress(char* dest, unsigned long* destLen, const char* source, unsigned long sourceLen, unsigned char* temp_buff);

#endif	// ! LZMA_UNCOMPRESS__H__INCLUDED__
