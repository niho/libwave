#import "SandboxViewController.h"

static void eventCallback(const WaveNotification* event, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onNotification:event];
}

static void errorCallback(WaveError error, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onError:error];
}

static void audioWrittenCallback(const void* buffer, size_t numBytes, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onAudioDataWritten:buffer numBytes:numBytes];
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
    
    [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                     target:self
                                   selector:@selector(updateTick)
                                   userInfo:nil
                                    repeats:YES];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    m_recordingTargetPath = [documentsDirectory stringByAppendingPathComponent:@"test-recording.opus"];
    m_uploadTargetPath = [documentsDirectory stringByAppendingPathComponent:@"test-recording-upload.opus"];
    NSLog(@"%@", m_recordingTargetPath);
    NSLog(@"%@", m_uploadTargetPath);
    
    return self;
}

-(void)onError:(WaveError)error
{
    self.sandboxView.latestErrorLabel.text = [NSString stringWithUTF8String:wave_error_str(error)];
}

-(void)onNotification:(const WaveNotification*)notification
{
    self.sandboxView.latestNotificationLabel.alpha = 0.6f;
    self.sandboxView.latestNotificationLabel.text = [NSString stringWithUTF8String:wave_notification_type_str(notification->type)];
    [UIView animateWithDuration:3.0f animations:^(void) {
        self.sandboxView.latestNotificationLabel.alpha = 0.1f;
    }];
    
    switch (notification->type)
    {
        case WAVE_DID_INITIALIZE:
        {
            [self.sandboxView.recToggleButton addTarget:self
                                                action:@selector(onRecStart:)
                                      forControlEvents:UIControlEventTouchUpInside];
            break;
        }
        case WAVE_STREAMING_STARTED:
        {
            m_uploadTargetFile = fopen([m_uploadTargetPath UTF8String], "wb");
            assert(m_uploadTargetFile);
            
            
            
            [self.sandboxView.recToggleButton addTarget:self
                                                 action:@selector(onRecStop:)
                                       forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecPause:)
                                      forControlEvents:UIControlEventTouchUpInside];
            self.sandboxView.recPauseButton.enabled = YES;
            [self.sandboxView.recToggleButton setTitle:@"stop" forState:UIControlStateNormal];
            self.sandboxView.recToggleButton.backgroundColor = [UIColor redColor];
            break;
        }
        case WAVE_STREAMING_STOPPED:
        {
            fclose(m_uploadTargetFile);
            m_uploadTargetFile = NULL;
            fclose(m_recordingTargetFile);
            m_recordingTargetFile = NULL;
            
            self.sandboxView.recPauseButton.enabled = NO;
            [self.sandboxView.recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
            [self.sandboxView.recToggleButton setTitle:@"start" forState:UIControlStateNormal];
            self.sandboxView.recToggleButton.backgroundColor = [UIColor clearColor];
            [self.sandboxView.recToggleButton addTarget:self
                                                 action:@selector(onRecStart:)
                                       forControlEvents:UIControlEventTouchUpInside];
            break;
        }
        case WAVE_STREAMING_PAUSED:
        {
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecResume:)
                                      forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton setTitle:@"resume" forState:UIControlStateNormal];
            break;
        }
        case WAVE_STREAMING_RESUMED:
        {
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecPause:)
                                      forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton setTitle:@"pause" forState:UIControlStateNormal];
            break;
        }
        case WAVE_LEVEL_LOW_WARNING:
        {
            break;
        }
        case WAVE_LEVEL_HIGH_WARNING:
        {
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
        wave_update(kUpdateInterval);
        WaveRealtimeInfo il;
        wave_get_realtime_info(0, 1, &il);
        
        WaveDevInfo di;
        wave_get_dev_info(&di);
        
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
    wave_start_streaming();
}

-(void)onRecStop:(id)sender
{
    wave_stop_streaming();
}

-(void)onRecPause:(id)sender
{
    wave_pause_streaming();
}

-(void)onRecResume:(id)sender
{
    wave_resume_streaming();
}

-(void)onUpdateIntervalChanged:(UISegmentedControl*)c
{
    int skip[5] = {1, 2, 4, 10, 10000000};
    m_updateStride = skip[c.selectedSegmentIndex];
    m_updateCounter = 0;
}

-(void)onInit:(id)sender
{
    //NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    //NSString *documentsDirectory = [paths objectAtIndex:0];
    
    WaveSettings settings;
    wave_settings_init(&settings);
    settings.encoderFormat = WAVE_ENCODER_FORMAT_OPUS;
    
    wave_init(eventCallback,
              errorCallback,
              audioWrittenCallback,
              (__bridge void*)(self),
              &settings);
}

-(void)onDeinit:(id)sender
{
    wave_deinit();
}

-(void)onAudioDataWritten:(const void*)buffer
                 numBytes:(size_t)numBytes
{
    NSLog(@"%lu bytes of audio data written", numBytes);
    
    assert(buffer);
    assert(numBytes > 0);
    
    size_t nw = fwrite(buffer, 1, numBytes, m_uploadTargetFile);
    fflush(m_uploadTargetFile);
    assert(nw == numBytes);
    
    m_numBytesWritten += numBytes;
    
}

@end
