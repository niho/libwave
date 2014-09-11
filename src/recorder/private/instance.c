#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include <assert.h>

#include "opus.h"
#include "mem.h"

#include "instance.h"
#include "digger_recorder.h"
#include "raw_encoder.h"
#include "platform_util.h"


drError drInstance_init(drInstance* instance,
                        drNotificationCallback notificationCallback,
                        drErrorCallback errorCallback,
                        drAudioWrittenCallback audioWrittenCallback,
                        void* callbackUserData,
                        drSettings* settings)
{
    memset(instance, 0, sizeof(drInstance));
    
    //use custom settings if provided.
    if (settings)
    {
        memcpy(&instance->settings, settings, sizeof(drSettings));
    }
    else
    {
        drSettings_setDefaults(&instance->settings);
    }
    
    //printf("opus_get_version_string %s\n", opus_get_version_string());
    
    
    //remember which thread created the instance to verify that functions get called
    //from the right threads
    instance->mainThread = thrd_current();
    
    //hook up notification callback
    instance->audioWrittenCallback = audioWrittenCallback;
    instance->notificationCallback = notificationCallback;
    instance->errorCallback = errorCallback;
    instance->callbackUserData = callbackUserData;
    
    drCreateEncoder(&instance->recordingSession.encoder);
    
    drLockFreeFIFO_init(&instance->inputAudioDataQueue,
                        instance->settings.recordFIFOCapacity,
                        sizeof(drRecordedChunk));
    
    //create error, notification and control event queues
    drLockFreeFIFO_init(&instance->notificationFIFO,
                        instance->settings.notificationFIFOCapacity,
                        sizeof(drNotification));
    
    drLockFreeFIFO_init(&instance->controlEventFIFO,
                        instance->settings.controlEventFIFOCapacity,
                        sizeof(drControlEvent));
    
    drLockFreeFIFO_init(&instance->errorFIFO,
                        instance->settings.errorFIFOCapacity,
                        sizeof(drError));
    
    drLockFreeFIFO_init(&instance->realTimeDataFifo,
                        instance->settings.realtimeDataFIFOCapacity,
                        sizeof(drRealtimeInfo));
    
    
    //create audio analyzers
    assert(instance->settings.desiredNumInputChannels <= MAX_NUM_INPUT_CHANNELS);
    for (int i = 0; i < instance->settings.desiredNumInputChannels; i++)
    {
        drLevelMeter_init(&instance->inputLevelMeters[i],
                          i,
                          instance->settings.desiredSampleRate,
                          instance->settings.levelMeterAttackTime,
                          instance->settings.levelMeterReleaseTime,
                          instance->settings.rmsWindowSizeInSeconds);
        drInstance_addInputAnalyzer(instance,
                                    &instance->inputLevelMeters[i],
                                    drLevelMeter_processBuffer,
                                    drLevelMeter_deinit);
    }
    
    //create level advisor
    drLevelAdvisor_init(&instance->levelAdvisor,
                        instance,
                        0,
                        instance->settings.desiredSampleRate);
    drInstance_addInputAnalyzer(instance,
                                &instance->levelAdvisor,
                                drLevelAdvisor_processBuffer,
                                drLevelAdvisor_deinit);
    
    drError initResult = drInstance_hostSpecificInit(instance);
    return initResult;
}

drError drInstance_deinit(drInstance* instance)
{
    assert(drInstance_isOnMainThread(instance));
    //stop the audio system
    drError deinitResult = drInstance_hostSpecificDeinit(instance);
    
    drNotification n;
    n.type = DR_DID_SHUT_DOWN;
    drInstance_invokeNotificationCallback(instance, &n);
    
    //clean up analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (instance->inputAnalyzerSlots[i].analyzerData &&
            instance->inputAnalyzerSlots[i].deinitCallback)
        {
            instance->inputAnalyzerSlots[i].deinitCallback(instance->inputAnalyzerSlots[i].analyzerData);
        }
    }
    
    DR_FREE(instance->recordingSession.encoder.encoderData);
    
    drLockFreeFIFO_deinit(&instance->inputAudioDataQueue);
    
    //release event queues
    drLockFreeFIFO_deinit(&instance->notificationFIFO);
    drLockFreeFIFO_deinit(&instance->controlEventFIFO);
    drLockFreeFIFO_deinit(&instance->errorFIFO);
    drLockFreeFIFO_deinit(&instance->realTimeDataFifo);

    //clear the instance
    memset(instance, 0, sizeof(drInstance));
    
    return deinitResult;
}

void drInstance_update(drInstance* instance, float timeStep)
{
    assert(drInstance_isOnMainThread(instance));
    
    //update dev info before consuming any events
    instance->devInfo.controlEventFIFOLevel = drLockFreeFIFO_getNumElements(&instance->controlEventFIFO) /
    ((float)instance->controlEventFIFO.capacity);
    
    instance->devInfo.notificationFIFOLevel = drLockFreeFIFO_getNumElements(&instance->notificationFIFO) /
    ((float)instance->notificationFIFO.capacity);
    
    instance->devInfo.recordFIFOLevel = drLockFreeFIFO_getNumElements(&instance->inputAudioDataQueue) /
    ((float)instance->inputAudioDataQueue.capacity);
    
    //get measured levels
    drRealtimeInfo l;
    while (drLockFreeFIFO_pop(&instance->realTimeDataFifo, &l))
    {
        //TODO: number of input channels?
        memcpy(&instance->realtimeInfo, &l, sizeof(drRealtimeInfo));
    }
    
    //invoke the error callback for any incoming errors on the main thread
    drError e;
    while (drLockFreeFIFO_pop(&instance->errorFIFO, &e))
    {
        drInstance_onMainThreadError(instance, e);
    }
    
    //invoke the event callback for any incoming events on the main thread
    drNotification n;
    while (drLockFreeFIFO_pop(&instance->notificationFIFO, &n))
    {
        drInstance_onMainThreadNotification(instance, &n);
    }
    
    //pump audio data FIFO after the notifiaction FIFO, to make sure
    //a recording started event arrives before the first audio data.
    drRecordedChunk c;
    int errorOccured = 0;
    while (drLockFreeFIFO_pop(&instance->inputAudioDataQueue, &c))
    {
        //printf("%d frames of recorded %d channel audio arrived on the main thread\n", c.numFrames, c.numChannels);
        if (!errorOccured)
        {
            
            //struct timeval  tv1, tv2;
            //gettimeofday(&tv1, NULL);
            /* stuff to do! */
            
            drError writeResult = DR_NO_ERROR;
            if (c.numFrames > 0)
            {
                int numBytesWritten = 0;
                writeResult = instance->recordingSession.encoder.writeCallback(instance->recordingSession.encoder.encoderData,
                                                                               c.numChannels,
                                                                               c.numFrames,
                                                                               c.samples,
                                                                               &numBytesWritten);
                
                if (numBytesWritten > 0)
                {
                    instance->audioWrittenCallback(instance->recordingSession.targetFilePath,
                                                   numBytesWritten,
                                                   instance->callbackUserData);
                }
            }
            
            //gettimeofday(&tv2, NULL);
            
            /*printf ("Encoder write callback took = %f seconds\n",
             (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
             (double) (tv2.tv_sec - tv1.tv_sec));*/
            
            if (writeResult != DR_NO_ERROR)
            {
                errorOccured = 1;
                drInstance_onMainThreadError(instance, writeResult);
                drInstance_enqueueControlEventOfType(instance, DR_STOP_RECORDING);
            }
            else
            {
                //printf("recorded %d frames on the main thread\n", instance->recordingSession.numRecordedFrames);
                if (c.lastChunk)
                {
                    drInstance_stopRecording(instance);
                }
            }
        }
    }
}

void drInstance_audioInputCallback(drInstance* in, const float* inBuffer, int numChannels, int numFrames)
{
    //pass audio input to analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (in->inputAnalyzerSlots[i].analyzerData != NULL)
        {
            in->inputAnalyzerSlots[i].audioCallback(in->inputAnalyzerSlots[i].analyzerData, inBuffer, numChannels, numFrames);
        }
    }
    
    //record, if requested
    if (in->stateAudioThread == DR_STATE_RECORDING ||
        in->stateAudioThread == DR_STATE_RECORDING_PAUSED)
    {
        drRecordedChunk c;
        const int lastBuffer = in->stopRecordingRequested;
        
        const int numSamples = numChannels * numFrames;
        int sampleIdx = 0;
        const int chunkSize = MAX_RECORDED_CHUNK_SIZE;
        const int numChunks = ceil(numSamples / (float)chunkSize);
        int chunkIdx = 0;
        while (sampleIdx < numSamples)
        {
            int samplesLeft = numSamples - sampleIdx;
            if (samplesLeft > chunkSize)
            {
                samplesLeft = chunkSize;
            }
            
            //TODO: this is a double memcpy....
            const int paused = in->stateAudioThread == DR_STATE_RECORDING_PAUSED;
            c.numFrames = paused ? 0 : samplesLeft;
            c.numChannels = numChannels;
            c.lastChunk = 0;
            if (lastBuffer && chunkIdx == numChunks - 1)
            {
                c.lastChunk = 1;
            }
            memcpy(c.samples, &inBuffer[sampleIdx], samplesLeft * sizeof(float));
            //printf("sending %d frames of recorded %d channel audio to the main thread\n", samplesLeft, numChannels);
            int pushSuccess = drLockFreeFIFO_push(&in->inputAudioDataQueue, &c);
            if (pushSuccess == 0)
            {
                in->devInfo.recordFIFOUnderrun = 1;
            }
            else if (in->stateAudioThread == DR_STATE_RECORDING)
            {
                in->recordingSession.numRecordedFrames += samplesLeft;
            }
            
            sampleIdx += samplesLeft;
            chunkIdx++;
        }
        
        
        if (in->stopRecordingRequested)
        {
            in->stopRecordingRequested = 0;
            in->stateAudioThread = DR_STATE_IDLE;
            drInstance_enqueueNotificationOfType(in, DR_RECORDING_STOPPED);
        }
    }
    
    //update measured levels
    for (int i = 0; i < MAX_NUM_INPUT_CHANNELS; i++)
    {
        drRealtimeInfo l;
        
        drLevelMeter* m = &in->inputLevelMeters[i];
        
        l.peakLevel = m->peak;
        l.peakLevelEnvelope = m->peakEnvelope;
        l.rmsLevel = m->rmsLevel;
        l.numRecordedSeconds = in->recordingSession.numRecordedFrames / (float)in->settings.desiredSampleRate;
        
        //TODO: edge detect or something?
        l.hasClipped = m->clip;
        
        //TODO: more channels
        drLockFreeFIFO_push(&in->realTimeDataFifo, &l);
    }
    
    //copy measured levels to the main thread. TODO: pass these using a FIFO
    //memcpy(in->inputLevelsShared, in->inputLevelsAudio, sizeof(drRealtimeInfo) * MAX_NUM_INPUT_CHANNELS);
    
    drControlEvent e;
    while (drLockFreeFIFO_pop(&in->controlEventFIFO, &e))
    {
        drInstance_onAudioThreadControlEvent(in, &e);
    }
}

void drInstance_audioOutputCallback(drInstance* in, float* inBuffer, int numChannels, int numFrames)
{
    if (in->firstSampleHasPlayed == 0)
    {
        drNotification e;
        e.type = DR_DID_INITIALIZE;
        drInstance_enqueueNotification(in, &e);
        in->firstSampleHasPlayed = 1;
    }
    
    for (int i = 0; i < numFrames; i++)
    {
        inBuffer[numChannels * i + 1] = 0.0f;//0.2f * (-1 + 0.0002f * (rand() % 10000));
    }
}

int drInstance_isOnMainThread(drInstance* instance)
{
    return thrd_equal(thrd_current(), instance->mainThread);
}


int drInstance_addInputAnalyzer(drInstance* instance,
                                void* analyzerData,
                                drAnalyzerAudioCallback audioCallback,
                                drAnalyzerDeinitCallback deinitCallback)
{
    //TODO: make sure this is not called when the audio system is running. or lock this somehow.
    for (int i = 0; MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (instance->inputAnalyzerSlots[i].analyzerData == NULL)
        {
            instance->inputAnalyzerSlots[i].analyzerData = analyzerData;
            instance->inputAnalyzerSlots[i].audioCallback = audioCallback;
            instance->inputAnalyzerSlots[i].deinitCallback = deinitCallback;
            
            return 0;
        }
    }
    
    //no free slots :(
    return 1;
}

void drInstance_requestStartRecording(drInstance* instance, const char* filePath)
{
    
    strncpy(instance->requestedAudioFilePath, filePath, DR_MAX_PATH_LEN);
    drInstance_enqueueControlEventOfType(instance, DR_START_RECORDING);

}

void drInstance_initiateRecording(drInstance* instance)
{
    assert(drInstance_isOnMainThread(instance));
    strcpy(instance->recordingSession.targetFilePath, instance->requestedAudioFilePath);
    instance->requestedAudioFilePath[0] = '\0';
    assert(strlen(instance->recordingSession.targetFilePath) > 0);
    drError initResult = instance->recordingSession.encoder.initCallback(instance->recordingSession.encoder.encoderData,
                                                    instance->recordingSession.targetFilePath,
                                                    instance->sampleRate,
                                                    instance->settings.desiredNumInputChannels); //TODO
    
    if (initResult != DR_NO_ERROR)
    {
        drInstance_invokeErrorCallback(instance, initResult);
    }
    
    printf("drInstance_initiateRecording\n");
}

void drInstance_stopRecording(drInstance* instance)
{
    
    assert(drInstance_isOnMainThread(instance));
    printf("drInstance_cancelRecording\n");
    
    //clear any queued up audio chunks
    while (!drLockFreeFIFO_isEmpty(&instance->inputAudioDataQueue))
    {
        drRecordedChunk c;
        drLockFreeFIFO_pop(&instance->inputAudioDataQueue, &c);
        
        //TODO: write this chunk!
        //instance->recordingSession.encoder.writeCallback(
    }
    
    instance->recordingSession.encoder.stopCallback(instance->recordingSession.encoder.encoderData);
    
    instance->recordingSession.numRecordedFrames = 0;
}


void drInstance_invokeErrorCallback(drInstance* instance, drError errorCode)
{
    assert(drInstance_isOnMainThread(instance));
    
    if (instance->errorCallback)
    {
        instance->errorCallback(errorCode, instance->callbackUserData);
    }
}

void drInstance_invokeNotificationCallback(drInstance* instance, const drNotification* notification)
{
    if (instance->notificationCallback)
    {
        instance->notificationCallback(notification, instance->callbackUserData);
    }
}

void drInstance_enqueueError(drInstance* instance, drError error)
{
    assert(!drInstance_isOnMainThread(instance));
    int pushSuccess = drLockFreeFIFO_push(&instance->errorFIFO, &error);
    if (pushSuccess == 0)
    {
        instance->devInfo.errorFIFOUnderrun = 1;
    }
}

void drInstance_enqueueNotification(drInstance* instance, const drNotification* notification)
{
    assert(!drInstance_isOnMainThread(instance));
    int pushSuccess = drLockFreeFIFO_push(&instance->notificationFIFO, notification);
    if (pushSuccess == 0)
    {
        instance->devInfo.notificationFIFOUnderrun = 1;
    }
}

void drInstance_enqueueNotificationOfType(drInstance* instance, drNotificationType type)
{
    drNotification n;
    memset(&n, 0, sizeof(drNotification));
    n.type = type;
    drInstance_enqueueNotification(instance, &n);
}

void drInstance_enqueueControlEvent(drInstance* instance, const drControlEvent* event)
{
    assert(drInstance_isOnMainThread(instance));
    int pushSuccess = drLockFreeFIFO_push(&instance->controlEventFIFO, (void*)event);
    if (pushSuccess == 0)
    {
        instance->devInfo.controlEventFIFOUnderrun = 1;
    }
}

void drInstance_enqueueControlEventOfType(drInstance* instance, drControlEventType type)
{
    drControlEvent e;
    memset(&e, 0, sizeof(drControlEvent));
    e.type = type;
    drInstance_enqueueControlEvent(instance, &e);
}

void drInstance_onAudioThreadControlEvent(drInstance* instance, const drControlEvent* event)
{
    assert(!drInstance_isOnMainThread(instance));
    
    switch (event->type)
    {
        case DR_START_RECORDING:
        {
            if (instance->stateAudioThread == DR_STATE_IDLE)
            {
                //recording requested and we're currently not recording.
                //start recording.
                instance->recordingSession.numRecordedFrames = 0;
                instance->stateAudioThread = DR_STATE_RECORDING;
                drInstance_enqueueNotificationOfType(instance, DR_RECORDING_STARTED);
            }
            break;
        }
        case DR_PAUSE_RECORDING:
        {
            if (instance->stateAudioThread == DR_STATE_RECORDING)
            {
                //recording pause requested and we're currently recording.
                //pause recording.
                instance->stateAudioThread = DR_STATE_RECORDING_PAUSED;
                drInstance_enqueueNotificationOfType(instance, DR_RECORDING_PAUSED);
            }
            break;
        }
        case DR_RESUME_RECORDING:
        {
            if (instance->stateAudioThread == DR_STATE_RECORDING_PAUSED)
            {
                //recording resume requested and we're currently paused.
                //resume recording.
                instance->stateAudioThread = DR_STATE_RECORDING;
                drInstance_enqueueNotificationOfType(instance, DR_RECORDING_RESUMED);
            }
            break;
        }
        case DR_STOP_RECORDING:
        {
            if (instance->stateAudioThread == DR_STATE_RECORDING ||
                instance->stateAudioThread == DR_STATE_RECORDING_PAUSED)
            {
                //cancel recording requested and we're currently recording.
                //cancel.
                instance->stopRecordingRequested = 1;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void drInstance_onMainThreadError(drInstance* instance, drError error)
{
    assert(drInstance_isOnMainThread(instance));
    
    drInstance_invokeErrorCallback(instance, error);
    
}

void drInstance_onMainThreadNotification(drInstance* instance, const drNotification* notification)
{
    assert(drInstance_isOnMainThread(instance));
    
    drInstance_invokeNotificationCallback(instance, notification);
    
    switch (notification->type)
    {
        case DR_DID_INITIALIZE:
        {
            break;
        }
            /*
        case DR_RECORDING_STOPPED:
        {
            drInstance_stopRecording(instance);
            instance->stateMainThread = DR_STATE_IDLE;
            break;
        }*/
        case DR_RECORDING_STARTED:
        {
            instance->stateMainThread = DR_STATE_RECORDING;
            //initiate recording
            drInstance_initiateRecording(instance);
            break;
        }
        case DR_RECORDING_STOPPED:
        {
            //nothing here. the recording is finished when the last buffer
            //has been received.
            instance->stateMainThread = DR_STATE_IDLE;
            break;
        }
        case DR_RECORDING_PAUSED:
        {
            instance->stateMainThread = DR_STATE_RECORDING_PAUSED;
            break;
        }
        case DR_RECORDING_RESUMED:
        {
            instance->stateMainThread = DR_STATE_RECORDING;
            break;
        }
        
        default:
        {
            break;
        }
    }
}

static float lin2LogLevel(float lin)
{
    //TODO: tweak this
    return powf(lin, 0.3f);
}

void drInstance_getRealtimeInfo(drInstance* instance, int channel, int logLevels, drRealtimeInfo* result)
{
    memset(result, 0, sizeof(drRealtimeInfo));
    
    if (channel >= MAX_NUM_INPUT_CHANNELS)
    {
        return;
    }
    
    drRealtimeInfo* lSrc = &instance->realtimeInfo;
    
    result->hasClipped = lSrc->hasClipped;
    result->peakLevel = logLevels ? lin2LogLevel(lSrc->peakLevel) : lSrc->peakLevel;
    result->peakLevelEnvelope = logLevels ? lin2LogLevel(lSrc->peakLevelEnvelope) : lSrc->peakLevelEnvelope;
    result->rmsLevel = logLevels ? lin2LogLevel(lSrc->rmsLevel) : lSrc->rmsLevel;
    result->numRecordedSeconds = lSrc->numRecordedSeconds;
}

void drInstance_getDevInfo(drInstance* instance, drDevInfo* devInfo)
{
    //copy the info to the caller
    memcpy(devInfo, &instance->devInfo, sizeof(drDevInfo));
    
    //clear any flags
    instance->devInfo.recordFIFOUnderrun = 0;
    instance->devInfo.controlEventFIFOUnderrun = 0;
    instance->devInfo.notificationFIFOUnderrun = 0;
    instance->devInfo.errorFIFOUnderrun = 0;
}
