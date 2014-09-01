
#import "SandboxViewController.h"

static void eventCallback(void* userData, const drEvent* event)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onEvent:event];
}

@implementation SandboxViewController

-(id)init
{
    self = [super init];
    self.sandboxView = [[SandboxView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.view = self.sandboxView;
    
    drInitialize(eventCallback, (__bridge void*)(self));
    
    NSTimer* t = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                                  target:self
                                                selector:@selector(updateTick)
                                                userInfo:nil
                                                 repeats:YES];
    
    return self;
}

-(void)onEvent:(const drEvent*)event
{
    
}

-(void)updateTick
{
    drUpdate(kUpdateInterval);
}

@end
