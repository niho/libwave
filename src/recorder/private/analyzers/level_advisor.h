#ifndef WAVE_LEVEL_ADVISOR_H
#define WAVE_LEVEL_ADVISOR_H

/*! \file */

#include "level_meter.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    typedef struct WaveInstance WaveInstance;
    
    /**
     *
     */
    typedef struct WaveLevelAdvisor
    {
        WaveInstance* instance;
        
        /** Used to generate a level envelope. */
        WaveLevelMeter levelMeter;
        
        int loudWarningDelayInFrames;
        int loudWarningTimer;
        float loudThreshold;
        
        int quietWarningDelayInFrames;
        int quietWarningTimer;
        float quietThreshold;
        
    } WaveLevelAdvisor;
    
    void wave_level_advisor_init(WaveLevelAdvisor* advisor,
                             WaveInstance* instance,
                             int channel,
                             float fs);
    
    /**
     * Buffer processing callback.
     */
    void wave_level_advisor_process_buffer(void* advisorPtr, const float* inBuffer, int numChannels, int numFrames);
    
    /**
     * Cleanup callback.
     */
    void wave_level_advisor_deinit(void* advisorPtr);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_LEVEL_ADVISOR_H */
