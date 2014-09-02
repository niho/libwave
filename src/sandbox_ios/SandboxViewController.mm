
#import "SandboxViewController.h"

static void eventCallback(const drNotification* event, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onNotification:event];
}

@implementation SandboxViewController

-(id)init
{
    self = [super init];
    self.sandboxView = [[SandboxView alloc] initWithFrame:[UIScreen mainScreen].bounds
                                                         :self];
    self.view = self.sandboxView;
    
    drInitialize(eventCallback, (__bridge void*)(self));
    
    NSTimer* t = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                                  target:self
                                                selector:@selector(updateTick)
                                                userInfo:nil
                                                 repeats:YES];
    
    return self;
}

-(void)onNotification:(const drNotification*)notification
{
    self.sandboxView.latestNotificationLabel.text = [NSString stringWithUTF8String:drNotificationTypeToString(notification->type)];
    
    switch (notification->type)
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

-(void)onRecStart:(id)sender
{
    drStartRecording();
}

-(void)onRecFinish:(id)sender
{
    drFinishRecording();
}

-(void)onRecCancel:(id)sender
{
    drCancelRecording();
}

-(void)onRecPause:(id)sender
{
    drPauseRecording();
}

-(void)onRecResume:(id)sender
{
    drResumeRecording();
}

@end
