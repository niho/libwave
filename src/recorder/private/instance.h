#ifndef DR_INSTANCE_H
#define DR_INSTANCE_H

/*! \file */

#include "kowalski.h"
#include "tinycthread.h"
#include "digger_recorder.h"
#include "level_meter.h"
#include "analyzer.h"
#include "lock_free_fifo.h"
#include "recording_session.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    #define MAX_NUM_ANALYZER_SLOTS 16
    #define MAX_NUM_INPUT_CHANNELS 1
    #define MAX_NUM_OUTPUT_CHANNELS 2
    
    #define kControlEventFIFOCapacity (50)
    #define kNotificationFIFOCapacity (50)
    #define kRecordFIFOCapacity (50)
    #define MAX_RECORDED_CHUNK_SIZE 1024
    
    typedef struct drRecordedChunk
    {
        int numChannels;
        int numFrames;
        float samples[MAX_RECORDED_CHUNK_SIZE];
        int lastChunk;
    } drRecordedChunk;
    
    /**
     * Valid control event types
     */
    typedef enum drControlEventType
    {
        DR_START_RECORDING = 0,
        DR_PAUSE_RECORDING,
        DR_RESUME_RECORDING,
        DR_CANCEL_RECORDING,
        DR_FINISH_RECORDING,
    } drControlEventType;
    
    /**
     * A control event, i.e an event sent form the 
     * main thread to the audio thread.
     */
    typedef struct drControlEvent
    {
        drControlEventType type;
    } drControlEvent;
    
    /**
     * Possible recorder instance states.
     */
    typedef enum drAudioThreadState
    {
        DR_STATE_IDLE = 0,
        DR_STATE_RECORDING,
        DR_STATE_RECORDING_PAUSED
        
    } drState;
    
    /**
     * An analyzer slot.
     */
    typedef struct drAnalyzerSlot
    {
        void* analyzerData;
        drAnalyzerAudioCallback audioCallback;
        drAnalyzerDeinitCallback deinitCallback;
    } drAnalyzerSlot;
    
    /**
     * A Digger recorder instance.
     */
    typedef struct drInstance
    {
        drState stateAudioThread;
        drState stateMainThread;
        
        drAnalyzerSlot inputAnalyzerSlots[MAX_NUM_ANALYZER_SLOTS];
        
        drDevInfo devInfo;
        
        float sampleRate;
        kwlDSPUnitHandle inputDSPUnit;
        kwlDSPUnitHandle outputDSPUnit;
        int firstSampleHasPlayed;
        int finishRecordingRequested;
        
        //used to verify that functions are being called from the right threads
        thrd_t mainThread;
        
        mtx_t communicationQueueLock;
        drLockFreeFIFO notificationFIFO;
        drLockFreeFIFO controlEventFIFO;
        
        drNotificationCallback notificationCallback;
        void* notificationCallbackData;
        
        //Level meters, only accessed from the audio thread
        drLevelMeter inputLevelMeters[MAX_NUM_INPUT_CHANNELS];
        
        //Measured levels, copied from audio to main via shared memory
        drLevels inputLevelsAudio[MAX_NUM_INPUT_CHANNELS];
        drLevels inputLevelsShared[MAX_NUM_INPUT_CHANNELS];
        drLevels inputLevelsMain[MAX_NUM_INPUT_CHANNELS];
        
        drLockFreeFIFO inputAudioDataQueue;
        
        drRecordingSession recordingSession;
        
    } drInstance;

    /**
     *
     */
    void drInstance_init(drInstance* instance,
                         drNotificationCallback notificationCallback,
                         void* notificationCallbackUserData);
    
    /**
     *
     */
    void drInstance_deinit(drInstance* instance);
    
    /**
     *
     */
    void drInstance_update(drInstance* instance, float timeStep);
        
    /**
     * Returns a non-zero value if called from the same thread that called drInstance_init.
     */
    int drInstance_isOnMainThread(drInstance* instance);
    
    /**
     *
     */
    void drInstance_getInputLevels(drInstance* instance, int channel, int logLevels, drLevels* result);
    
    /**
     * Invoked when a control event reaches the audio thread. Invoked on the audio thread.
     */
    void drInstance_onAudioThreadControlEvent(drInstance* instance, const drControlEvent* event);
    
    /**
     * Invoked when a notification reaches the main thread. Invoked on the main thread.
     */
    void drInstance_onMainThreadNotification(drInstance* instance, const drNotification* notification);
    
    /**
     * Returns 0 on success, or non-zero if there is no free analyzer slot.
     */
    int drInstance_addInputAnalyzer(drInstance* instance,
                                    void* analyzerData,
                                    drAnalyzerAudioCallback audioCallback,
                                    drAnalyzerDeinitCallback deinitCallback);
    
    /**
     *
     */
    void drInstance_initiateRecording(drInstance* instance);
    
    /**
     *
     */
    void drInstance_finishRecording(drInstance* instance);
    
    /**
     *
     */
    void drInstance_cancelRecording(drInstance* instance);
    
    /**
     * Must be called <strong>only from the audio thread</strong>!
     */
    void drInstance_enqueueNotification(drInstance* instance, const drNotification* notification);
    
    /**
     * Must be called <strong>only from the audio thread</strong>!
     */
    void drInstance_enqueueNotificationOfType(drInstance* instance, drNotificationType type);
    
    /**
     * Must be called <strong>only from the main thread</strong>!
     */
    void drInstance_enqueueControlEvent(drInstance* instance, const drControlEvent* event);
    
    /**
     * Must be called <strong>only from the main thread</strong>!
     */
    void drInstance_enqueueControlEventOfType(drInstance* instance, drControlEventType type);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_INSTANCE_H */