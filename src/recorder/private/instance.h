#ifndef WAVE_INSTANCE_H
#define WAVE_INSTANCE_H

/*! \file */

#include <tinycthread.h>
#include "wave_recorder.h"
#include "level_meter.h"
#include "analyzer.h"
#include "lock_free_fifo.h"
#include "wave_settings.h"
#include "encoder.h"
#include "level_advisor.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    #define MAX_NUM_ANALYZER_SLOTS 16
    #define MAX_NUM_INPUT_CHANNELS 1
    #define MAX_NUM_OUTPUT_CHANNELS 2
    
    #define MAX_STREAM_CHUNK_SIZE 1024
    
    typedef struct WaveStreamingSession
    {
        WaveEncoder encoder;
        int numRecordedFrames;
    } WaveStreamingSession;
    
    typedef struct WaveAudioChunk
    {
        int numChannels;
        int numFrames;
        float samples[MAX_STREAM_CHUNK_SIZE];
        int lastChunk;
    } WaveAudioChunk;
    
    /**
     * Valid control event types
     */
    typedef enum WaveControlEventType
    {
        WAVE_START_STREAMING = 0,
        WAVE_PAUSE_STREAMING,
        WAVE_RESUME_STREAMING,
        WAVE_STOP_STREAMING,
    } WaveControlEventType;
    
    /**
     * A control event, i.e an event sent form the 
     * main thread to the audio thread.
     */
    typedef struct WaveControlEvent
    {
        WaveControlEventType type;
    } WaveControlEvent;
    
    /**
     * Possible recorder instance states.
     */
    typedef enum WaveAudioThreadState
    {
        WAVE_STATE_IDLE = 0,
        WAVE_STATE_STREAMING,
        WAVE_STATE_STREAMING_PAUSED
    } WaveState;
    
    /**
     * An analyzer slot.
     */
    typedef struct WaveAnalyzerSlot
    {
        void* analyzerData;
        WaveAnalyzerAudioCallback audioCallback;
        WaveAnalyzerDeinitCallback deinitCallback;
    } WaveAnalyzerSlot;
    
    /**
     * A recorder instance.
     */
    typedef struct WaveInstance
    {
        WaveSettings settings;
        
        WaveState stateAudioThread;
        WaveState stateMainThread;
        
        WaveAnalyzerSlot inputAnalyzerSlots[MAX_NUM_ANALYZER_SLOTS];
        
        WaveDevInfo devInfo;
        
        int firstSampleHasPlayed;
        int stopStreamingRequested;
        
        //used to verify that functions are being called from the right threads
        thrd_t mainThread;
        
        WaveLockFreeFIFO notificationFIFO;
        WaveLockFreeFIFO controlEventFIFO;
        WaveLockFreeFIFO errorFIFO;
        WaveLockFreeFIFO realTimeDataFifo;
        
        WaveAudioStreamCallback audioStreamCallback;
        WaveNotificationCallback notificationCallback;
        WaveErrorCallback errorCallback;
        void* callbackUserData;
        
        //Level meters, only accessed from the audio thread
        WaveLevelMeter inputLevelMeters[MAX_NUM_INPUT_CHANNELS];
        
        WaveLevelAdvisor levelAdvisor;
        
        //Measured levels, copied from audio to main via shared memory
        WaveRealtimeInfo realtimeInfo;
        
        WaveLockFreeFIFO inputAudioDataQueue;
        
        WaveStreamingSession streamingSession;
        
        //actual audio stream parameters
        int sampleRate;
        int numInputChannels;
        int numOutputChannels;
        
        //
        int isInputDisabled;
    } WaveInstance;

    /**
     *
     */
    WaveError wave_instance_init(WaveInstance* instance,
                            WaveNotificationCallback notificationCallback,
                            WaveErrorCallback errorCallback,
                            WaveAudioStreamCallback audioStreamCallback,
                            void* callbackUserData,
                            WaveSettings* settings);
    
    /** Implemented for each host. */
    WaveError wave_instance_host_specific_init(WaveInstance* instance);
    
    /**
     *
     */
    WaveError wave_instance_deinit(WaveInstance* instance);
    
    /** Implemented for each host.*/
    WaveError wave_instance_host_specific_deinit(WaveInstance* instance);
    
    /**
     *
     */
    void wave_instance_update(WaveInstance* instance, float timeStep);
    
    /**
     *
     */
    void wave_instance_audio_input_callback(WaveInstance* in, const float* inBuffer, int numChannels, int numFrames);
    
    /**
     *
     */
    void wave_instance_audio_output_callback(WaveInstance* in, float* inBuffer, int numChannels, int numFrames);
    
    /**
     * Returns a non-zero value if called from the same thread that called wave_instance_init.
     */
    int wave_instance_is_on_main_thread(WaveInstance* instance);
    
    /**
     *
     */
    void wave_instance_get_realtime_info(WaveInstance* instance, int channel, int logLevels, WaveRealtimeInfo* result);
    
    /**
     * Invoked when a control event reaches the audio thread. Invoked on the audio thread.
     */
    void wave_instance_on_audio_thread_control_event(WaveInstance* instance, const WaveControlEvent* event);
    
    /**
     * Invoked when a notification reaches the main thread. Invoked on the main thread.
     */
    void wave_instance_on_main_thread_notification(WaveInstance* instance, const WaveNotification* notification);
    
    /**
     *
     */
    void wave_instance_on_main_thread_error(WaveInstance* instance, WaveError error);
    
    /**
     * Returns 0 on success, or non-zero if there is no free analyzer slot.
     */
    int wave_instance_add_input_analyzer(WaveInstance* instance,
                                    void* analyzerData,
                                    WaveAnalyzerAudioCallback audioCallback,
                                    WaveAnalyzerDeinitCallback deinitCallback);
    
    /**
     *
     */
    void wave_instance_request_start_streaming(WaveInstance* instance);
    
    /**
     *
     */
    void wave_nstance_initiate_streaming(WaveInstance* instance);
    
    /**
     *
     */
    void wave_instance_stop_streaming(WaveInstance* instance);
    
    /**
     *
     */
    void wave_instance_invoke_error_callback(WaveInstance* instance, WaveError errorCode);
    
    /**
     *
     */
    void wave_instance_invoke_notification_callback(WaveInstance* instance, const WaveNotification* notification);
    
    /**
     * Must be called <strong>only from the audio thread</strong>!
     */
    void wave_instance_enqueue_error(WaveInstance* instance, WaveError error);
    
    /**
     * Must be called <strong>only from the audio thread</strong>!
     */
    void wave_instance_enqueue_notification(WaveInstance* instance, const WaveNotification* notification);
    
    /**
     * Must be called <strong>only from the audio thread</strong>!
     */
    void wave_instance_enqueue_notification_of_type(WaveInstance* instance, WaveNotificationType type);
    
    /**
     * Must be called <strong>only from the main thread</strong>!
     */
    void wave_instance_enqueue_control_event(WaveInstance* instance, const WaveControlEvent* event);
    
    /**
     * Must be called <strong>only from the main thread</strong>!
     */
    void wave_instance_enqueue_control_event_of_type(WaveInstance* instance, WaveControlEventType type);
    
    /**
     *
     */
    void wave_instance_get_dev_info(WaveInstance* instance, WaveDevInfo* devInfo);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_INSTANCE_H */
