
#import "Accelerometer.h"
#include <math.h>

#define kAccelerometerFrequency		30.0 //Hz
#define kFilteringFactor			0.4

extern void debug_out(const char* x, ...);

static Accelerometer* _sharedInstance = nil;

@implementation Accelerometer

- (id)init
{
    self = [super init];
    
    if (self != nil) 
	{
		//Configure and start accelerometer
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / kAccelerometerFrequency)];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];		
    }
    return self;
}

- (void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
	dx = x;
	dy = y;
	dz = z;
	
	//Use a basic low-pass filter to only keep the gravity in the accelerometer values
//	x = acceleration.x * kFilteringFactor + x * (1.0 - kFilteringFactor);
//	y = acceleration.y * kFilteringFactor + y * (1.0 - kFilteringFactor);
//	z = acceleration.z * kFilteringFactor + z * (1.0 - kFilteringFactor);

	x = acceleration.x;
	y = acceleration.y;
	z = acceleration.z;
	
	dx = x - dx;
	dy = y - dy;
	dz = z - dz;
}

+ (Accelerometer *)getSharedInstance 
{
	if (_sharedInstance == nil)
		_sharedInstance = [[Accelerometer alloc] init];
	
	return _sharedInstance;
}

@end


//#define ORIENTATION_INVALID				(-1)
//#define ORIENTATION_PORTRAIT			(1 << 0)
//#define ORIENTATION_LANDSCAPE_90		(1 << 1)
//#define ORIENTATION_LANDSCAPE_270		(1 << 2)

#define ORIENTATION_TRIGER				0.7f
#define BALLANCE_MAX					0.5f

#define ONE 4096

extern "C" int GetPhoneOrientation()
{
	Accelerometer* accel = [Accelerometer getSharedInstance];
	
	float x = (float)(accel->x);
	float y = (float)(accel->y);
	//float z = (float)(accel->z);
	
	if (y < -ORIENTATION_TRIGER)
		return ORIENTATION_PORTRAIT;
	else if (x > ORIENTATION_TRIGER)
		return ORIENTATION_LANDSCAPE_90;
	else if (x < -ORIENTATION_TRIGER)
		return ORIENTATION_LANDSCAPE_270;
	else
		return ORIENTATION_INVALID;

}

extern "C" int GetPhoneRotLeftRight(int phoneOrientation)
{
	Accelerometer* accel = [Accelerometer getSharedInstance];
	
	float x = (float)(accel->x);
	float y = (float)(accel->y);
	float angle = 0.0f;
	
	switch (phoneOrientation)
	{
		case ORIENTATION_PORTRAIT:
			angle = atan2f(x, -y);
			break;
		
		case ORIENTATION_LANDSCAPE_90:
			angle = atan2f(y, x);
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			angle = atan2f(-y, -x);
			break;
	}
	
	return angle * ONE / M_PI_2;
}

extern "C" int GetPhoneRotSpeedLeftRight(int phoneOrientation)
{
	Accelerometer* accel = [Accelerometer getSharedInstance];
	
	float x = (float)(accel->x);
	float y = (float)(accel->y);
	
	float dx = (float)(accel->dx * kAccelerometerFrequency);
	float dy = (float)(accel->dy * kAccelerometerFrequency);

	
	float dAlpha = 0.0f;
	
	switch (phoneOrientation)
	{
		case ORIENTATION_PORTRAIT:
			dAlpha = (dx * (-y) - x * (-dy)) / (x * x + y * y);
			break;
			
		case ORIENTATION_LANDSCAPE_90:
			dAlpha = (dy * x - y * dx) / (x * x + y * y);
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			dAlpha = ((-dy) * (-x) - (-y) * (-dx)) / (x * x + y * y);
			break;
	}
	
	return dAlpha * ONE;
}


extern "C" int GetPhoneRotFwdBack(int phoneOrientation)
{
	Accelerometer* accel = [Accelerometer getSharedInstance];
	
	float x = (float)(accel->x);
	float y = (float)(accel->y);
	float z = (float)(accel->z);
	
	float angle = 0.0f;
	
	switch (phoneOrientation)
	{
		case ORIENTATION_PORTRAIT:
			angle = atan2f(-z, -y);
			break;
			
		case ORIENTATION_LANDSCAPE_90:
			angle = atan2f(-z, x);
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			angle = atan2f(-z, -x);
			break;
	}
	
	return angle * ONE / M_PI_2; 	// rad per second
}


extern "C" int GetPhoneRotSpeedFwdBack(int phoneOrientation)
{
	Accelerometer* accel = [Accelerometer getSharedInstance];
	
	float x = (float)(accel->x);
	float y = (float)(accel->y);
	float z = (float)(accel->z);
	
	float dx = (float)(accel->dx * kAccelerometerFrequency);
	float dy = (float)(accel->dy * kAccelerometerFrequency);
	float dz = (float)(accel->dz * kAccelerometerFrequency);
	
	float dAlpha = 0.0f;
	
	switch (phoneOrientation)
	{
		case ORIENTATION_PORTRAIT:
			dAlpha = ((-dz) * (-y) - (-z) * (-dy)) / ( z * z + y * y);
			break;
			
		case ORIENTATION_LANDSCAPE_90:
			dAlpha = ((-dz) * x - (-z) * dx) / ( z * z + x * x);
			break;
			
		case ORIENTATION_LANDSCAPE_270:
			dAlpha = ((-dz) * (-x) - (-z) * (-dx)) / ( z * z + x * x);
			break;
	}
	
	return dAlpha * ONE; 	// rad per second
}


