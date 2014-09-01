#ifndef DR_DIGGER_RECORDER_H
#define DR_DIGGER_RECORDER_H

/*! \file 
 Public Digger Recorder API
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Error codes.
     */
    typedef enum drError
    {
        DR_NO_ERROR = 0,
        DR_ALREADY_INITIALIZED,
        DR_NOT_INITIALIZED
    } drError;

    /**
     * Valid event types.
     */
    typedef enum drEventType
    {
        DR_DID_START_AUDIO_STREAM = 0,
        DR_DID_DEINITIALIZE_AUDIO_SYSTEM,
        
        DR_RECORDING_STARTED,
        DR_RECORDING_PAUSED,
        DR_RECORDING_STOPPED,
        
        DR_INPUT_CLIPPED
    } drEventType;
    
    /**
     * An event passed to the main (i.e UI) thread.
     */
    typedef struct drEvent
    {
        drEventType type;
    } drEvent;
    
    /**
     * A callback to invoke for each incoming event.
     */
    typedef void (*drEventCallback)(const drEvent* event, void* userData);
    
    /**
     * Initializes the recorder.
     */
    drError drInitialize(drEventCallback eventCallback, void* eventCallbackUserData);
    
    /**
     * Shuts down the recorder.
     */
    drError drDeinitialize();
    
    /**
     * Called continually from the main/UI thread to pump new events
     * and invoke event callbacks.
     */
    drError drUpdate(float timeStep);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_DIGGER_RECORDER_H */