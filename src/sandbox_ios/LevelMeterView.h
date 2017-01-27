
#import <UIKit/UIKit.h>
#import "wave_recorder.h"

@interface LevelMeterView : UIView
{
    UIView* m_rmsLine;
    UIView* m_peakLine;
    UIView* m_peakEnvBar;
    UIView* m_clipIndicator;
    
    WaveRealtimeInfo m_currentInfo;
}

-(void)updateInfo:(WaveRealtimeInfo*)info;

@end
