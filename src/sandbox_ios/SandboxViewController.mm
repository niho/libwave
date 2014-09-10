
#import "SandboxViewController.h"

static void eventCallback(const drNotification* event, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onNotification:event];
}

static void errorCallback(drError error, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onError:error];
}

static const char* writableFilePathCallback(void* userData)
{
    //SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    NSString* fileName = [NSString stringWithFormat:@"audio-recording-%d", arc4random()];
    return [[documentsDirectory stringByAppendingPathComponent:fileName] UTF8String];
}

static void audioWrittenCallback(const char* path, int numBytes, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    printf("wrote %d bytes to %s\n", numBytes, path);
}


@implementation SandboxViewController

-(id)init
{
    self = [super init];
    m_updateStride = 1;
    self.sandboxView = [[SandboxView alloc] initWithFrame:[UIScreen mainScreen].bounds
                                                         :self];
    self.view = self.sandboxView;
    
    [self onInit:nil];
    
    NSTimer* t = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                                  target:self
                                                selector:@selector(updateTick)
                                                userInfo:nil
                                                 repeats:YES];
    
    return self;
}

-(void)onError:(drError)error
{
    self.sandboxView.latestErrorLabel.text = [NSString stringWithUTF8String:drErrorToString(error)];
}

-(void)onNotification:(const drNotification*)notification
{
    self.sandboxView.latestNotificationLabel.text = [NSString stringWithUTF8String:drNotificationTypeToString(notification->type)];
    
    switch (notification->type)
    {
        case DR_DID_INITIALIZE:
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
    if (m_updateCounter == 0)
    {
        drUpdate(kUpdateInterval);
        drRealtimeInfo il;
        drGetRealtimeInfo(0, 1, &il);
        
        drDevInfo di;
        drGetDevInfo(&di);
        
        float a = 0.2f;
        float dur = 2.0f;
        if (di.errorFIFOUnderrun)
        {
            self.sandboxView.errorFIFOUnderrunLabel.alpha = 1.0f;
            [UIView animateWithDuration:dur animations:^(void) {
                self.sandboxView.errorFIFOUnderrunLabel.alpha = a;
            }];
        }
        if (di.recordFIFOUnderrun)
        {
            self.sandboxView.recordingFIFOUnderrunLabel.alpha = 1.0f;
            [UIView animateWithDuration:dur animations:^(void) {
                self.sandboxView.recordingFIFOUnderrunLabel.alpha = a;
            }];
        }
        if (di.controlEventFIFOUnderrun)
        {
            self.sandboxView.controlFIFOUnderrunLabel.alpha = 1.0f;
            [UIView animateWithDuration:dur animations:^(void) {
                self.sandboxView.controlFIFOUnderrunLabel.alpha = a;
            }];
        }
        if (di.notificationFIFOUnderrun)
        {
            self.sandboxView.notificationFIFOUnderrunLabel.alpha = 1.0f;
            [UIView animateWithDuration:dur animations:^(void) {
                self.sandboxView.notificationFIFOUnderrunLabel.alpha = a;
            }];
        }
        
        [self.sandboxView.levelMeterView updateInfo:&il];
        [self.sandboxView.devInfoView updateInfo:&di];
    }
    
    m_updateCounter = (m_updateCounter + 1) % m_updateStride;
}

-(void)onRecStart:(id)sender
{
    drStartRecording(writableFilePathCallback(NULL));
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

-(void)onUpdateIntervalChanged:(UISegmentedControl*)c
{
    int skip[5] = {1, 2, 4, 10, 10000000};
    m_updateStride = skip[c.selectedSegmentIndex];
    m_updateCounter = 0;
}

-(void)onInit:(id)sender
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString* settingsFilePath = [documentsDirectory stringByAppendingPathComponent:@"drsettings.json"];
    
    drInitialize(eventCallback,
                 errorCallback,
                 audioWrittenCallback,
                 (__bridge void*)(self),
                 [settingsFilePath UTF8String],
                 NULL);
}

-(void)onDeinit:(id)sender
{
    drDeinitialize();
}

@end
