
#import "MoviePlayer.h"
#import "GLoftApp.h"
volatile bool g_bExecPlayHasFinished = false;
extern void debug_out(const char* x, ...);

static MoviePlayer* g_sharedMoviePlayerInstance = nil;

@implementation MoviePlayer

#pragma mark Movie Player Routines


//  Notification called when the movie finished preloading.
// - (void) moviePreloadDidFinish:(NSNotification*)notification
// {
     
    //   < add your code here >
    
// }

//  Notification called when the movie finished playing.
- (void) moviePlayBackDidFinish:(NSNotification*)notification
{	
	isMoviePlaying = false;
	//printf("MovieFinished\n");
}

//  Notification called when the movie scaling mode has changed.
//- (void) movieScalingModeDidChange:(NSNotification*)notification
//{
     
//        < add your code here >
        
//        For example:
//            MPMoviePlayerController* theMovie=[aNotification object];
//            etc.
//}


-(id)initWithFile:(NSString*)fileName
{
	self = [super init];
    
    if (self != nil) 
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
		/*

		 Now create a MPMoviePlayerController object using the movie file provided in our bundle.

		 The MPMoviePlayerController class supports any movie or audio files that already play 
		 correctly on an iPod or iPhone. For movie files, this typically means files with the extensions 
		 .mov, .mp4, .mpv, and .3gp and using one of the following compression standards:

			- H.264 Baseline Profile Level 3.0 video, up to 640 x 480 at 30 fps. Note that B frames 
				are not supported in the Baseline profile.
        
			- MPEG-4 Part 2 video (Simple Profile)

		 If you use this class to play audio files, it displays a black screen while the audio plays. For 
		 audio files, this class class supports AAC-LC audio at up to 48 kHz.

		 */
		
		mMoviePlayer = nil;
		NSURL *movieURL = nil;
		NSBundle *bundle = [NSBundle mainBundle];
        if (bundle)
        {
            NSString *moviePath = [bundle pathForResource:fileName ofType:nil];
            if (moviePath)
            {
                movieURL = [NSURL fileURLWithPath:moviePath];
				
				mMoviePlayer = [[MPMoviePlayerController alloc] initWithContentURL:movieURL];
            }
        }
		
		if (mMoviePlayer == nil)
		{
			[self dealloc];
			return nil;
		}
		
		/*
		 
		 Because it takes time to load movie files into memory, MPMoviePlayerController
		 automatically begins loading your movie file shortly after you initialize a new 
		 instance. When it is done preloading the movie file, it sends the
		 MPMoviePlayerContentPreloadDidFinishNotification notification to any registered 
		 observers. If an error occurred during loading, the userInfo dictionary of the 
		 notification object contains the error information. If you call the play method 
		 before preloading is complete, no notification is sent and your movie begins 
		 playing as soon as it is loaded into memory.
		 
		 */
		
		// Register to receive a notification when the movie is in memory and ready to play.
//		[[NSNotificationCenter defaultCenter] addObserver:self 
//												 selector:@selector(moviePreloadDidFinish:) 
//													 name:MPMoviePlayerContentPreloadDidFinishNotification 
//												   object:mMoviePlayer];
		
		/*
			In addition to the MPMoviePlayerContentPreloadDidFinishNotification notification,
			the MPMoviePlayerPlaybackDidFinishNotification notification is sent to
			registered observers when the movie has finished playing, and the 
			MPMoviePlayerScalingModeDidChangeNotification notification is sent when the 
			movie scaling mode has changed.
		 */
		
		// Register to receive a notification when the movie has finished playing. 
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(moviePlayBackDidFinish:) 
													 name:MPMoviePlayerPlaybackDidFinishNotification 
												   object:mMoviePlayer];
     
		// Register to receive a notification when the movie scaling mode has changed. 
//		[[NSNotificationCenter defaultCenter] addObserver:self 
//												 selector:@selector(movieScalingModeDidChange:) 
//													 name:MPMoviePlayerScalingModeDidChangeNotification 
//												   object:mMoviePlayer];

		/* Set movie player settings (scaling, controller type and background color) to the currently set values
			as specified in the Settings application */
		
		/* 
			Movie scaling mode can be one of: MPMovieScalingModeNone, MPMovieScalingModeAspectFit,
				MPMovieScalingModeAspectFill, MPMovieScalingModeAspectFit.
		 */
		mMoviePlayer.scalingMode = MPMovieScalingModeNone;
    
		/* 
			Movie control mode can be one of: MPMovieControlModeDefault, MPMovieControlModeVolumeOnly,
				MPMovieControlModeHidden.
		 */
		mMoviePlayer.movieControlMode = MPMovieControlModeDefault;

		/*
			The color of the background area behind the movie can be any UIColor value.
		 */
		mMoviePlayer.backgroundColor = [UIColor blackColor];
		
		[pool release];
		
		isMoviePlaying = false;
	}
	
    return self;
}

+(MoviePlayer*)getSharedInstance
{
	return g_sharedMoviePlayerInstance;
}

-(void)execPlay
{
	//printf("Exec Play\n");
	[mMoviePlayer play];
	
	g_bExecPlayHasFinished = true;
}

-(IBAction)playMovie
{
    /*
                       
     As soon as you call the play: method, the player initiates a transition that fades 
     the screen from your current window content to the designated background 
     color of the player. If the movie cannot begin playing immediately, the player 
     object continues displaying the background color and may also display a progress 
     indicator to let the user know the movie is loading. When playback finishes, the 
     player uses another fade effect to transition back to your window content.
     
     */
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	//printf("PlayMovie\n");
	//[mMoviePlayer play];	

	g_bExecPlayHasFinished = false;
	
	[NSTimer
	 scheduledTimerWithTimeInterval:0.0
	 target: self
	 selector: @selector(execPlay)
	 userInfo: nil
	 repeats: NO
	 ];
	//mMoviePlayer.movieControlMode = MPMovieControlModeDefault;
	//mMoviePlayer.scalingMode = MPMovieScalingModeNone;
	
	[pool release];
		
	while(!g_bExecPlayHasFinished)
	{
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, false);		
	}
	usleep(10000);
	isMoviePlaying = true;
}

-(IBAction)stopMovie
{
    /*
	 
     As soon as you call the play: method, the player initiates a transition that fades 
     the screen from your current window content to the designated background 
     color of the player. If the movie cannot begin playing immediately, the player 
     object continues displaying the background color and may also display a progress 
     indicator to let the user know the movie is loading. When playback finishes, the 
     player uses another fade effect to transition back to your window content.
     
     */
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
    [mMoviePlayer stop];	
	
	[pool release];	
	
//	mMoviePlayer.movieControlMode = MPMovieControlModeHidden;
//  observer responsable for this	
//	isMoviePlaying = false;
}

-(Boolean)isPlaying
{
	return isMoviePlaying;
}


#pragma mark View Controller Routines

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return YES;
}

- (void)freeMovie
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	if (mMoviePlayer)
	{
		// remove movie notifications
		//[[NSNotificationCenter defaultCenter] removeObserver:self
		//												name:MPMoviePlayerContentPreloadDidFinishNotification
		//											  object:mMoviePlayer];
		
		[[NSNotificationCenter defaultCenter] removeObserver:self
														name:MPMoviePlayerPlaybackDidFinishNotification
													  object:mMoviePlayer];
		
		//[[NSNotificationCenter defaultCenter] removeObserver:self
		//												name:MPMoviePlayerScalingModeDidChangeNotification
		//											  object:mMoviePlayer];
		
		// free our movie player
		[mMoviePlayer release];
	}

	mMoviePlayer = nil;
	
	[pool release];
}

- (void)dealloc
{    
	[self freeMovie];
    [super dealloc];
}


@end

static bool s_isMoviePaused = false; 
static bool s_shouldHideStatusBar = false;

// wrappers
extern "C" void LoadMovie(const char* movieFileName)
{
	if (g_sharedMoviePlayerInstance != nil)
		[g_sharedMoviePlayerInstance release];

	g_sharedMoviePlayerInstance = nil;
	
	NSString* name = [[NSString alloc] initWithUTF8String:movieFileName];
	g_sharedMoviePlayerInstance = [[MoviePlayer alloc] initWithFile:name];
}

extern "C" void PlayMovie()
{
	if(g_sharedMoviePlayerInstance != nil)
	{
		[g_sharedMoviePlayerInstance playMovie];
	}	
}

extern "C" void showStatusBar(bool bShow);
extern "C" void StopMovie()
{	
	if (g_sharedMoviePlayerInstance != nil)
	{
		[g_sharedMoviePlayerInstance stopMovie];	
		[g_sharedMoviePlayerInstance release];
	}

	g_sharedMoviePlayerInstance = nil;
	s_isMoviePaused = false;
	
	if (s_shouldHideStatusBar)
	{
		showStatusBar(true);
		showStatusBar(false);
	}
}

extern "C" void SuspendMovie()
{
	if (g_sharedMoviePlayerInstance != nil)
	{
		if ([g_sharedMoviePlayerInstance isPlaying])
		{
			s_isMoviePaused = true;
			[g_sharedMoviePlayerInstance stopMovie];
			//printf("SuspendMovie\n");
		}
		else
		{	
			s_shouldHideStatusBar = true;
		}
	}
}

extern "C" void ResumeMovie()
{
	if (g_sharedMoviePlayerInstance != nil && s_isMoviePaused)
	{		
		[g_sharedMoviePlayerInstance playMovie];
		s_isMoviePaused = false;
		//printf("ResumeMovie\n");
	}
}

extern "C" bool IsMoviePaused()
{
	return s_isMoviePaused;
}
extern "C" bool IsMoviePlaying()
{
	if (g_sharedMoviePlayerInstance != nil)
		return [g_sharedMoviePlayerInstance isPlaying];
	
	return false;
}
