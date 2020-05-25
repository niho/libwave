#ifndef WAVE_H
#define WAVE_H

/*! \file 
 Public  API
 */

#include <stdio.h>

#include "wave_error_codes.h"
#include "wave_notification.h"
#include "wave_settings.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Real time info, like input levels.
     */
    typedef struct WaveRealtimeInfo
    {
        float rmsLevel;
        float peakLevel;
        float peakLevelEnvelope;
        int hasClipped;
        float numRecordedSeconds;
    } WaveRealtimeInfo;
    
    /**
     * Information useful when debugging and monitoring.
     */
    typedef struct WaveDevInfo
    {
        /** 0 - 1, 0 empty, 1 full. */
        float recordFIFOLevel;
        /** 0 - 1, 0 empty, 1 full. */
        float notificationFIFOLevel;
        /** 0 - 1, 0 empty, 1 full. */
        float controlEventFIFOLevel;
        /** 
         * Gets set to a non-zero value if the audio recording FIFO has
         * had an overrun since the previous call to wave_get_dev_info().
         */
        int recordFIFOUnderrun;
        /**
         * Gets set to a non-zero value if the notification FIFO has
         * had an overrun since the previous call to wave_get_dev_info().
         */
        int notificationFIFOUnderrun;
        /**
         * Gets set to a non-zero value if the control event FIFO has
         * had an overrun since the previous call to wave_get_dev_info().
         */
        int controlEventFIFOUnderrun;
        /**
         * Gets set to a non-zero value if the error FIFO has
         * had an overrun since the previous call to wave_get_dev_info().
         */
        int errorFIFOUnderrun;
    } WaveDevInfo;
    
    /**
     * A callback to invoke when errors occur.
     */
    typedef void (*WaveErrorCallback)(WaveError errorCode, void* userData);
    
    /**
     * A callback to invoke for each incoming notification.
     */
    typedef void (*WaveNotificationCallback)(const WaveNotification* event, void* userData);
    
    /**
     * A callback to invoke for streaming audio.
     */
    typedef void (*WaveAudioStreamCallback)(const void* buffer, size_t numBytes, void* userData);
    
    /**
     * Initializes the recorder.
     */
    WaveError wave_init(WaveNotificationCallback notificationCallback,
                        WaveErrorCallback errorCallback,
                        WaveAudioStreamCallback audioStreamCallback,
                        void* callbackUserData,
                        WaveSettings* settings);
    
    /**
     * Shuts down the recorder.
     */
    WaveError wave_deinit();
    
    /**
     * Called continually from the main/UI thread to pump new events
     * and invoke event callbacks.
     */
    WaveError wave_update(float timeStep);
    
    /**
     *
     */
    WaveError wave_get_realtime_info(int channel, int logLevels, WaveRealtimeInfo* result);
    
    /**
     * Starts streaming audio to the audioWrittenCallback.
     */
    WaveError wave_start_streaming();
    
    
    /**
     * Shuts down the encoder and closes the stream.
     * If streaming is not in progress, this function does nothing.
     */
    WaveError wave_stop_streaming();
    
    /**
     * Pauses streaming, i.e prevents the audio thread from passing
     * audio data to the encoder until \c wave_resume_streaming() is called.
     * If the system is not streaming or if streaming is already paused,
     * this function does nothing.
     * @see wave_resume_streaming
     */
    WaveError wave_pause_streaming();
    
    /**
     * Resumes streaming, i.e instructs the audio thread to start
     * passing audio data to the encoder again.
     * @see wave_pause_streaming
     */
    WaveError wave_resume_streaming();
    
    /**
     *
     */
    WaveError wave_get_dev_info(WaveDevInfo* devInfo);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_H */
