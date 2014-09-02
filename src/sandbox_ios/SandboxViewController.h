
#import <UIKit/UIKit.h>

#import "SandboxView.h"

#import "digger_recorder.h"

#define kUpdateInterval (0.025f)

@interface SandboxViewController : UIViewController

@property (nonatomic, retain) SandboxView* sandboxView;

-(void)onEvent:(const drNotification*)event;

-(void)updateTick;

@end
