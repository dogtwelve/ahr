#ifndef MY_PVRTC_H__

#define IPHONE_BUG_SQUARE_PVR_TEXTURES

bool CanConvertFileToPVRTC(char *namein);
int ConvertFileToPVRTC(char *namein, char *nameout);

extern char g_absPathToPVRToolEXE[1024];

extern void fillTgaHeader(TGA_HEADER *tgah, unsigned char *data);
extern unsigned short	RGB24TO16( RGB24* c );

#endif // MY_PVRTC_H__