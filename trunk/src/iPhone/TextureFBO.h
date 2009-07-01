
#ifndef _TEXTUREFBO_H_
#define _TEXTUREFBO_H_

#ifdef IPHONE
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>	
#else
	#include <windows.h>
	//#include "GLES/egl.h"
	#include "GL/gl.h"
	#include "GL/glext.h"
	#include "GL/GLES_TO_WGL.h"	
#endif


class TextureFBO
{
public:
	TextureFBO(int width, int height, bool alpha = true);
	~TextureFBO();

//used to save/restore info about current linkage
void saveFBOLinkage();
void restoreFBOLinkage();

void bindFBO();

//used to save info about current linkage
GLuint m_SavedRenderBuffer;
GLuint m_SavedFrameBuffer;

GLuint m_DepthRenderBuffer;
GLuint m_FrameBuffer;
GLuint m_glTextureName;
};

#endif //_TEXTUREFBO_H_