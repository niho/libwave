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
     * Information about audio levels for a single channel.
     */
    typedef struct drLevels
    {
        float rmsLevel;
        float peakLevel;
        float peakLevelEnvelope;
        int hasClipped;
    } drLevels;
    
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
     * Initializes the recorder.
     * @param notificationCallback
     * @param errorCallback
     * @param callbackUserData
     * @param settings If NULL, default settings will be used.
     */
    drError drInitialize(drNotificationCallback notificationCallback,
                         drErrorCallback errorCallback,
                         void* callbackUserData,
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
    drError drGetInputLevels(int channel, int logLevels, drLevels* result);
    
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