
#import <UIKit/UIKit.h>
#import "LevelMeterView.h"

@class SandboxViewController;

@interface SandboxView : UIView
{
    UIScrollView* m_scrollView;
    int m_currUIRow;
}

@property (nonatomic, retain) UILabel* latestNotificationLabel;
@property (nonatomic, retain) LevelMeterView* levelMeterView;

@property (nonatomic, retain) UIButton* recToggleButton;
@property (nonatomic, retain) UIButton* recPauseButton;
@property (nonatomic, retain) UIButton* recCancelButton;

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc;

@end
