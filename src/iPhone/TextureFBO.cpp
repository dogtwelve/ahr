#include "TextureFBO.h"
#include "DevUtil.h"


TextureFBO::TextureFBO(int width, int height, bool alpha)
{
	GLuint oldRenderBuffer;
	GLuint oldFrameBuffer;
	
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, (GLint *) &oldRenderBuffer);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, (GLint *) &oldFrameBuffer);
	
	//create the framebuffer object
	glGenFramebuffersOES(1, &m_FrameBuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer);	
	
	//attach depth render buffer
	glGenRenderbuffersOES(1, &m_DepthRenderBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_DepthRenderBuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, width, height);	
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_DepthRenderBuffer);
	
	//attach texture to framebuffer
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_glTextureName);
	glBindTexture(GL_TEXTURE_2D, m_glTextureName);
	glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_glTextureName, 0);
	
	
	GLuint tempValue = 0;
	glGetFramebufferAttachmentParameterivOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES,(GLint*)&tempValue);
		
	glGetFramebufferAttachmentParameterivOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES,(GLint*)&tempValue);
	
	//check for errors
	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	A_ASSERT(status==GL_FRAMEBUFFER_COMPLETE_OES);
	
	//restore old linkage
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFrameBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRenderBuffer);
}

TextureFBO::~TextureFBO()
{	
	glDeleteTextures( 1, &m_glTextureName);
	m_glTextureName = 0;

	glDeleteRenderbuffersOES(1, &m_DepthRenderBuffer);
	m_DepthRenderBuffer = 0;

	glDeleteFramebuffersOES(1, &m_FrameBuffer);
	m_FrameBuffer = 0;	
}


void TextureFBO::saveFBOLinkage()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, (GLint *)&m_SavedFrameBuffer);
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES,(GLint *)&m_SavedRenderBuffer);
}

void TextureFBO::restoreFBOLinkage()
{
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_SavedFrameBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_SavedRenderBuffer);
}

void TextureFBO::bindFBO()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, 0);

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_FrameBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_DepthRenderBuffer);
}