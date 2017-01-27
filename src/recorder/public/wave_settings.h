#ifndef WAVE_SETTINGS_H
#define WAVE_SETTINGS_H

/*! \file  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Encoder formats.
     */
    typedef enum WaveEncoderFormat
    {
        WAVE_ENCODER_FORMAT_RAW = 0,
        WAVE_ENCODER_FORMAT_OPUS,
        WAVE_ENCODER_FORMAT_AAC       // Only available on iOS
    } WaveEncoderFormat;
    
    /**
     * Settings
     */
    typedef struct WaveSettings
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
        WaveEncoderFormat encoderFormat;
    } WaveSettings;
    
    /**
     * @param settings
     */
    void wave_settings_init(WaveSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_SETTINGS_H */
