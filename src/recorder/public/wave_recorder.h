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
    typedef struct drRealtimeInfo
    {
        float rmsLevel;
        float peakLevel;
        float peakLevelEnvelope;
        int hasClipped;
        float numRecordedSeconds;
    } drRealtimeInfo;
    
    /**
     * Information useful when debugging and monitoring.
     */
    typedef struct drDevInfo
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
    } drDevInfo;
    
    /**
     * A callback to invoke when errors occur.
     */
    typedef void (*drErrorCallback)(WaveError errorCode, void* userData);
    
    /**
     * A callback to invoke for each incoming notification.
     */
    typedef void (*drNotificationCallback)(const drNotification* event, void* userData);
    
    /**
     *
     */
    typedef void (*drAudioWrittenCallback)(const char* path, int numBytes, void* userData);
    
    /**
     * Initializes the recorder.
     */
    WaveError drInitialize(drNotificationCallback notificationCallback,
                         drErrorCallback errorCallback,
                         drAudioWrittenCallback audioWrittenCallback,
                         void* callbackUserData,
                         drSettings* settings);
    
    /**
     * Shuts down the recorder.
     */
    WaveError drDeinitialize();
    
    /**
     * Called continually from the main/UI thread to pump new events
     * and invoke event callbacks.
     */
    WaveError drUpdate(float timeStep);
    
    /**
     *
     */
    WaveError drGetRealtimeInfo(int channel, int logLevels, drRealtimeInfo* result);
    
    /**
     * Starts recording audio to a file at a given path. If \c audioFilePath
     * already exists, it is assumed that some audio has already been recorded
     * to that file and new audio will be appended to the end of the file.
     * @param audioFilePath The path of the file to record to.
     */
    WaveError drStartRecording(const char* audioFilePath);
    
    
    /**
     * Shuts down the encoder and closes the file it's recording to.
     * If recording is not in progress, this function does nothing.
     */
    WaveError drStopRecording();
    
    /**
     * Pauses recording, i.e prevents the audio thread from passing
     * audio data to the encoder until \c drResumeRecording() is called.
     * If the system is not recording or if recording is already paused,
     * this function does nothing.
     * @see drResumeRecording
     */
    WaveError drPauseRecording();
    
    /**
     * Resumes recording, i.e instructs the audio thread to start
     * passing audio data to the encoder again.
     * @see drPauseRecording
     */
    WaveError drResumeRecording();
    
    /**
     *
     */
    WaveError drGetDevInfo(drDevInfo* devInfo);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_RECORDER_H */
