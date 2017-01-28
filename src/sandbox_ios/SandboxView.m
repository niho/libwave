#import "SandboxViewController.h"
#import "SandboxView.h"

@implementation SandboxView

#define kUIRowHeight (45)

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc
{
    self = [super initWithFrame:frame];
    
    m_currUIRow = 0;
    self.backgroundColor = [UIColor whiteColor];
    m_scrollView = [[UIScrollView alloc] initWithFrame:self.bounds];
    
    //init/deinit buttons
    {
        [self addUITitleRow:@"Digger Recorder lifecycle"];
        self.waveInitButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [self.waveInitButton setTitle:@"Initialize" forState:UIControlStateNormal];
        [self.waveInitButton addTarget:vc action:@selector(onInit:)
                        forControlEvents:UIControlEventTouchUpInside];
        
        self.waveDeinitButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [self.waveDeinitButton setTitle:@"Shut down" forState:UIControlStateNormal];
        [self.waveDeinitButton addTarget:vc action:@selector(onDeinit:)
                      forControlEvents:UIControlEventTouchUpInside];
        
        [self addUIRow:self.waveInitButton
                      :self.waveDeinitButton];
    }
    
    //latest notification
    {
        [self addUITitleRow:@"Latest notification"];
        
        self.latestNotificationLabel = [[UILabel alloc] init];
        self.latestNotificationLabel.textAlignment = NSTextAlignmentCenter;
        self.latestNotificationLabel.alpha = 0.5f;
        [self addUIRow:self.latestNotificationLabel];
    }
    
    //latest error
    {
        [self addUITitleRow:@"Latest error"];
        self.latestErrorLabel = [[UILabel alloc] init];
        self.latestErrorLabel.textAlignment = NSTextAlignmentCenter;
        self.latestErrorLabel.alpha = 0.5f;
        [self addUIRow:self.latestErrorLabel];
    }
    
    //rec buttons
    {
        [self addUITitleRow:@"Recording controls"];
        
        self.recToggleButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [self.recToggleButton setTitle:@"start" forState:UIControlStateNormal];
        
        self.recPauseButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [self.recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
        self.recPauseButton.enabled = NO;
        
        [self addUIRow:self.recToggleButton
                      :self.recPauseButton];
    }
    
    //rec dev control buttons
    if (0)
    {
        UIButton* recStartButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recStartButton setTitle:@"start" forState:UIControlStateNormal];
        [recStartButton addTarget:vc action:@selector(onRecStart:) forControlEvents:UIControlEventTouchUpInside];
        
        UIButton* recPauseButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
        [recPauseButton addTarget:vc action:@selector(onRecPause:) forControlEvents:UIControlEventTouchUpInside];
        
        UIButton* recResumeButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recResumeButton setTitle:@"resume" forState:UIControlStateNormal];
        [recResumeButton addTarget:vc action:@selector(onRecResume:) forControlEvents:UIControlEventTouchUpInside];
        
        UIButton* recCancelButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recCancelButton setTitle:@"cancel" forState:UIControlStateNormal];
        [recCancelButton addTarget:vc action:@selector(onRecCancel:) forControlEvents:UIControlEventTouchUpInside];
        
        UIButton* recFinishButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recFinishButton setTitle:@"finish" forState:UIControlStateNormal];
        [recFinishButton addTarget:vc action:@selector(onRecStop:) forControlEvents:UIControlEventTouchUpInside];
        
        [self addUITitleRow:@"Recording dev controls"];
        
        [self addUIRow:recStartButton
                      :recPauseButton
                      :recResumeButton
                      :recFinishButton
                      :recCancelButton];
    }
    
    //level meters
    {
        [self addUITitleRow:@"Input level"];
        self.levelMeterView = [[LevelMeterView alloc] init];
        [self addUIRow:self.levelMeterView];
    }
    
    //update interval button
    {
        [self addUITitleRow:@"Poll frequency"];
        self.updateIntervalButton = [[UISegmentedControl alloc] initWithItems:[NSArray arrayWithObjects:@"100%", @"50%", @"25%", @"10%", @"0%", nil]];
        self.updateIntervalButton.selectedSegmentIndex = 0;
        [self.updateIntervalButton addTarget:vc action:@selector(onUpdateIntervalChanged:) forControlEvents:UIControlEventValueChanged];
        [self addUIRow:self.updateIntervalButton];
        
    }
    
    //dev info
    {
        [self addUITitleRow:@"Dev info"];
        self.devInfoView = [[DevInfoView alloc] init];
        [self addUIRow:self.devInfoView];
        
        const float a = 0.2f;
        
        self.errorFIFOUnderrunLabel = [[UILabel alloc] init];
        self.errorFIFOUnderrunLabel.text = @"Err FIFO";
        self.errorFIFOUnderrunLabel.textColor = [UIColor redColor];
        self.errorFIFOUnderrunLabel.alpha = a;
        
        self.controlFIFOUnderrunLabel = [[UILabel alloc] init];
        self.controlFIFOUnderrunLabel.text = @"Ctrl FIFO";
        self.controlFIFOUnderrunLabel.textColor = [UIColor redColor];
        self.controlFIFOUnderrunLabel.alpha = a;
        
        self.notificationFIFOUnderrunLabel = [[UILabel alloc] init];
        self.notificationFIFOUnderrunLabel.text = @"Not FIFO";
        self.notificationFIFOUnderrunLabel.textColor = [UIColor redColor];
        self.notificationFIFOUnderrunLabel.alpha = a;
        
        self.recordingFIFOUnderrunLabel = [[UILabel alloc] init];
        self.recordingFIFOUnderrunLabel.text = @"Rec FIFO";
        self.recordingFIFOUnderrunLabel.textColor = [UIColor redColor];
        self.recordingFIFOUnderrunLabel.alpha = a;
        
        [self addUIRow:self.controlFIFOUnderrunLabel
                      :self.notificationFIFOUnderrunLabel
                      :self.recordingFIFOUnderrunLabel
                      :self.errorFIFOUnderrunLabel];
    }
    
    m_scrollView.contentSize = CGSizeMake(self.frame.size.width, m_currUIRow * kUIRowHeight);
    [self addSubview:m_scrollView];
    
    return self;
}

-(void)addUITitleRow:(NSString*)title
{
    UILabel* l = [[UILabel alloc] init];
    l.text = title;
    l.textAlignment = NSTextAlignmentCenter;
    l.backgroundColor = [UIColor colorWithWhite:0.9f alpha:1.0f];
    [self addUIRow:l];
}

-(void)addUIRow:(UIView*)v0
{
    const int w = self.frame.size.width;
    v0.frame = CGRectMake(0, m_currUIRow * kUIRowHeight, w, kUIRowHeight);
    [m_scrollView addSubview:v0];
    
    m_currUIRow++;
}

-(void)addUIRow:(UIView*)v0
               :(UIView*)v1
{
    const int vw = self.frame.size.width / 2;
    
    v0.frame = CGRectMake(0 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v1.frame = CGRectMake(1 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    
    [m_scrollView addSubview:v0];
    [m_scrollView addSubview:v1];
    
    m_currUIRow++;
}

-(void)addUIRow:(UIView*)v0
               :(UIView*)v1
               :(UIView*)v2
{
    const int vw = self.frame.size.width / 3;
    
    v0.frame = CGRectMake(0 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v1.frame = CGRectMake(1 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v2.frame = CGRectMake(2 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    
    [m_scrollView addSubview:v0];
    [m_scrollView addSubview:v1];
    [m_scrollView addSubview:v2];
    
    m_currUIRow++;
}

-(void)addUIRow:(UIView*)v0
               :(UIView*)v1
               :(UIView*)v2
               :(UIView*)v3
{
    const int vw = self.frame.size.width / 4;
    
    v0.frame = CGRectMake(0 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v1.frame = CGRectMake(1 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v2.frame = CGRectMake(2 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v3.frame = CGRectMake(3 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    
    [m_scrollView addSubview:v0];
    [m_scrollView addSubview:v1];
    [m_scrollView addSubview:v2];
    [m_scrollView addSubview:v3];
    
    m_currUIRow++;
}


-(void)addUIRow:(UIView*)v0
               :(UIView*)v1
               :(UIView*)v2
               :(UIView*)v3
               :(UIView*)v4
{
    const int vw = self.frame.size.width / 5;
    
    v0.frame = CGRectMake(0 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v1.frame = CGRectMake(1 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v2.frame = CGRectMake(2 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v3.frame = CGRectMake(3 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    v4.frame = CGRectMake(4 * vw, m_currUIRow * kUIRowHeight, vw, kUIRowHeight);
    
    [m_scrollView addSubview:v0];
    [m_scrollView addSubview:v1];
    [m_scrollView addSubview:v2];
    [m_scrollView addSubview:v3];
    [m_scrollView addSubview:v4];
    
    m_currUIRow++;
}


@end
