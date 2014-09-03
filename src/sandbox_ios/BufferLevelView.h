
#import <UIKit/UIKit.h>

@interface BufferLevelView : UIView
{
    float m_currentLevel;
    UILabel* m_label;
    UIView* m_bar;
}

-(id)initWithFrame:(CGRect)frame
                  :(NSString*)title;

-(void)updateLevel:(float)level;

@end
