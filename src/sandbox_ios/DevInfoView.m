#import "DevInfoView.h"

@implementation DevInfoView

-(id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    m_controlEventFIFOLevelView = [[BufferLevelView alloc] initWithFrame:frame
                                                                        :@"ctrl"];
    m_audioRecordingFIFOLevelView = [[BufferLevelView alloc] initWithFrame:frame
                                                                          :@"rec"];
    m_notificationFIFOLevelView = [[BufferLevelView alloc] initWithFrame:frame
                                                                           :@"not"];
    
    [self addSubview:m_controlEventFIFOLevelView];
    [self addSubview:m_audioRecordingFIFOLevelView];
    [self addSubview:m_notificationFIFOLevelView];
    
    return self;
}

-(void)layoutSubviews
{
    [self updateInfo:&m_currentInfo];
    
    const int w = self.frame.size.width;
    const int h = self.frame.size.height;
    const int mw = w / 3;
    
    m_controlEventFIFOLevelView.frame = CGRectMake(0 * mw, 0, mw, h);
    m_notificationFIFOLevelView.frame = CGRectMake(1 * mw, 0, mw, h);
    m_audioRecordingFIFOLevelView.frame = CGRectMake(2 * mw, 0, mw, h);
    
}

-(void)updateInfo:(WaveDevInfo*)info
{
    memcpy(&m_currentInfo, info, sizeof(WaveDevInfo));
    
    [m_controlEventFIFOLevelView updateLevel:m_currentInfo.controlEventFIFOLevel];
    [m_notificationFIFOLevelView updateLevel:m_currentInfo.notificationFIFOLevel];
    [m_audioRecordingFIFOLevelView updateLevel:m_currentInfo.recordFIFOLevel];
    
}

@end
