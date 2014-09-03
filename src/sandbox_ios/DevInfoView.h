
#import <UIKit/UIKit.h>
#import "digger_recorder.h"

@interface DevInfoView : UIView
{
    drDevInfo m_currentInfo;
}

-(void)updateInfo:(drDevInfo*)info;

@end
