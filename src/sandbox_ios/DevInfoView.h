
#import <UIKit/UIKit.h>
#import "wave_recorder.h"
#import "BufferLevelView.h"

@interface DevInfoView : UIView
{
    drDevInfo m_currentInfo;
    BufferLevelView* m_controlEventFIFOLevelView;
    BufferLevelView* m_audioRecordingFIFOLevelView;
    BufferLevelView* m_notificationFIFOLevelView;
    
}

-(void)updateInfo:(drDevInfo*)info;

@end
