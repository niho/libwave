
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
            [self.sandboxView.recToggleButton addTarget:self
                                                action:@selector(onRecStart:)
                                      forControlEvents:UIControlEventTouchUpInside];
            break;
        }
        case DR_RECORDING_STARTED:
        {
            [self.sandboxView.recToggleButton addTarget:self
                                                 action:@selector(onRecFinish:)
                                       forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecPause:)
                                      forControlEvents:UIControlEventTouchUpInside];
            self.sandboxView.recPauseButton.enabled = YES;
            self.sandboxView.recCancelButton.enabled = YES;
            [self.sandboxView.recToggleButton setTitle:@"finish" forState:UIControlStateNormal];
            self.sandboxView.recToggleButton.backgroundColor = [UIColor redColor];
            break;
        }
        case DR_RECORDING_FINISHED:
        {
            self.sandboxView.recPauseButton.enabled = NO;
            self.sandboxView.recCancelButton.enabled = NO;
            [self.sandboxView.recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
            [self.sandboxView.recToggleButton setTitle:@"start" forState:UIControlStateNormal];
            self.sandboxView.recToggleButton.backgroundColor = [UIColor clearColor];
            [self.sandboxView.recToggleButton addTarget:self
                                                 action:@selector(onRecStart:)
                                       forControlEvents:UIControlEventTouchUpInside];
            break;
        }
        case DR_RECORDING_PAUSED:
        {
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecResume:)
                                      forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton setTitle:@"resume" forState:UIControlStateNormal];
            break;
        }
        case DR_RECORDING_RESUMED:
        {
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecPause:)
                                      forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
            break;
        }
        case DR_RECORDING_CANCELED:
        {
            self.sandboxView.recPauseButton.enabled = NO;
            self.sandboxView.recCancelButton.enabled = NO;
            [self.sandboxView.recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
            [self.sandboxView.recToggleButton setTitle:@"start" forState:UIControlStateNormal];
            self.sandboxView.recToggleButton.backgroundColor = [UIColor clearColor];
            [self.sandboxView.recToggleButton addTarget:self
                                                 action:@selector(onRecStart:)
                                       forControlEvents:UIControlEventTouchUpInside];
            break;
        }
        default:
        {
            break;
        }
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
