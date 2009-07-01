#ifndef _GLoftApp_H_
#define _GLoftApp_H_

#include "Config.h"

#ifdef USE_IPHONE_KEYBOARD
	#import "TextInput.h"
#endif 
#undef WIN32
//iphone library
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreFoundation/CFBundle.h>

//client library
#import "UIGLView.h"
#import "IphoneUtil.h"

#include <stdio.h>
#include <string.h>

//unix library
#include <pthread.h>
#include <sched.h>
#include <unistd.h>



#ifdef USE_SYNCHRONIZE_TOUCH

extern "C" void lockTouchSyncObj();
extern "C" void unlockTouchSyncObj();
extern "C" void destroyTouchSyncObj();

#endif

@class UIView;
@class UIGLView;


@interface GLoftApp : NSObject
{
	UIWindow* m_window;
    UIGLView* m_GLView;
}

@property (readonly) UIGLView* view;
@property (readonly) UIWindow* window;

+(GLoftApp*)sharedInstance; 
#ifdef USE_IPHONE_KEYBOARD
	-(void)textInputInit:(TextInputInitParams*)params;
#endif 

@end

#endif /* _GLoftApp_H_ */
