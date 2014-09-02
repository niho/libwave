
#import <UIKit/UIKit.h>

@class SandboxViewController;

@interface SandboxView : UIView
{
    UIScrollView* m_scrollView;
    int m_currUIRow;
}

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc;

@end
