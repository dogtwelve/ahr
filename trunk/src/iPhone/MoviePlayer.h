
#undef WIN32
#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>


@interface MoviePlayer : NSObject
{
    MPMoviePlayerController *mMoviePlayer;
	Boolean isMoviePlaying;
}

-(id)initWithFile:(NSString*)fileName;
-(IBAction)playMovie;
-(IBAction)stopMovie;
-(Boolean)isPlaying;
+(MoviePlayer*)getSharedInstance;

@end
