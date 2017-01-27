
#import <UIKit/UIKit.h>
#import "wave_recorder.h"

@interface LevelMeterView : UIView
{
    UIView* m_rmsLine;
    UIView* m_peakLine;
    UIView* m_peakEnvBar;
    UIView* m_clipIndicator;
    
    drRealtimeInfo m_currentInfo;
}

-(void)updateInfo:(drRealtimeInfo*)info;

@end
