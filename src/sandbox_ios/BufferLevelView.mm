
#import "BufferLevelView.h"

@implementation BufferLevelView

-(id)initWithFrame:(CGRect)frame
                  :(NSString*)title
{
    self = [super initWithFrame:frame];
    
    m_label = [[UILabel alloc] init];
    m_label.textAlignment = NSTextAlignmentCenter;
    m_label.text = title;
    
    m_bar = [[UIView alloc] init];
    
    [self addSubview:m_label];
    [self addSubview:m_bar];
    
    return self;
}

-(void)updateLevel:(float)level
{
    m_currentLevel = level;
    
    m_bar.frame = CGRectMake(0, 0, (int)(self.frame.size.width * m_currentLevel), self.frame.size.height);
    m_bar.backgroundColor = [UIColor colorWithHue:m_currentLevel saturation:0.8f brightness:0.8f alpha:0.8f];
    m_label.frame = self.bounds;
}

-(void)layoutSubviews
{
    [self updateLevel:m_currentLevel];
}

@end
