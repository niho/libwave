#ifndef DIGGER_RECORDER_H
#define DIGGER_RECORDER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    typedef enum drError
    {
        DR_NO_ERROR = 0
    } drError;
    
    typedef enum drEventType
    {
        DR_RECORDING_STARTED = 0,
        DR_RECORDING_PAUSED,
        DR_RECORDING_STOPPED
    } drEventType;
    
    typedef struct drEvent
    {
        drEventType type;
    } drEvent;
    
    typedef void (*drEventListener)(void* userData, const drEvent* event);
    
    //TODO: pass sample rate etc here
    drError drInitialize(drEventListener* eventListener);
    
    drError drDeinitialize();
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DIGGER_RECORDER_H */