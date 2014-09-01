
#import <UIKit/UIKit.h>

@interface LevelMeterView : UIView
{
    UIView* rmsLinView;
    UIView* rmsLinEnvView;
    UIView* rmsLogView;
    UIView* rmsLogEnvView;
    UIView* peakLinView;
    UIView* peakLinEnvView;
    UIView* peakLogView;
    UIView* peakLogEnvView;
    UIView* clipView;
}

@end
