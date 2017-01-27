
#import <UIKit/UIKit.h>

#import "SandboxView.h"

#import "wave_recorder.h"

#define kUpdateInterval (0.025f)

@interface SandboxViewController : UIViewController
{
    int m_updateStride;
    int m_updateCounter;
    int m_numBytesWritten;
    NSString* m_recordingTargetPath;
    NSString* m_uploadTargetPath;
    FILE* m_recordingTargetFile;
    FILE* m_uploadTargetFile;
}

@property (nonatomic, retain) SandboxView* sandboxView;

-(void)onError:(drError)error;

-(void)onNotification:(const drNotification*)event;

-(void)updateTick;

-(void)onRecStart:(id)sender;

-(void)onRecStop:(id)sender;

-(void)onRecCancel:(id)sender;

-(void)onRecPause:(id)sender;

-(void)onRecResume:(id)sender;

-(void)onUpdateIntervalChanged:(UISegmentedControl*)c;

-(void)onInit:(id)sender;

-(void)onDeinit:(id)sender;

-(void)onAudioDataWritten:(NSString*)path:(int)numBytes;

@end
