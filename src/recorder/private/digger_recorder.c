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

static drInstance* instance = NULL;

drError drInitialize(drEventCallback eventCallback, void* eventCallbackUserData)
{
    if (instance)
    {
        return DR_ALREADY_INITIALIZED;
    }
    
    instance = malloc(sizeof(drInstance));
    drInstance_init(instance, eventCallback, eventCallbackUserData);
    
    
    return DR_NO_ERROR;
}

drError drDeinitialize()
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    drInstance_deinit(instance);
    
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