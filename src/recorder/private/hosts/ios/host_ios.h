
/*! \file */

#ifndef WAVE_HOST_IOS_H
#define WAVE_HOST_IOS_H

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
    OSStatus wave_core_audio_input_callback(void *inRefCon,
                                      AudioUnitRenderActionFlags *ioActionFlags,
                                      const AudioTimeStamp *inTimeStamp,
                                      UInt32 inBusNumber,
                                      UInt32 inNumberFrames,
                                      AudioBufferList *ioData);
    
    /**
     * The callback for rendering a new buffer of output samples.
     */
    OSStatus wave_core_audio_output_callback(void *inRefCon,
                                       AudioUnitRenderActionFlags *ioActionFlags,
                                       const AudioTimeStamp *inTimeStamp,
                                       UInt32 inBusNumber,
                                       UInt32 inNumberFrames,
                                       AudioBufferList *ioData);
    
    /** Creates the singleton remote I/O unit instance. */
    void wave_create_remote_io_instance(void);
    
    /**
     * Stops and uninitializes the remote I/O unit.
     */
    void wave_stop_and_deinit_remote_io(void);
    
    /**
     * Initializes and starts the remote I/O unit.
     */
    void wave_init_and_start_remote_io(void);
    
    /**
     * Initializes the AVAudioSession.
     */
    void wave_init_audio_session(void);
    
    void wave_suspend(void);
    
    /**
     * Returns non-zero if successful, zero otherwise.
     */
    int wave_resume(void);
    
    /**
     * Called when the audio session gets interrupted.
     */
    void wave_audio_session_interruption_callback(void *inClientData,  UInt32 inInterruptionState);
    
    /**
     * Called when audio input availability changes.
     */
    void wave_input_available_change_callback(void *inUserData,
                                        AudioSessionPropertyID inPropertyID,
                                        UInt32 inPropertyValueSize,
                                        const void *inPropertyValue);
    
    /**
     * Called when the audio route changes.
     */
    void wave_audio_route_change_callback(void *inUserData,
                                    AudioSessionPropertyID inPropertyID,
                                    UInt32 inPropertyValueSize,
                                    const void *inPropertyValue);
    
    
    /**
     *
     */
    void wave_server_died_callback(void *inUserData,
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
    void wave_set_asbd(AudioStreamBasicDescription* asbd, int numChannels, float sampleRate);
    
    /**
     * Generates a meaningful assert if the result of an audio unit operation
     * is not successful.
     * @param result The error code to check.
     */
    void wave_ensure_no_audio_unit_error(OSStatus result);
    
    /**
     * Generates a meaningful assert if the result of an audio session operation
     * is not successful.
     * @param result The error code to check.
     */
    void wave_ensure_no_audio_session_error(OSStatus result);
    
    /** 
     * Prints some info about the current audio session to the console.
     */
    void wave_debug_print_audio_session_info();
    
    /** 
     * Prints some info about the remote I/O unit to the console.
     */
    void wave_debug_print_remote_io_info();
    
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //WAVE_HOST_IOS_H
