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

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    
    drInitialize(eventCallback, (__bridge void*)(self));
    
    NSTimer* t = [NSTimer scheduledTimerWithTimeInterval:kUpdateInterval
                                                  target:self
                                                selector:@selector(updateTick)
                                                userInfo:nil
                                                 repeats:YES];
    

}

static void eventCallback(void* userData, const drEvent* event)
{
    
}


-(void)onEvent:(const drEvent*)event
{
    
}

-(void)updateTick
{
    drUpdate(kUpdateInterval);
}

@end
