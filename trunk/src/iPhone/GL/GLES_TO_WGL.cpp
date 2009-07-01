
//wrapper for glOES functions (specific to iphone)
#ifndef IPHONE
#ifdef USE_OGL

#include <windows.h>
#include "GL/gl.h"
#include "GL/glext.h"
#include "GL/GLES_TO_WGL.h"	


//GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersOES = 0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferOES = 0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersOES = 0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferOES = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageOES = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferOES = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DOES = 0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivOES = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusOES = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersOES = 0;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersOES = 0;

PFNGLACTIVETEXTUREARBPROC glActiveTexture = 0;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture = 0;

PFNGLBLENDCOLORPROC glBlendColor = 0;


//this will link OES (iphone) to EXT (wgl)
void LinkOES2EXT()
{
	const GLubyte* ext = glGetString(GL_EXTENSIONS);

	//get corresponding extensions functions specific to WGL	
	//GL_EXT_framebuffer_object
	glGenFramebuffersOES = (PFNGLGENFRAMEBUFFERSEXTPROC) wglGetProcAddress("glGenFramebuffersEXT");
	glBindFramebufferOES = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glGenRenderbuffersOES = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glBindRenderbufferOES = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glRenderbufferStorageOES = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glFramebufferRenderbufferOES = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	glFramebufferTexture2DOES = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glGetFramebufferAttachmentParameterivOES = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
	glCheckFramebufferStatusOES = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
	glDeleteFramebuffersOES = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
	glDeleteRenderbuffersOES = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");

	glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");

	glBlendColor = (PFNGLBLENDCOLORPROC)wglGetProcAddress("glBlendColor");
}

#endif //USE_OGL
#endif //!IPHONE