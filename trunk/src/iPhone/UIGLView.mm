
#undef WIN32

#import <UIKit/UIKit.h>
#import "UIGLView.h"


extern "C" void UpdateTouchPress(unsigned char touchId, int x, int y );
extern "C" void UpdateTouchRelease(unsigned char touchId, int x, int y);
extern "C" void UpdateTouchMove(unsigned char touchId, int x, int y);

void LOGDEBUG(const char *error, ...);


//EAGLNativeWindow			g_EAGLNativeWindow;
//EAGLPixelFormat				g_EAGLFormat;
//EAGLContext					g_EAGLContext;
//EAGLSurface					g_EAGLSurface;

CAEAGLLayer*	g_EAGLLayer;
EAGLContext* g_EAGLContext;
GLuint g_frameBuffer;
GLuint g_renderBuffer;
GLuint g_depthBuffer;
//GLuint g_stencilBuffer;

extern "C" void InitOpenGLESWindow()
{
	g_EAGLLayer.opaque = YES;//NEW
	
	g_EAGLLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil]; //NEW
	
	//get full screen width, height
	CAEAGLLayer* eaglLayer = (CAEAGLLayer*) g_EAGLLayer;
	CGSize newSize;
	newSize = [eaglLayer bounds].size;
	newSize.width = roundf(newSize.width);
	newSize.height = roundf(newSize.height);
		
	// create opengl context
	g_EAGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	
	[EAGLContext setCurrentContext:g_EAGLContext];
	
		
	GLuint oldRenderBuffer;
	GLuint oldFrameBuffer;
	
	glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, (GLint *) &oldRenderBuffer);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, (GLint *) &oldFrameBuffer);
		
	
	//create FRAMEBUFFER
	glGenFramebuffersOES(1, &g_frameBuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, g_frameBuffer);

	// create render buffer
	glGenRenderbuffersOES(1, &g_renderBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_renderBuffer);
	[g_EAGLContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:g_EAGLLayer];
	//attach render buffer to framebuffer
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, g_renderBuffer);

	//create DEPTHBUFFER
	glGenRenderbuffersOES(1, &g_depthBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_depthBuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, newSize.width, newSize.height);
	//attach depth buffer to framebuffer
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, g_depthBuffer);		

	//create STENCILBUFFER
	//glGenRenderbuffersOES(1, &g_stencilBuffer);
	//glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_stencilBuffer);
	//glRenderbufferStorageOES(GL_RENDERBUFFER_OES, 6401, newSize.width, newSize.height);
	//attach stencil buffer to framebuffer
	//glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, g_stencilBuffer);
		
	glViewport(0, 0, newSize.width, newSize.height);
	glScissor(0, 0, newSize.width, newSize.height);
	
	int tempValue = 0;
	
	//get render bits
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_renderBuffer);		
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_RED_SIZE_OES, &tempValue );
	LOGDEBUG( "red size [%d]", tempValue);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_GREEN_SIZE_OES, &tempValue );
	LOGDEBUG( "green size [%d]", tempValue);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_BLUE_SIZE_OES, &tempValue );
	LOGDEBUG( "blue size [%d]", tempValue);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_ALPHA_SIZE_OES, &tempValue );
	LOGDEBUG( "alpha size [%d]", tempValue);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_INTERNAL_FORMAT_OES, &tempValue );
	LOGDEBUG( "alpha internal format [%x]", tempValue);
	
	//get depth bits
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_depthBuffer);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_DEPTH_SIZE_OES, &tempValue );
	LOGDEBUG( "depth size [%d]", tempValue);
	
	//get stencil bits
	//glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_stencilBuffer);
	//glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_STENCIL_SIZE_OES, &tempValue );
	//LOGDEBUG( "stencil size [%d]", tempValue);	
		
	//bind to renderig buffer
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, g_renderBuffer);	
	
	//bind to framebuffer
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, g_frameBuffer);
	
	
	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_OES:
			LOGDEBUG("SUCCESSFULLY CREATED FBO");
			break;
		default:
			LOGDEBUG("FAILED TO CREATE FBO  with CODE [%d]\n", status);
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFrameBuffer);
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRenderBuffer);
			return;
	}

	//openGL specific
	::glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Black Background
	::glClearDepthf(1.0f);									// Depth Buffer Setup
	::glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	::glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
}

extern bool g_bSkipRendering;

extern "C" void UIViewBeginRender(){}

extern "C" void UIViewEndRender()
{
	if( !g_bSkipRendering)
	{
		//eaglSwapBuffers(g_EAGLSurface);
		[g_EAGLContext presentRenderbuffer:GL_RENDERBUFFER_OES];
	}
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, false);
	sched_yield();
}






//temporary in use
int g_isTouch;


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////UIGLView Implementation////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation UIGLView


- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];

	// convert base layer
	g_EAGLLayer = (CAEAGLLayer*)[self layer];
	return self;
}


//override layerClass method
+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (void) destroySurface
{
	EAGLContext *oldContext = [EAGLContext currentContext];

	//set current context to the game context
	if (oldContext != g_EAGLContext)
		[EAGLContext setCurrentContext:g_EAGLContext];

	//destroy associated buffers

	//destroy render buffer
	glDeleteRenderbuffersOES(1, &g_renderBuffer);
	g_renderBuffer = 0;
	
	//destroy depth buffer
	glDeleteRenderbuffersOES(1, &g_depthBuffer);
	g_depthBuffer = 0;
	
	//destroy stencil buffer
	//glDeleteRenderbuffersOES(1, &g_stencilBuffer);
	//g_stencilBuffer = 0;

	//destroy frame buffer
	glDeleteFramebuffersOES(1, &g_frameBuffer);
	g_frameBuffer = 0;

	//restore context
	if (oldContext != g_EAGLContext)
		[EAGLContext setCurrentContext:oldContext];
}

- (void) dealloc
{
	//if(g_EAGLContext)
	//eaglDestroyContext(g_EAGLContext);

	[self destroySurface];

	[g_EAGLContext release];
	g_EAGLContext = nil;

	[super dealloc];
}


- (int) getTouchIndex: (UITouch*)touch
	{
		int i;
		for(i = 0 ; i < m_touchCount ; i++)
		{
			if(touch == m_crtTouches[i])
				{
					return i;
				}
		}
		return -1;
	}

- (void) addTouch: (UITouch*) touch
	{
		if(m_touchCount < MAX_TOUCHES)
			{
				m_crtTouches[m_touchCount] = touch;
				m_touchCount++;
			}
	}

- (void) removeTouch: (UITouch*) touch
{
	int touchIndex = [self getTouchIndex:touch];
	int i;
	if(touchIndex != -1)
	{
		for(i = touchIndex ; i < MAX_TOUCHES - 1 ; i++)
		{
			m_crtTouches[i] = m_crtTouches[i + 1];
		}

		m_touchCount--;
	}
}

- (void)touchesChangedWithEvent:(UIEvent*)event
{

#ifdef USE_SYNCHRONIZE_TOUCH
	lockTouchSyncObj();
#endif

    NSSet *touches = [event allTouches];

    for (UITouch *myTouch in touches)
    {
    	//check to see whether this touch event already exists
		int touchIndex = [self getTouchIndex:myTouch];
    	if(touchIndex == -1 && myTouch.phase == UITouchPhaseBegan) //doesn't exist yet, add it
    	{
    		touchIndex = m_touchCount;
			[self addTouch:myTouch];
    	}

    	//get current touch coordinates
		CGPoint touchPoint = [myTouch locationInView:self];
		int fingerX = touchPoint.x;
		int fingerY = touchPoint.y;

		if(touchIndex != -1)
		{
			if (myTouch.phase == UITouchPhaseBegan)
			{
				UpdateTouchPress((unsigned char)touchIndex, fingerX, fingerY);
			}

			if (myTouch.phase == UITouchPhaseMoved)
			{
				UpdateTouchMove((unsigned char)touchIndex, fingerX, fingerY);
			}

			if (myTouch.phase == UITouchPhaseEnded || myTouch.phase == UITouchPhaseCancelled)
			{
				UpdateTouchRelease((unsigned char)touchIndex, fingerX, fingerY);
				[self removeTouch:myTouch];
			}
	   }
    }

#ifdef USE_SYNCHRONIZE_TOUCH	
	unlockTouchSyncObj();
#endif

}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{

	{
		[self touchesChangedWithEvent:event];
	}

}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	{
		[self touchesChangedWithEvent:event];
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	{
		[self touchesChangedWithEvent:event];
	}
}


@end


