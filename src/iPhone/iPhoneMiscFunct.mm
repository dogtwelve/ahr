/*#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSUserDefaults.h>

#include <sys/time.h>
#include <malloc/malloc.h>
#include <stdio.h>

#include "../game/GX_Device.h"
*/

#undef WIN32

#import "Accelerometer.h"
#import <UIKit/UIKit.h>

//#import "GLoftApp.h"

class GLoftApp;


extern GLoftApp* g_sharedInstance;

class UIApplication;
extern UIApplication* g_sharedInstanceUIApplication;


void GetDeviceLanguage(char *lang)
{
    //To know the current language    
     NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
     NSArray* languages = [defs objectForKey:@"AppleLanguages"];
     NSString* preferredLang = [languages objectAtIndex:0];
     sprintf(lang, "%s", [preferredLang UTF8String]);

}

extern "C" void showStatusBar(bool bShow)
{
	[g_sharedInstanceUIApplication setStatusBarHidden:!bShow animated:YES];
}

extern "C" void setStatusBarOrientation(int orient)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	UIDeviceOrientation orientation;
		
	switch (orient)
	{
		case ORIENTATION_PORTRAIT:
		default:
			orientation = UIDeviceOrientationPortrait;
			break;
		case ORIENTATION_LANDSCAPE_90:
			orientation = UIDeviceOrientationLandscapeRight;
			break;
		case ORIENTATION_LANDSCAPE_270:
			orientation = UIDeviceOrientationLandscapeLeft;
			break;
	}
	
	g_sharedInstanceUIApplication.statusBarOrientation = (UIInterfaceOrientation) orientation;
	
	[pool release];
}