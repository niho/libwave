//
//  AppDelegate.m
//  sandbox_osx
//
//  Created by perarne on 9/1/14.
//
//

#import "AppDelegate.h"

#define kUpdateInterval (0.02f)

#import "digger_recorder.h"

static void eventCallback(const drNotification* event, void* userData)
{
    AppDelegate* ad = (__bridge AppDelegate*)userData;
}

static void errorCallback(drError error, void* userData)
{
    AppDelegate* ad = (__bridge AppDelegate*)userData;
}

static const char* writableFilePathCallback(void* userData)
{
    //SandboxViewController* vc = (__bridge SandboxViewController*)userData;
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    NSString* fileName = [NSString stringWithFormat:@"audio-recording-%d", arc4random()];
    return [[documentsDirectory stringByAppendingPathComponent:fileName] UTF8String];
}

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString* settingsFilePath = [documentsDirectory stringByAppendingPathComponent:@"drsettings.json"];
    
    drInitialize(eventCallback,
                 errorCallback,
                 writableFilePathCallback,
                 (__bridge void*)(self),
                 [settingsFilePath UTF8String]);
    
    NSTimer* t = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                                  target:self
                                                selector:@selector(updateTick)
                                                userInfo:nil
                                                 repeats:YES];
    

}


-(void)onNotification:(const drNotification*)event
{
    
}

-(void)updateTick
{
    drUpdate(kUpdateInterval);
}

@end
