#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "kowalski.h"

#include "digger_recorder.h"
#include "messagequeue.h"
#include "tinycthread.h"
#include "instance.h"
#include "mem.h"

static drInstance* instance = NULL;

drError drInitialize(drNotificationCallback notificationCallback, void* notificationCallbackUserData)
{
    if (instance)
    {
        return DR_ALREADY_INITIALIZED;
    }
    
    instance = DR_MALLOC(sizeof(drInstance), "Digger recorder instance");
    drInstance_init(instance, notificationCallback, notificationCallbackUserData);
    
    
    return DR_NO_ERROR;
}

drError drDeinitialize()
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drInstance_deinit(instance);
    DR_FREE(instance);
    
    return DR_NO_ERROR;
}

drError drUpdate(float timeStep)
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drInstance_update(instance, timeStep);
    
    return DR_NO_ERROR;
}

drError drGetInputLevels(int channel, int logLevels, drLevels* result)
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drInstance_getInputLevels(instance, channel, logLevels, result);
    
    return DR_NO_ERROR;
}

drError drStartRecording()
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drControlEvent ce;
    ce.type = DR_START_RECORDING;
    drInstance_enqueueControlEvent(instance, &ce);
    
    return DR_NO_ERROR;
}

drError drStopRecording()
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drControlEvent ce;
    ce.type = DR_STOP_RECORDING;
    drInstance_enqueueControlEvent(instance, &ce);
    
    return DR_NO_ERROR;
}

drError drPauseRecording()
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drControlEvent ce;
    ce.type = DR_PAUSE_RECORDING;
    drInstance_enqueueControlEvent(instance, &ce);
    
    return DR_NO_ERROR;
}
