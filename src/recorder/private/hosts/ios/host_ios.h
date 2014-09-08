
/*! \file */

#ifndef DR_HOST_IOS_H
#define DR_HOST_IOS_H

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudioTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    
    /**
     * The callback for processing a new buffer of input samples.
     */
    OSStatus drCoreAudioInputCallback(void *inRefCon,
                                      AudioUnitRenderActionFlags *ioActionFlags,
                                      const AudioTimeStamp *inTimeStamp,
                                      UInt32 inBusNumber,
                                      UInt32 inNumberFrames,
                                      AudioBufferList *ioData);
    
    /**
     * The callback for rendering a new buffer of output samples.
     */
    OSStatus drCoreAudioOutputCallback(void *inRefCon,
                                       AudioUnitRenderActionFlags *ioActionFlags,
                                       const AudioTimeStamp *inTimeStamp,
                                       UInt32 inBusNumber,
                                       UInt32 inNumberFrames,
                                       AudioBufferList *ioData);
    
    /** Creates the singleton remote I/O unit instance. */
    void drCreateRemoteIOInstance(void);
    
    /**
     * Stops and uninitializes the remote I/O unit.
     */
    void drStopAndDeinitRemoteIO(void);
    
    /**
     * Initializes and starts the remote I/O unit.
     */
    void drInitAndStartRemoteIO(void);
    
    /**
     * Initializes the AVAudioSession.
     */
    void drInitAudioSession(void);
    
    void drSuspend(void);
    
    /**
     * Returns non-zero if successful, zero otherwise.
     */
    int drResume(void);
    
    /**
     * Called when the audio session gets interrupted.
     */
    void drAudioSessionInterruptionCallback(void *inClientData,  UInt32 inInterruptionState);
    
    /**
     * Called when audio input availability changes.
     */
    void drInputAvailableChangeCallback(void *inUserData,
                                        AudioSessionPropertyID inPropertyID,
                                        UInt32 inPropertyValueSize,
                                        const void *inPropertyValue);
    
    /**
     * Called when the audio route changes.
     */
    void drAudioRouteChangeCallback(void *inUserData,
                                    AudioSessionPropertyID inPropertyID,
                                    UInt32 inPropertyValueSize,
                                    const void *inPropertyValue);
    
    
    /**
     *
     */
    void drServerDiedCallback(void *inUserData,
                              AudioSessionPropertyID inPropertyID,
                              UInt32 inPropertyValueSize,
                              const void *inPropertyValue);
    
    /**
     * Helper function that initializes an AudioStreamBasicDescription corresponding
     * to linear PCM with a given number of channels and a given sample rate
     * @param asbd The AudioStreamBasicDescription to initialize.
     * @param numChannels The number of channels.
     * @param sampleRate The sample rate.
     */
    void drSetASBD(AudioStreamBasicDescription* asbd, int numChannels, float sampleRate);
    
    /**
     * Generates a meaningful assert if the result of an audio unit operation
     * is not successful.
     * @param result The error code to check.
     */
    void drEnsureNoAudioUnitError(OSStatus result);
    
    /**
     * Generates a meaningful assert if the result of an audio session operation
     * is not successful.
     * @param result The error code to check.
     */
    void drEnsureNoAudioSessionError(OSStatus result);
    
    /** 
     * Prints some info about the current audio session to the console.
     */
    void drDebugPrintAudioSessionInfo();
    
    /** 
     * Prints some info about the remote I/O unit to the console.
     */
    void drDebugPrintRemoteIOInfo();
    
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //DR_HOST_IOS_H
