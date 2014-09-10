#ifndef DR_DIGGER_RECORDER_H
#define DR_DIGGER_RECORDER_H

/*! \file 
 Public Digger Recorder API
 */

#include <stdio.h>

#include "error_codes.h"
#include "notification.h"
#include "settings.h"

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
    typedef void (*drErrorCallback)(drError errorCode, void* userData);
    
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
    drError drInitialize(drNotificationCallback notificationCallback,
                         drErrorCallback errorCallback,
                         drAudioWrittenCallback audioWrittenCallback,
                         void* callbackUserData,
                         const char* settingsFilePath,
                         drSettings* settings);
    
    /**
     * Shuts down the recorder.
     */
    drError drDeinitialize();
    
    /**
     * Called continually from the main/UI thread to pump new events
     * and invoke event callbacks.
     */
    drError drUpdate(float timeStep);
    
    /**
     *
     */
    drError drGetRealtimeInfo(int channel, int logLevels, drRealtimeInfo* result);
    
    /**
     * Starts recording audio to a file at a given path. If \c audioFilePath
     * already exists, it is assumed that some audio has already been recorded
     * to that file and new audio will be appended to the end of the file.
     * @param audioFilePath The path of the file to record to.
     */
    drError drStartRecording(const char* audioFilePath);
    
    /**
     *
     */
    drError drCancelRecording();
    
    /**
     *
     */
    drError drFinishRecording();
    
    /**
     * Pauses recording, i.e prevents the audio thread from passing
     * audio data to the encoder until \c drResumeRecording() is called.
     * If the system is not recording or if recording is already paused,
     * this function does nothing.
     * @see drResumeRecording
     */
    drError drPauseRecording();
    
    /**
     * Resumes recording, i.e instructs the audio thread to start
     * passing audio data to the encoder again.
     * @see drPauseRecording
     */
    drError drResumeRecording();
    
    /**
     *
     */
    drError drGetDevInfo(drDevInfo* devInfo);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_DIGGER_RECORDER_H */