

#undef WIN32
#import <UIKit/UIKit.h>

//void willExit()
//{
//}

int main(int argc, char *argv[]) 
{
//	int ret = atexit(willExit);
	
	NSAutoreleasePool*		pool = [NSAutoreleasePool new];
	UIApplicationMain(argc, argv, nil, @"GLoftApp");	
	[pool release];	
	
	return 0;
	
}