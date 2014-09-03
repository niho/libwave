
#import <UIKit/UIKit.h>

#import "SandboxView.h"

#import "digger_recorder.h"

#define kUpdateInterval (0.025f)

@interface SandboxViewController : UIViewController
{
    int m_updateStride;
    int m_updateCounter;
}
@property (nonatomic, retain) SandboxView* sandboxView;

-(void)onNotification:(const drNotification*)event;

-(void)updateTick;

-(void)onRecStart:(id)sender;

-(void)onRecFinish:(id)sender;

-(void)onRecCancel:(id)sender;

-(void)onRecPause:(id)sender;

-(void)onRecResume:(id)sender;

-(void)onUpdateIntervalChanged:(UISegmentedControl*)c;

@end
