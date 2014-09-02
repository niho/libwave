#ifndef DR_LEVEL_METER_H
#define DR_LEVEL_METER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * The state of a single channel level meter.
     */
    typedef struct drLevelMeter
    {
        /** The index of the channel to process. */
        int channel;
        /** */
        int rmsWindowPos;
        /** */
        int rmsWindowSize;
        /** */
        float runningSquareSum;
        /** 
         * @see http://www.mathworks.se/help/signal/ref/rms.html
         */
        float rmsLevel;
        /** */
        float* rmsWindow;
        
        /** */
        float peak;
        /** */
        float peakEnvelope;
        /** */
        float peakEnvelopeFeedbackAttack;
        /** */
        float peakEnvelopeFeedbackDecay;
        /** */
        int clip;
    } drLevelMeter;
    
    void drLevelMeter_init(drLevelMeter* meter,
                           int channel,
                           float fs,
                           float attackTime,
                           float decayTime,
                           float rmsWindowSizeInSeconds);
    
    /** 
     * Buffer processing callback.
     */
    void drLevelMeter_processBuffer(void* levelMeter, float* inBuffer, int numChannels, int numFrames);
    
    /** 
     * Cleanup callback.
     */
    void drLevelMeter_deinit(void* levelMeter);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_LEVEL_METER_H */