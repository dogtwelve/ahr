#include "uncompress.h"
#include "devutil.h"

//#include "stdinc.h"
//#include "def/def.h"

//#if USE_LZMA_UNCOMPRESSION == 1

extern "C"
{ 
	#include "LzmaDecode.h"
}

//#include "Game.h"
//#include "game/common/LZMAIndex.h"

int uncompress(char* dest, unsigned long* destLen, const char* source, unsigned long sourceLen, unsigned char* temp_buff)
{
	unsigned char properties[5];
	unsigned char prop0;

	for(int i = 0; i < sizeof(properties) / sizeof(unsigned char); ++i)
	{
		properties[i] = *source++;
	}

	unsigned int outSize = 0;

	for(int ii = 0; ii < 4; ii++)
	{
		unsigned char b = *source++;
		outSize += (unsigned int)(b) << (ii * 8);
	}

	A_ASSERT(outSize != 0xFFFFFFFF);

	for(int ii = 0; ii < 4; ii++)
	{
		unsigned char b = *source++;

		if (b != 0)
		{
			return 1;
		}
	}

	void const* inStream = source;

	prop0 = properties[0];
	
	if (prop0 >= (9*5*5))
	{
		return 1;
	}
	
	int lc, lp, pb;

	for (pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
	for (lp = 0; prop0 >= 9; lp++, prop0 -= 9);
	
	lc = prop0;

	unsigned int compressedSize = sourceLen - 13;

	unsigned int outSizeProcessed;
	unsigned int lzmaInternalSize = (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp)))* sizeof(CProb);
	
	A_ASSERT(lzmaInternalSize <= 16 * 1024);
	// --- TODO: use a global buffer
//	unsigned char* lzmaInternalData = gll_new unsigned char[lzmaInternalSize];// = MALLOC(lzmaInternalSize); //TODO: use a temp buffer so that no new are done every time we uncompress a file
	unsigned char* lzmaInternalData = temp_buff;

	int res = LzmaDecode(
		(unsigned char *)lzmaInternalData, lzmaInternalSize,
		lc, lp, pb,
		(unsigned char*)inStream, compressedSize,
		(unsigned char*)dest, *destLen, 
		&outSizeProcessed);

	*destLen = outSizeProcessed;

//	gll_delete(lzmaInternalData);

	return res;
}

//#endif // defined USE_LZMA_UNCOMPRESSION
