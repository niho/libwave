
#import "SandboxViewController.h"

@implementation SandboxViewController

-(id)init
{
    self = [super init];
    self.sandboxView = [[SandboxView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.view = self.sandboxView;
    
    return self;
}

@end
