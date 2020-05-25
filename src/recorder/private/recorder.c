#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "wave.h"
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
        case WAVE_STREAMING_STARTED:
        {
            return "streaming started";
        }
        case WAVE_STREAMING_PAUSED:
        {
            return "streaming paused";
        }
        case WAVE_STREAMING_RESUMED:
        {
            return "streaming resumed";
        }
        case WAVE_STREAMING_STOPPED:
        {
            return "streaming stopped";
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
                    WaveAudioStreamCallback audioStreamCallback,
                    void* callbackUserData,
                    WaveSettings* settings)
{
    if (instance)
    {
        return WAVE_ALREADY_INITIALIZED;
    }
    
    instance = WAVE_MALLOC(sizeof(WaveInstance), "libwave instance");
    wave_instance_init(instance,
                    notificationCallback,
                    errorCallback,
                    audioStreamCallback,
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

WaveError wave_start_streaming()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_request_start_streaming(instance);
    
    return WAVE_NO_ERROR;
}

WaveError wave_stop_streaming()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_enqueue_control_event_of_type(instance, WAVE_STOP_STREAMING);
    
    return WAVE_NO_ERROR;
}

WaveError wave_pause_streaming()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_enqueue_control_event_of_type(instance, WAVE_PAUSE_STREAMING);
    
    return WAVE_NO_ERROR;
}

WaveError wave_resume_streaming()
{
    if (!instance)
    {
        return WAVE_NOT_INITIALIZED;
    }
    
    wave_instance_enqueue_control_event_of_type(instance, WAVE_RESUME_STREAMING);
    
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
