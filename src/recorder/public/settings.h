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
        int controlEventFIFOCapacity;
        int notificationFIFOCapacity;
        int recordFIFOCapacity;
        int desiredSampleRate;
        int desiredBufferSizeInFrames;
        /** In seconds. */
        float levelMeterAttackTime;
        /** In seconds. */
        float levelMeterReleaseTime;
    } drSettings;
    
    /**
     * @param settings
     */
    void drSettings_setDefaults(drSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_SETTINGS_H */