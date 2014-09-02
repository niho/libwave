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
    
    //level meters
    {
        UILabel* l = [[UILabel alloc] init];
        l.text = @"Latest notification";
        l.textAlignment = NSTextAlignmentCenter;
        [self addUIRow:l];
        
        self.latestNotificationLabel = [[UILabel alloc] init];
        self.latestNotificationLabel.textAlignment = NSTextAlignmentCenter;
        self.latestNotificationLabel.alpha = 0.7f;
        [self addUIRow:self.latestNotificationLabel];
    }
    
    //rec control buttons
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
        [recFinishButton addTarget:vc action:@selector(onRecFinish:) forControlEvents:UIControlEventTouchUpInside];
        
        UILabel* l = [[UILabel alloc] init];
        l.text = @"Recording";
        l.textAlignment = NSTextAlignmentCenter;
        [self addUIRow:l];
        
        [self addUIRow:recStartButton
                      :recPauseButton
                      :recCancelButton
                      :recResumeButton
                      :recFinishButton];
    }
    
    //level meters
    {
        UILabel* l = [[UILabel alloc] init];
        l.text = @"Levels";
        l.textAlignment = NSTextAlignmentCenter;
        [self addUIRow:l];
    }
    
    m_scrollView.contentSize = CGSizeMake(self.frame.size.width, m_currUIRow * kUIRowHeight);
    [self addSubview:m_scrollView];
    
    return self;
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
