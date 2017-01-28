#ifndef WAVE_RECORDER_H
#define WAVE_RECORDER_H

/*! \file 
 Public Recorder API
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
         * had an overrun since the previous call to drGetDevInfo.
         */
        int recordFIFOUnderrun;
        /**
         * Gets set to a non-zero value if the notification FIFO has
         * had an overrun since the previous call to drGetDevInfo.
         */
        int notificationFIFOUnderrun;
        /**
         * Gets set to a non-zero value if the control event FIFO has
         * had an overrun since the previous call to drGetDevInfo.
         */
        int controlEventFIFOUnderrun;
        /**
         * Gets set to a non-zero value if the error FIFO has
         * had an overrun since the previous call to drGetDevInfo.
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
     *
     */
    typedef void (*WaveAudioWrittenCallback)(const char* path, int numBytes, void* userData);
    
    /**
     * Initializes the recorder.
     */
    WaveError wave_init(WaveNotificationCallback notificationCallback,
                        WaveErrorCallback errorCallback,
                        WaveAudioWrittenCallback audioWrittenCallback,
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
     * Starts recording audio to a file at a given path. If \c audioFilePath
     * already exists, it is assumed that some audio has already been recorded
     * to that file and new audio will be appended to the end of the file.
     * @param audioFilePath The path of the file to record to.
     */
    WaveError wave_start_recording(const char* audioFilePath);
    
    
    /**
     * Shuts down the encoder and closes the file it's recording to.
     * If recording is not in progress, this function does nothing.
     */
    WaveError wave_stop_recording();
    
    /**
     * Pauses recording, i.e prevents the audio thread from passing
     * audio data to the encoder until \c drResumeRecording() is called.
     * If the system is not recording or if recording is already paused,
     * this function does nothing.
     * @see drResumeRecording
     */
    WaveError wave_pause_recording();
    
    /**
     * Resumes recording, i.e instructs the audio thread to start
     * passing audio data to the encoder again.
     * @see drPauseRecording
     */
    WaveError wave_resume_recording();
    
    /**
     *
     */
    WaveError wave_get_dev_info(WaveDevInfo* devInfo);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_RECORDER_H */
