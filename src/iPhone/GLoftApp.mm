////////////////////////////////////////////////////////////////////////////////
// GLApp
#include "Config.h"
#include "HighGear.h"


#undef WIN32
#import "GLoftApp.h"

GLoftApp* g_sharedInstance = nil;
UIApplication* g_sharedInstanceUIApplication = nil;
bool m_bIsRunning = false;
volatile bool m_bIsAppPaused = false;
volatile bool m_bMovieMustBePaused = false;

#ifdef DEBUG_MEMORY_WARNING
bool m_bAppGotAnMemoryWarning = false;
#endif

//specific to UIView rendering technique
extern "C" void InitOpenGLESWindow();
extern "C" void UIViewBeginRender();
extern "C" void UIViewEndRender();

//specific to game engine
extern "C" void GameLoop();
extern "C" void GameInit();
extern "C" void GameEnd();
extern "C" void GamePause();
extern "C" void GameResume();

extern "C" void SuspendMovie();
extern "C" void ResumeMovie();

extern "C" unsigned int GetVersionNumber(char *versionString);


//extern EAGLNativeWindow				g_EAGLNativeWindow;
//extern EAGLPixelFormat				g_EAGLFormat;
//extern EAGLContext					g_EAGLContext;
//extern EAGLSurface					g_EAGLSurface;

extern CAEAGLLayer*	g_EAGLLayer;
extern EAGLContext* g_EAGLContext;
extern GLuint g_frameBuffer;
extern GLuint g_renderBuffer;

char g_AppPath[1024];

void ChangeCurrentDirToApplicationDir()
{
	CFBundleRef bundle;
	CFURLRef bundle_url;
	CFStringRef sr;
	char path[FILENAME_MAX], *end;

	if( !(bundle = CFBundleGetMainBundle()) )
	{
		return;
	}

	if( !(bundle_url = CFBundleCopyBundleURL( bundle )) )
	{
		return;
	}

	if( !(sr = CFURLCopyFileSystemPath( bundle_url, kCFURLPOSIXPathStyle )) )
	{
		CFRelease( bundle_url );
		return;
	}

	if( !CFStringGetCString( sr, path, FILENAME_MAX, kCFStringEncodingASCII ) )
	{
		CFRelease( bundle_url );
		CFRelease( sr );
		return;
	}

	//main bundle should not be released
	CFRelease( bundle_url );
	CFRelease( sr );

	if( end = strrchr( path, '/' ) )
		{
			*end = '\0';
		}

	//save current application absolute path
	strcpy(g_AppPath, path);
	strcat(g_AppPath, "/Engine.app/");
	
	LOGDEBUG("path:[%s]\n", g_AppPath );
	
	chdir( path );
}


void GetSaveFilePath(char* filePath, const char* filename)
{	
	sprintf(filePath, "/%s", filename);
	
	NSString *path = [[NSString alloc] initWithUTF8String:filePath];
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	path = [documentsDirectory stringByAppendingPathComponent:path];
	
	sprintf(filePath, "%s", [path UTF8String]);
	[path release];
}


pthread_t tId;

void* ThreadRoutine(void* param)
{
	ChangeCurrentDirToApplicationDir();
	
	//Initialize
	LOGDEBUG("INTIALIZE EGL/EAGL");
	
	InitOpenGLESWindow();

	LOGDEBUG("INITIALIZE...APPLICATION");
	
	GameInit();
	
	LOGDEBUG("SUCCES...APPLICATION...");
	
	while( m_bIsRunning )
	{
		
		UIViewBeginRender();  
		
		if(m_bIsAppPaused)
		{
			if(IsMoviePlaying())
			{
			   if(m_bMovieMustBePaused)
				{
					//printf("Warrning ... suspend\n");
					SuspendMovie();
				}
			}
			else
			{
				m_bMovieMustBePaused = false;
			}
		}
		
		GameLoop();
		
		UIViewEndRender();		
		
	}	
}

extern "C" void StartThread()
{
	if (pthread_create(&tId, NULL, &ThreadRoutine, NULL)) 
	{
		
		LOGDEBUG("ERROR StartThread \n");
		
		exit(1);
		
	}
	
	LOGDEBUG("Thread Created\n");

}

void JoinThread()
{
	
	if (pthread_join(tId, NULL))
	{
		
		LOGDEBUG("ERROR JoinThread\n");	
		
	}
	
	LOGDEBUG("Thread Destroyed\n");
	
}

////////////////////////////////////////////////////////////////////////////////////
// GLApp


@implementation GLoftApp

@synthesize view = m_GLView;
@synthesize window = m_window;

- (void) applicationDidFinishLaunching: (UIApplication*)application
{
	//initialize game
	m_bIsRunning = false;
	m_bIsAppPaused = false;

	//NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    	g_sharedInstance = self;
	g_sharedInstanceUIApplication = application;
	g_sharedInstanceUIApplication.idleTimerDisabled = YES;
	
	//init display
	
	CGRect screenRect = [[UIScreen mainScreen] bounds];
	
	m_window = [[UIWindow alloc] initWithFrame: screenRect];
	
	//Create the OpenGL ES view and add it to the window
	m_GLView = [[UIGLView alloc] initWithFrame: screenRect];
	
	m_GLView.multipleTouchEnabled = YES;
	
	[m_window addSubview: m_GLView];
	
	//Show the window
	[m_window makeKeyAndVisible];
	
//	[g_sharedInstanceUIApplication setStatusBarStyle: UIStatusBarStyleBlackTranslucent];
		
	//CAEAGLLayer* eaglLayer = (CAEAGLLayer*)[m_GLView layer];
	
	//eaglLayer.opaque = TRUE;
	
	//g_EAGLNativeWindow = [eaglLayer nativeWindow];	
	
	
    // Initialize Timer (30FPS)
    /*[NSTimer
        scheduledTimerWithTimeInterval:0.033
        target: self
        selector: @selector(update)
        userInfo: nil
        repeats: YES
    ];*/    
		
	//[pool release];
	
	
	char temp[256];
	strcpy(temp, [ [UIDevice currentDevice].model cString]);
	//printf("%s\n", temp);
	temp[6] = '\0';
	if(!strcmp(temp,"iPhone"))
	{
		g_DeviceType = DEVICE_TYPE_IPHONE;
	}
	
	strcpy(temp, [ [UIDevice currentDevice].systemVersion cString]);
	g_SystemVersion = GetVersionNumber(temp);

#ifdef USE_IPHONE_INTRO_VIDEO
	LoadMovie("asphalt4.mp4");
#endif // USE_IPHONE_INTRO_VIDEO
	
	//launch game
	StartThread();
}


//- (void)applicationSuspend:(struct __GSEvent *)event
//{

//	int a = 0;
//   m_bIsRunning = false;
	
//	GameEnd();
	
//    [ self terminate ];
	
//}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	m_bIsRunning = false;
//	CHighGear::GetInstance()->SaveProfile();

#ifdef USE_SYNCHRONIZE_TOUCH
	destroyTouchSyncObj();
#endif
		
	//if (IsMoviePlaying())
	//	StopMovie();
}

//TO DO - for interrupts
- (void)applicationDidBecomeActive:(UIApplication *)application
{
	//printf("applicationDidBecomeActive\n");
	ResumeMovie();
	GameResume();	
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	//printf("applicationWillResignActive\n");
	m_bMovieMustBePaused = true;
	SuspendMovie();
	GamePause();	
}

#ifdef DEBUG_MEMORY_WARNING
- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	m_bAppGotAnMemoryWarning = true;
}
#endif

- (void) dealloc
{
	
	m_bIsRunning = false;
	
	[m_GLView release];
	
	[m_window release];	
	
	[super dealloc];
	
}

+(GLoftApp*)sharedInstance
{
	return g_sharedInstance;
} 

#ifdef USE_IPHONE_KEYBOARD
-(void)textInputInit:(TextInputInitParams*)params
{
	TextInput* ti = [[TextInput alloc] initWithTextBuffer:(char*)params->textBuff WithSize:params->buffSize AtX:params->x Y:params->y Width:params->w Height:params->h];
	[TextInput setSharedInstance:ti];
}
#endif 





@end