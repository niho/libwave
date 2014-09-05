
#import <UIKit/UIKit.h>
#import "DevInfoView.h"
#import "LevelMeterView.h"

@class SandboxViewController;

@interface SandboxView : UIView
{
    UIScrollView* m_scrollView;
    int m_currUIRow;
}

@property (nonatomic, retain) UILabel* latestErrorLabel;
@property (nonatomic, retain) UILabel* latestNotificationLabel;
@property (nonatomic, retain) LevelMeterView* levelMeterView;
@property (nonatomic, retain) DevInfoView* devInfoView;

@property (nonatomic, retain) UIButton* recToggleButton;
@property (nonatomic, retain) UIButton* recPauseButton;
@property (nonatomic, retain) UIButton* recCancelButton;

@property (nonatomic, retain) UIButton* drInitButton;
@property (nonatomic, retain) UIButton* drDeinitButton;

@property (nonatomic, retain) UISegmentedControl* updateIntervalButton;

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc;

@end
