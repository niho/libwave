#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "wave_recorder.h"
#include "tinycthread.h"
#include "instance.h"
#include "mem.h"

const char* wave_notification_type_str(WaveNotificationType type)
{
    switch (type)
    {
        case WAVE_DID_INITIALIZE:
        {
            return "audio system did initialize";
        }
        case WAVE_DID_SHUT_DOWN:
        {
            return "audio system shut down";
        }
        case WAVE_RECORDING_STARTED:
        {
            return "recording started";
        }
        case WAVE_RECORDING_PAUSED:
        {
            return "recording paused";
        }
        case WAVE_RECORDING_RESUMED:
        {
            return "recording resumed";
        }
        case WAVE_RECORDING_STOPPED:
        {
            return "recording stopped";
        }
        case WAVE_LEVEL_HIGH_WARNING:
        {
            return "input level high warning";
        }
        case WAVE_LEVEL_LOW_WARNING:
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

static WaveInstance* instance = NULL;

WaveError wave_init(WaveNotificationCallback notificationCallback,
                    WaveErrorCallback errorCallback,
                    WaveAudioWrittenCallback audioWrittenCallback,
                    void* callbackUserData,
                    WaveSettings* settings)
{
    if (instance)
    {
        return WAVE_ALREADY_INITIALIZED;
    }
    
    instance = WAVE_MALLOC(sizeof(WaveInstance), "libwave recorder instance");
    wave_instance_init(instance,
                    notificationCallback,
                    errorCallback,
                    audioWrittenCallback,
                    callbackUserData,
                    settings);
    
    return WAVE_NO_ERROR;
}

WaveError wave_deinit()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_deinit(instance);
    WAVE_FREE(instance);
    instance = 0;
    
    return WAVE_NO_ERROR;
}

WaveError wave_update(float timeStep)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_update(instance, timeStep);
    
    return WAVE_NO_ERROR;
}

WaveError wave_get_realtime_info(int channel, int logLevels, WaveRealtimeInfo* result)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_get_realtime_info(instance, channel, logLevels, result);
    
    return WAVE_NO_ERROR;
}

WaveError wave_start_recording(const char* audioFilePath)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_request_start_recording(instance, audioFilePath);
    
    return WAVE_NO_ERROR;
}

WaveError wave_stop_recording()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_enqueue_control_event_of_type(instance, WAVE_STOP_RECORDING);
    
    return WAVE_NO_ERROR;
}

WaveError wave_pause_recording()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_enqueue_control_event_of_type(instance, WAVE_PAUSE_RECORDING);
    
    return WAVE_NO_ERROR;
}

WaveError wave_resume_recording()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_enqueue_control_event_of_type(instance, WAVE_RESUME_RECORDING);
    
    return WAVE_NO_ERROR;
}

WaveError wave_get_dev_info(WaveDevInfo* devInfo)
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_get_dev_info(instance, devInfo);
    
    return WAVE_NO_ERROR;
}
