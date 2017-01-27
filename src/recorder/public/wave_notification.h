#ifndef WAVE_NOTIFICATION_H
#define WAVE_NOTIFICATION_H

/*! \file  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Valid notification types.
     */
    typedef enum WaveNotificationType
    {
        WAVE_DID_INITIALIZE = 0,
        WAVE_DID_SHUT_DOWN,
        
        WAVE_RECORDING_STARTED,
        WAVE_RECORDING_PAUSED,
        WAVE_RECORDING_RESUMED,
        WAVE_RECORDING_STOPPED,
        
        WAVE_LEVEL_LOW_WARNING,
        WAVE_LEVEL_HIGH_WARNING

        
    } WaveNotificationType;
    
    /**
     * A notification passed to the main (i.e UI) thread.
     */
    typedef struct WaveNotification
    {
        WaveNotificationType type;
    } WaveNotification;
    
    /**
     * Helper function that returns a human readable description of a
     * notification type.
     */
    const char* wave_notification_type_str(WaveNotificationType type);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_NOTIFICATION_H */
