#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "wave_recorder.h"
#include "tinycthread.h"
#include "instance.h"
#include "mem.h"

const char* drNotificationTypeToString(drNotificationType type)
{
    switch (type)
    {
        case DR_DID_INITIALIZE:
        {
            return "audio system did initialize";
        }
        case DR_DID_SHUT_DOWN:
        {
            return "audio system shut down";
        }
        case DR_RECORDING_STARTED:
        {
            return "recording started";
        }
        case DR_RECORDING_PAUSED:
        {
            return "recording paused";
        }
        case DR_RECORDING_RESUMED:
        {
            return "recording resumed";
        }
        case DR_RECORDING_STOPPED:
        {
            return "recording stopped";
        }
        case DR_LEVEL_HIGH_WARNING:
        {
            return "input level high warning";
        }
        case DR_LEVEL_LOW_WARNING:
        {
            return "input level low warning";
        }
        default:
        {
            break;
        }
    }
    
    return "unknown notification type";
}

static drInstance* instance = NULL;

WaveError drInitialize(drNotificationCallback notificationCallback,
                     drErrorCallback errorCallback,
                     drAudioWrittenCallback audioWrittenCallback,
                     void* callbackUserData,
                     drSettings* settings)
{
    if (instance)
    {
        return WAVE_ALREADY_INITIALIZED;
    }
    
    instance = DR_MALLOC(sizeof(drInstance), "libwave recorder instance");
    drInstance_init(instance, 
                    notificationCallback,
                    errorCallback,
                    audioWrittenCallback,
                    callbackUserData,
                    settings);
    
    return WAVE_NO_ERROR;
}

WaveError drDeinitialize()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_deinit(instance);
    DR_FREE(instance);
    instance = 0;
    
    return WAVE_NO_ERROR;
}

WaveError drUpdate(float timeStep)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_update(instance, timeStep);
    
    return WAVE_NO_ERROR;
}

WaveError drGetRealtimeInfo(int channel, int logLevels, drRealtimeInfo* result)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_getRealtimeInfo(instance, channel, logLevels, result);
    
    return WAVE_NO_ERROR;
}

WaveError drStartRecording(const char* audioFilePath)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_requestStartRecording(instance, audioFilePath);
    
    return WAVE_NO_ERROR;
}

WaveError drStopRecording()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_enqueueControlEventOfType(instance, DR_STOP_RECORDING);
    
    return WAVE_NO_ERROR;
}

WaveError drPauseRecording()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_enqueueControlEventOfType(instance, DR_PAUSE_RECORDING);
    
    return WAVE_NO_ERROR;
}

WaveError drResumeRecording()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_enqueueControlEventOfType(instance, DR_RESUME_RECORDING);
    
    return WAVE_NO_ERROR;
}

WaveError drGetDevInfo(drDevInfo* devInfo)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    drInstance_getDevInfo(instance, devInfo);
    
    return WAVE_NO_ERROR;
}
