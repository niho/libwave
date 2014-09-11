#ifndef DR_LEVEL_ADVISOR_H
#define DR_LEVEL_ADVISOR_H

/*! \file */

#include "level_meter.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    typedef struct drInstance drInstance;
    
    /**
     *
     */
    typedef struct drLevelAdvisor
    {
        drInstance* instance;
        
        /** Used to generate a level envelope. */
        drLevelMeter levelMeter;
        
        int loudWarningDelayInFrames;
        int loudWarningTimer;
        float loudThreshold;
        
        int quietWarningDelayInFrames;
        int quietWarningTimer;
        float quietThreshold;
        
    } drLevelAdvisor;
    
    void drLevelAdvisor_init(drLevelAdvisor* advisor,
                             drInstance* instance,
                             int channel,
                             float fs);
    
    /**
     * Buffer processing callback.
     */
    void drLevelAdvisor_processBuffer(void* advisorPtr, const float* inBuffer, int numChannels, int numFrames);
    
    /**
     * Cleanup callback.
     */
    void drLevelAdvisor_deinit(void* advisorPtr);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_LEVEL_ADVISOR_H */