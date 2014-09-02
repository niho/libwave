#import "SandboxViewController.h"
#import "SandboxView.h"

@implementation SandboxView

#define kUIRowHeight (45)

-(id)initWithFrame:(CGRect)frame
                  :(SandboxViewController*)vc
{
    self = [super initWithFrame:frame];
    
    m_currUIRow = 0;
    
    m_scrollView = [[UIScrollView alloc] initWithFrame:self.bounds];
    
    //rec control buttons
    {
        UIButton* recStartButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recStartButton setTitle:@"rec start" forState:UIControlStateNormal];
        [recStartButton addTarget:vc action:@selector(onRecStart:) forControlEvents:UIControlEventTouchUpInside];
        
        UIButton* recPauseButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recPauseButton setTitle:@"rec pause" forState:UIControlStateNormal];
        [recPauseButton addTarget:vc action:@selector(onRecPause:) forControlEvents:UIControlEventTouchUpInside];
        
        UIButton* recStopButton = [UIButton buttonWithType:UIButtonTypeSystem];
        [recStopButton setTitle:@"rec stop" forState:UIControlStateNormal];
        [recStopButton addTarget:vc action:@selector(onRecStop:) forControlEvents:UIControlEventTouchUpInside];
        
        [self addUIRow:recStartButton
                      :recPauseButton
                      :recStopButton];
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


@end
