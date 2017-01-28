//
//  AppDelegate.m
//  sandbox_osx
//
//  Created by perarne on 9/1/14.
//
//

#import "AppDelegate.h"

#define kUpdateInterval (0.02f)

#import "wave_recorder.h"

static void eventCallback(const WaveNotification* event, void* userData)
{
    AppDelegate* ad = (__bridge AppDelegate*)userData;
}

static void errorCallback(WaveError error, void* userData)
{
    AppDelegate* ad = (__bridge AppDelegate*)userData;
}

static void audioWrittenCallback(const char* path, int numBytes, void* userData)
{
    AppDelegate* ad = (__bridge AppDelegate*)userData;
}

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    WaveSettings settings;
    wave_settings_init(&settings);
    settings.encoderFormat = WAVE_ENCODER_FORMAT_AAC;
    
    wave_init(eventCallback,
              errorCallback,
              audioWrittenCallback,
              (__bridge void*)(self),
              &settings);
    
    NSTimer* t = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                                  target:self
                                                selector:@selector(updateTick)
                                                userInfo:nil
                                                 repeats:YES];
    

}


-(void)onNotification:(const WaveNotification*)event
{
    
}

-(void)updateTick
{
    wave_update(kUpdateInterval);
}

@end
