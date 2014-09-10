#ifndef DR_SETTINGS_H
#define DR_SETTINGS_H

/*! \file  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     *
     */
    typedef struct drSettings
    {
        int errorFIFOCapacity;
        int realtimeDataFIFOCapacity;
        int controlEventFIFOCapacity;
        int notificationFIFOCapacity;
        int recordFIFOCapacity;
        
        /** In seconds. */
        float levelMeterAttackTime;
        /** In seconds. */
        float levelMeterReleaseTime;
        /** 0 to disable. */
        float rmsWindowSizeInSeconds;
        
        //desired audio stream parameters
        int desiredSampleRate;
        int desiredBufferSizeInFrames;
        int desiredNumInputChannels;
        int desiredNumOutputChannels;
    } drSettings;
    
    /**
     * @param settings
     */
    void drSettings_setDefaults(drSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_SETTINGS_H */