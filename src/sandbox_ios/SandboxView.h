
#import <UIKit/UIKit.h>

@class SandboxViewController;

@interface SandboxView : UIView
{
    UIScrollView* m_scrollView;
    int m_currUIRow;
}

@property (nonatomic, retain) UILabel* latestNotificationLabel;

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc;

@end
