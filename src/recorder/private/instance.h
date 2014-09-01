#ifndef DR_INSTANCE_H
#define DR_INSTANCE_H

/*! \file */

#include "kowalski.h"
#include "tinycthread.h"

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
        
        mtx_t outgoingEventQueueLock;
        stfMessageQueue* outgoingEventQueue;
        
    } drInstance;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_INSTANCE_H */