
#import <UIKit/UIKit.h>
#import "digger_recorder.h"

@interface LevelMeterView : UIView
{
    UIView* m_rmsLine;
    UIView* m_peakLine;
    UIView* m_peakEnvBar;
    UIView* m_clipIndicator;
    
    drLevels m_currentLevels;
}

-(void)updateLevels:(drLevels*)levels;

@end
