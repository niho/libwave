#ifndef DR_NOTIFICATION_H
#define DR_NOTIFICATION_H

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
        DR_RECORDING_FINISHED,
        DR_RECORDING_CANCELED
        
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

#endif /* DR_NOTIFICATION_H */