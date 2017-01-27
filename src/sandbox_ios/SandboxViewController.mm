
#import "SandboxViewController.h"

static const int tempBufSize = 16000;
static char tempBuf[tempBufSize];

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

static void audioWrittenCallback(const char* path, int numBytes, void* userData)
{
    SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    [vc onAudioDataWritten:[NSString stringWithUTF8String:path]
                          :numBytes];
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
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    m_recordingTargetPath = [documentsDirectory stringByAppendingPathComponent:@"test-recording.adts"];
    m_uploadTargetPath = [documentsDirectory stringByAppendingPathComponent:@"test-recording-upload.adts"];
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
        case WAVE_RECORDING_STARTED:
        {
            memset(tempBuf, 0, tempBufSize);
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
        case WAVE_RECORDING_STOPPED:
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
        case WAVE_RECORDING_PAUSED:
        {
            [self.sandboxView.recPauseButton addTarget:self
                                                action:@selector(onRecResume:)
                                      forControlEvents:UIControlEventTouchUpInside];
            [self.sandboxView.recPauseButton setTitle:@"resume" forState:UIControlStateNormal];
            break;
        }
        case WAVE_RECORDING_RESUMED:
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
    drStartRecording([m_recordingTargetPath UTF8String]);
}

-(void)onRecStop:(id)sender
{
    drStopRecording();
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
    
    WaveSettings settings;
    wave_settings_init(&settings);
    settings.encoderFormat = WAVE_ENCODER_FORMAT_AAC;
    
    drInitialize(eventCallback,
                 errorCallback,
                 audioWrittenCallback,
                 (__bridge void*)(self),
                 &settings);
}

-(void)onDeinit:(id)sender
{
    drDeinitialize();
}

-(void)onAudioDataWritten:(NSString*)path
                         :(int)numBytes
{
    NSLog(@"%d bytes of audio data written", numBytes);
    
    assert(path);
    assert([path isEqualToString:m_recordingTargetPath]);
    
    if (!m_recordingTargetFile)
    {
        m_recordingTargetFile = fopen([m_recordingTargetPath UTF8String], "r");
        assert(m_recordingTargetFile);
    }
    

    fseek(m_recordingTargetFile, m_numBytesWritten, SEEK_SET);
    int nr = fread(tempBuf, 1, numBytes, m_recordingTargetFile);
    
        
    
    assert(nr = numBytes);
    int nw = fwrite(tempBuf, 1, numBytes, m_uploadTargetFile);
    fflush(m_uploadTargetFile);
    assert(nw == numBytes);
    
    m_numBytesWritten += numBytes;
    
}

@end
