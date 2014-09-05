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
        int controlEventFIFOCapacity;
        int notificationFIFOCapacity;
        int recordFIFOCapacity;
    } drSettings;
    
    void drSettings_setDefaults(drSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_SETTINGS_H */