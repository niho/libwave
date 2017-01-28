
#import <UIKit/UIKit.h>
#import "DevInfoView.h"
#import "LevelMeterView.h"

@class SandboxViewController;

@interface SandboxView : UIView
{
    UIScrollView* m_scrollView;
    int m_currUIRow;
}

@property (nonatomic, retain) UILabel* errorFIFOUnderrunLabel;
@property (nonatomic, retain) UILabel* controlFIFOUnderrunLabel;
@property (nonatomic, retain) UILabel* notificationFIFOUnderrunLabel;
@property (nonatomic, retain) UILabel* recordingFIFOUnderrunLabel;

@property (nonatomic, retain) UILabel* latestErrorLabel;
@property (nonatomic, retain) UILabel* latestNotificationLabel;
@property (nonatomic, retain) LevelMeterView* levelMeterView;
@property (nonatomic, retain) DevInfoView* devInfoView;

@property (nonatomic, retain) UIButton* recToggleButton;
@property (nonatomic, retain) UIButton* recPauseButton;

@property (nonatomic, retain) UIButton* waveInitButton;
@property (nonatomic, retain) UIButton* waveDeinitButton;

@property (nonatomic, retain) UISegmentedControl* updateIntervalButton;

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc;

@end
