#ifndef _IphoneUtil_H_
#define _IphoneUtil_H_

#ifdef IPHONE

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>

extern "C" unsigned long OS_GetTime (void);

extern "C" void LOGDEBUG(const char *error, ...);

// Accelerometer
extern "C" int GetPhoneOrientation();

extern "C" int GetPhoneRotLeftRight(int phoneOrientation);
extern "C" int GetPhoneRotSpeedLeftRight(int phoneOrientation);

extern "C" int GetPhoneRotFwdBack(int phoneOrientation);
extern "C" int GetPhoneRotSpeedFwdBack(int phoneOrientation);

// Movie Player
extern "C" void LoadMovie(const char* movieFileName);
extern "C" void PlayMovie();
extern "C" void StopMovie();
extern "C" bool IsMoviePlaying();
extern "C" bool IsMoviePaused();

// Text Input
extern "C" void TextInputStart(char* textBuf, unsigned int bufSize, int x, int y, int w, int h);
extern "C" void TextInputClose();
extern "C" bool TextInputIsDone();

#endif /* IPHONE */

#endif /* _IphoneUtil_H_ */