#ifndef WAVE_SETTINGS_H
#define WAVE_SETTINGS_H

/*! \file  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Error codes.
     */
    typedef enum drEncoderFormat
    {
        DR_ENCODER_FORMAT_RAW = 0,
        DR_ENCODER_FORMAT_OPUS,
        DR_ENCODER_FORMAT_AAC       // Only available on iOS
    } drEncoderFormat;
    
    /**
     * Settings
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
        
        //encoder format
        drEncoderFormat encoderFormat;
    } drSettings;
    
    /**
     * @param settings
     */
    void drSettings_setDefaults(drSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_SETTINGS_H */
