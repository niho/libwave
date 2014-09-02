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
    
    typedef enum drControlEventType
    {
        DR_START_RECORDING = 0,
        DR_PAUSE_RECORDING,
        DR_STOP_RECORDING
    } drControlEventType;
    
    typedef struct drControlEvent
    {
        drControlEventType type;
    } drControlEvent;
    
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
        
        //Audio analyzers
        drLevelMeter inputLevelMeters[MAX_NUM_INPUT_CHANNELS];
        drLevelMeter outputLevelMeters[MAX_NUM_OUTPUT_CHANNELS];
        
    } drInstance;
    
    void drInstance_init(drInstance* instance, drNotificationCallback notificationCallback, void* notificationCallbackUserData);
    
    void drInstance_deinit(drInstance* instance);
    
    void drInstance_update(drInstance* instance, float timeStep);
    
    /**
     * Returns a non-zero value if called from the same thread that called drInstance_init.
     */
    int drInstance_isOnMainThread(drInstance* instance);
    
    void drInstance_getInputLevels(drInstance* instance, int channel, int logLevels, drLevels* result);
    
    void drInstance_onAudioThreadControlEvent(drInstance* instance, const drControlEvent* event);
    
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
     *
     */
    void drInstance_enqueueControlEvent(drInstance* instance, const drControlEvent* event);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_INSTANCE_H */