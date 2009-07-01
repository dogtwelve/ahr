////////////////////////////////////////////////////////////////////////////////
// UIGLView.h
// Provides UIView for OpenGL rendering on iPhone
////////////////////////////////////////////////////////////////////////////////
#ifndef _UIGLView_H_
#define _UIGLView_H_

//iphone library
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>


//client library
#import "GLoftApp.h"

#define MAX_TOUCHES		10

////////////////////////////////////////////////////////////////////////////////
// UIGLView
@interface UIGLView : UIView
{
@private
	int						m_touchCount;
	UITouch*				m_crtTouches[MAX_TOUCHES];
}

- (id)initWithFrame:(CGRect)frame;
- (void) addTouch: (UITouch*) touch;
- (int) getTouchIndex: (UITouch*) touch;
- (void) removeTouch: (UITouch*) touch;

@end


#endif //_UIGLView_H_
