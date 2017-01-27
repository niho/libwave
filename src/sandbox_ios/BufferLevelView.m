
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
    
    [self addSubview:m_bar];
    [self addSubview:m_label];
    
    return self;
}

-(void)updateLevel:(float)level
{
    m_currentLevel = level;
    
    m_bar.frame = CGRectMake(0, 0, (int)(self.frame.size.width * m_currentLevel), self.frame.size.height);
    m_bar.backgroundColor = [UIColor colorWithRed:2 * m_currentLevel green:2.0f - 2 * m_currentLevel blue:0.0f alpha:0.5f];
    m_label.frame = self.bounds;
}

-(void)layoutSubviews
{
    [self updateLevel:m_currentLevel];
}

@end
