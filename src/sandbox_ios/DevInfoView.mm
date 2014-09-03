#import "DevInfoView.h"

@implementation DevInfoView

-(id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    
    
    return self;
}

-(void)layoutSubviews
{
    [self updateInfo:&m_currentInfo];
}

-(void)updateInfo:(drDevInfo*)info
{
    memcpy(&m_currentInfo, info, sizeof(drDevInfo));
    
    const int w = self.frame.size.width;
    const int h = self.frame.size.height;
    const int lw = 4;
}

@end
