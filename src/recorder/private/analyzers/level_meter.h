#ifndef WAVE_LEVEL_METER_H
#define WAVE_LEVEL_METER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * The state of a single channel level meter.
     */
    typedef struct WaveLevelMeter
    {
        /** The index of the channel to process. */
        int channel;
        
        int disableRMS;
        
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
    } WaveLevelMeter;
    
    void wave_level_meter_init(WaveLevelMeter* meter,
                           int channel,
                           float fs,
                           float attackTime,
                           float decayTime,
                           float rmsWindowSizeInSeconds);
    
    /** 
     * Buffer processing callback.
     */
    void wave_level_meter_process_buffer(void* levelMeter, const float* inBuffer, int numChannels, int numFrames);
    
    /** 
     * Cleanup callback.
     */
    void wave_level_meter_deinit(void* levelMeter);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_LEVEL_METER_H */
