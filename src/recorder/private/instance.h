#ifndef DR_INSTANCE_H
#define DR_INSTANCE_H

/*! \file */

#include "kowalski.h"
#include "tinycthread.h"
#include "messagequeue.h"
#include "digger_recorder.h"
#include "level_meter.h"
#include "analyzer.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    #define MAX_NUM_ANALYZER_SLOTS 16
    #define MAX_NUM_INPUT_CHANNELS 1
    #define MAX_NUM_OUTPUT_CHANNELS 2
    
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
    typedef enum drState
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
        drState state;
        drAnalyzerSlot inputAnalyzerSlots[MAX_NUM_ANALYZER_SLOTS];
        
        float sampleRate;
        kwlDSPUnitHandle inputDSPUnit;
        kwlDSPUnitHandle outputDSPUnit;
        int firstSampleHasPlayed;
        
        //used to verify that functions are being called from the right threads
        thrd_t mainThread;
        
        mtx_t communicationQueueLock;
        drMessageQueue* outgoingEventQueueShared;
        drMessageQueue* outgoingEventQueueMain;
        drMessageQueue* outgoingEventQueueAudio;
        
        drMessageQueue* controlEventQueueShared;
        drMessageQueue* controlEventQueueMain;
        drMessageQueue* controlEventQueueAudio;
        
        drNotificationCallback notificationCallback;
        void* notificationCallbackData;
        
        //Level meters, only accessed from the audio thread
        drLevelMeter inputLevelMeters[MAX_NUM_INPUT_CHANNELS];
        
        //Measured levels
        drLevels inputLevelsAudio[MAX_NUM_INPUT_CHANNELS];
        drLevels inputLevelsShared[MAX_NUM_INPUT_CHANNELS];
        drLevels inputLevelsMain[MAX_NUM_INPUT_CHANNELS];
        
    } drInstance;

    /**
     *
     */
    void drInstance_init(drInstance* instance, drNotificationCallback notificationCallback, void* notificationCallbackUserData);
    
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