#ifndef DR_DIGGER_RECORDER_H
#define DR_DIGGER_RECORDER_H

/*! \file 
 Public Digger Recorder API
 */

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
     * A callback that gets invoked when the recorder needs to create
     * an audio file to record to.
     */
    typedef const char* (*drWritableAudioFilePathCallback)(void* userData);
    
    /**
     * Initializes the recorder.
     * @param notificationCallback
     * @param errorCallback
     * @param callbackUserData Gets passed as an argument in all callbacks.
     * @param settings If NULL, default settings will be used.
     */
    drError drInitialize(drNotificationCallback notificationCallback,
                         drErrorCallback errorCallback,
                         drWritableAudioFilePathCallback writableFilePathCallback,
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
     *
     */
    drError drStartRecording();
    
    /**
     * 
     */
    drError drCancelRecording();
    
    /**
     *
     */
    drError drFinishRecording();
    
    /**
     *
     */
    drError drPauseRecording();
    
    /**
     *
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