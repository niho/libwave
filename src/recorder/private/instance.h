#ifndef DR_INSTANCE_H
#define DR_INSTANCE_H

/*! \file */

#include "kowalski.h"
#include "tinycthread.h"
#include "messagequeue.h"
#include "digger_recorder.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    /**
     * A Digger recorder instance.
     */
    typedef struct drInstance
    {
        float sampleRate;
        kwlDSPUnitHandle inputDSPUnit;
        kwlDSPUnitHandle outputDSPUnit;
        float inputPeakValue;
        
        mtx_t sharedEventQueueLock;
        stfMessageQueue* outgoingEventQueueShared;
        stfMessageQueue* outgoingEventQueueMain;
        stfMessageQueue* outgoingEventQueueAudio;
        
    } drInstance;
    
    void drInstance_init(drInstance* instance);
    
    void drInstance_deinit(drInstance* instance);
    
    void drInstance_enqueuEventFromAudioToMainThread(drInstance* instance, const drEvent* event);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_INSTANCE_H */