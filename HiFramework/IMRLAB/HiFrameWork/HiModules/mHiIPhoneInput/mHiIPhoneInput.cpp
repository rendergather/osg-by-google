
///////////////////////////////////////////////////
// module name: mHiIPhoneInput
// module creation date: 09-07-09
// Generated by hiFramework module frame Maker 1.0
///////////////////////////////////////////////////

#include "HiModules/mHiIPhoneInput/mHiIPhoneInput.h"
#include "HiKernel/HiOSGRenderer.h"
#include "HiKernel/HiEventManager.h"
#include "HiModules/enumerations.h"
#include "HiClient/HiCocoaTouchEAGLView.h"


#include "HiKernel/HiXmlNode.h"
#include "HiKernel/HiTelegram.h"
#include "HiKernel/HiModuleManager.h"


#import <corelocation/CoreLocation.h>

using namespace HiModules;

@interface AccelerationGetter : UIView
<UIAccelerometerDelegate, CLLocationManagerDelegate>
{
    UIAccelerationValue   acceleration_value_x_;
    UIAccelerationValue   acceleration_value_y_;
    UIAccelerationValue   acceleration_value_z_;
    
	//  CLLocationManager*      location_manager_;
	//  CLLocationDirection     magnetic_heading_;
	
	EAGLView*             glView;
	
}

+ (AccelerationGetter*) getInstance;
- (UIAccelerationValue) getAccelerationValueX;
- (UIAccelerationValue) getAccelerationValueY;
- (UIAccelerationValue) getAccelerationValueZ;
//- (CLLocationDirection) getMagneticHeading;
//- (CLLocationManager*)  getLocationManager;
- (void)                start;
- (void)                stop;

//터치센서
- (void)    touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)    touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)    touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;


@end


id AccelerationGetter_instance = nil;

@implementation AccelerationGetter

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
    // acceleration 값 보관. -- 1
    acceleration_value_x_ = [acceleration x];
    acceleration_value_y_ = [acceleration y];
    acceleration_value_z_ = [acceleration z];
}

- (void)dealloc
{
//    [location_manager_ release];
    [super dealloc];
}

+ (AccelerationGetter*) getInstance
{
    if (nil == AccelerationGetter_instance)
    {
        AccelerationGetter_instance = [[AccelerationGetter alloc] init];

    }
    
    return AccelerationGetter_instance;
}

- (UIAccelerationValue)           getAccelerationValueX
{
    return acceleration_value_x_;
}
- (UIAccelerationValue)           getAccelerationValueY
{
    return acceleration_value_y_;
}
- (UIAccelerationValue)           getAccelerationValueZ
{
    return acceleration_value_z_;
}

//- (CLLocationDirection) getMagneticHeading
//{
//    return magnetic_heading_;
//}
//
//- (CLLocationManager*)  getLocationManager;
//{
//    return location_manager_;
//}

- (id) init
{
    if (self = [super init])
    {
#if defined (HI_IPAD)
		self.frame = CGRectMake(0, 0, 768, 1024); // IPAD Resoultion
#elif defined (HI_IPHONE)
		self.frame = CGRectMake(0, 0, 320, 480); // iPhone Resoultion
#endif
//        location_manager_ = [[CLLocationManager alloc] init];
//        location_manager_.headingFilter = kCLHeadingFilterNone;
//        location_manager_.delegate = self;
    }
    
    return self;
}

//- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)heading
//{
//    magnetic_heading_ = heading.magneticHeading;
//}
//
//- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
//{
//    if (kCLErrorDenied == [error code])
//    {
//        [manager stopUpdatingHeading];
//        
//    }else if (kCLErrorHeadingFailure)
//    {
//    }
//}

- (void)                start
{
    [[UIAccelerometer sharedAccelerometer] setDelegate: AccelerationGetter_instance];
}

- (void)                stop
{
    [[UIAccelerometer sharedAccelerometer] setDelegate: nil];
}

//터치센서
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSSet *allTouches = [event allTouches];
	CGPoint _Point;
	switch ([allTouches count]) 
	{
		case 1: 
		{ //Single touch
			UITouch *touch1 = [[allTouches allObjects] objectAtIndex:0];
			_Point = [touch1 locationInView:self];
			NSLog(@"1 touch",_Point.x,_Point.y);
			//osg::Vec3 Test;
			//Test.set(_Point.x,_Point.y, 0.0f);
#ifdef HI_OSG_RENDERER
			HiKernel::OSGRenderer->getViewer()->TouchEvent(osgViewer::ONE_TOUCH_DOWN, _Point.x,_Point.y);
			HiKernel::Dispatch->GiveMessage(HiKernel::SEND_MSG_IMMEDIATELY, "gameworld", "gameworld", HiModules::IPHONE_TOUCHED, &_Point);
#endif			
			//cal3d모듈에 애니메이션 바뀌게 하는 부분
//			Dispatch->DispatchMessage(0, "OsgCal", "OsgCal", IPHONEINPUT_ONETOUCH_DOWN , 0);
			
		} break;
		case 2: 
		{ //Double Touch
			UITouch *touch2 = [[allTouches allObjects] objectAtIndex:1];
			_Point = [touch2 locationInView:self];
#ifdef HI_OSG_RENDERER
			HiKernel::OSGRenderer->getViewer()->TouchEvent(osgViewer::TWO_TOUCH_DOWN, _Point.x,_Point.y);
#endif
			NSLog(@"2 touch");
		} break;
		default:
			break;
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint _Point;
	NSSet *allTouches = [event allTouches];
	
	switch ([allTouches count])
	{
		case 1: 
		{
			UITouch *touch1 = [[allTouches allObjects] objectAtIndex:0];
			_Point = [touch1 locationInView:self];
#ifdef HI_OSG_RENDERER			
			HiKernel::OSGRenderer->getViewer()->TouchEvent(osgViewer::ONE_TOUCH_DOWN, _Point.x,_Point.y);
#endif
			NSLog(@"1 touch move");
		} break;
		case 2: 
		{
			UITouch *touch2 = [[allTouches allObjects] objectAtIndex:1];
			_Point = [touch2 locationInView:self];
#ifdef HI_OSG_RENDERER			
			HiKernel::OSGRenderer->getViewer()->TouchEvent(osgViewer::TWO_TOUCH_DOWN, _Point.x,_Point.y);
#endif			
			NSLog(@"2 touch move");
		} break;
	}
#ifdef HI_OSG_RENDERER	
	HiKernel::OSGRenderer->getViewer()->TouchEvent(5, _Point.x,_Point.y);
#endif	
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event 
{	
	CGPoint _Point;
	NSSet *allTouches = [event allTouches];
	
	switch ([allTouches count])
	{
		case 1:
		{
			UITouch *touch1 = [[allTouches allObjects] objectAtIndex:0];
			_Point = [touch1 locationInView:self];		
#ifdef HI_OSG_RENDERER			
			HiKernel::OSGRenderer->getViewer()->TouchEvent(osgViewer::ONE_TOUCH_UP, _Point.x,_Point.y);
#endif			
			NSLog(@"1 touch end");
		}
			break;
		case 2:
		{
			UITouch *touch2 = [[allTouches allObjects] objectAtIndex:1];
			_Point = [touch2 locationInView:self];
#ifdef HI_OSG_RENDERER
			HiKernel::OSGRenderer->getViewer()->TouchEvent(osgViewer::TWO_TOUCH_UP, _Point.x,_Point.y);
#endif			
			NSLog(@"2 touch end");
			break;
		}
	}	
}


@end

mHiIPhoneInput::mHiIPhoneInput(HiKernel::HiXmlNode*xml)
{
	acceleration_getter_ = nil;
}

mHiIPhoneInput::~mHiIPhoneInput()
{
}

bool mHiIPhoneInput::HandleMessage(const HiKernel::HiTelegram& msg)
{
	if(MSG_SIZE <= msg.Msg)
		return false;
	
	if(msg.Msg == 0)
	{
		m_EaglView = (EAGLView*)msg.ExtraInfo;

		[m_EaglView addSubview:acceleration_getter_];
//		acceleration_getter_.backgroundColor = [UIColor redColor];
//		[m_EaglView setUserInteractionEnabled: YES];
//		[acceleration_getter_ setUserInteractionEnabled: YES];
	}
	

	return true;
}

void mHiIPhoneInput::PostConfig()
{
	acceleration_getter_ = [AccelerationGetter getInstance];
	
//	[[acceleration_getter_ getLocationManager] startUpdatingHeading];
	[acceleration_getter_ start];
	
}

void mHiIPhoneInput::Update()
{
	acceleration_value_x_ = [acceleration_getter_ getAccelerationValueX];
    acceleration_value_y_ = [acceleration_getter_ getAccelerationValueY];
    acceleration_value_z_ = [acceleration_getter_ getAccelerationValueZ];
    
//    magnetic_heading_ = [acceleration_getter_ getMagneticHeading];
	
//	NSLog(@"OsgInput x = %f, y = %f, z = %f",acceleration_value_x_, acceleration_value_y_, acceleration_value_z_ );
}

void mHiIPhoneInput::Render()
{
}

void mHiIPhoneInput::Terminate()
{
	[acceleration_getter_ release];
}
