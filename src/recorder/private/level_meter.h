#ifndef DR_LEVEL_METER_H
#define DR_LEVEL_METER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
      
    typedef struct drLevelMeter
    {
        /** The index of the channel to process. */
        int channel;
        
        float peakLog;
        float peakLin;
        float peakEnvelopeLog;
        float peakEnvelopeLin;
        
        float peakEnvelopeFeedback;
        
        float rmsLog;
        float rmsLin;
        float rmsEnvelopeLog;
        float rmsEnvelopeLin;
        
        float rmsEnvelopeFeedback;
        
        int clip;
    } drLevelMeter;
    
    //TODO: pass parameters, like env speed
    void drLevelMeter_init(drLevelMeter* meter, int channel);
    
    void drLevelMeter_processBuffer(void* levelMeter, float* inBuffer, int numChannels, int numFrames);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_LEVEL_METER_H */