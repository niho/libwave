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
    typedef enum drNotificationType
    {
        DR_DID_INITIALIZE = 0,
        DR_DID_SHUT_DOWN,
        
        DR_RECORDING_STARTED,
        DR_RECORDING_PAUSED,
        DR_RECORDING_RESUMED,
        DR_RECORDING_STOPPED,
        
        DR_LEVEL_LOW_WARNING,
        DR_LEVEL_HIGH_WARNING

        
    } drNotificationType;
    
    /**
     * A notification passed to the main (i.e UI) thread.
     */
    typedef struct drNotification
    {
        drNotificationType type;
    } drNotification;
    
    /**
     * Helper function that returns a human readable description of a
     * notification type.
     */
    const char* drNotificationTypeToString(drNotificationType type);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_NOTIFICATION_H */
