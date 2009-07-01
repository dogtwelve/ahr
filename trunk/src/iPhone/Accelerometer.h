
#undef WIN32
#import <UIKit/UIAccelerometer.h>

#define ORIENTATION_INVALID				(-1)
#define ORIENTATION_PORTRAIT			(1 << 0)
#define ORIENTATION_LANDSCAPE_90		(1 << 1)
#define ORIENTATION_LANDSCAPE_270		(1 << 2)


@interface Accelerometer : NSObject <UIAccelerometerDelegate>
{
@public
	UIAccelerationValue		x, y, z;
	UIAccelerationValue		dx, dy, dz;
}

- (id)init;
+ ( Accelerometer *)getSharedInstance;

@end
