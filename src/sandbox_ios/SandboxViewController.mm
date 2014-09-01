
#import "SandboxViewController.h"

static void eventCallback(const drEvent* event, void* userData)
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
    switch (event->type)
    {
        case DR_DID_START_AUDIO_STREAM:
        {
            break;
        }
        default:
            break;
    }
}

-(void)updateTick
{
    drUpdate(kUpdateInterval);
}

@end
