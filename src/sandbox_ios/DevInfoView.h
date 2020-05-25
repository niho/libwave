
#import <UIKit/UIKit.h>
#import "wave.h"
#import "BufferLevelView.h"

@interface DevInfoView : UIView
{
    WaveDevInfo m_currentInfo;
    BufferLevelView* m_controlEventFIFOLevelView;
    BufferLevelView* m_audioRecordingFIFOLevelView;
    BufferLevelView* m_notificationFIFOLevelView;
    
}

-(void)updateInfo:(WaveDevInfo*)info;

@end
