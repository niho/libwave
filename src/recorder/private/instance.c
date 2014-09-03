#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "opus.h"

#include "instance.h"
#include "digger_recorder.h"
#include "raw_encoder.h"
#include "platform_util.h"


/**
 * Gets called from the main thread.
 */
static void mainThreadUpdateCallback(void* data)
{
    drInstance* in = (drInstance*)data;
    
    
    mtx_lock(&in->communicationQueueLock);
    
    //get measured levels
    memcpy(in->inputLevelsMain, in->inputLevelsShared, sizeof(drLevels) * MAX_NUM_INPUT_CHANNELS);
    
    mtx_unlock(&in->communicationQueueLock);
    
    //invoke the event callback for any incoming events on the main thread
    if (in->notificationCallback)
    {
        drNotification n;
        while (drLockFreeFIFO_pop(&in->notificationFIFO, &n))
        {
            drInstance_onMainThreadNotification(in, &n);
            in->notificationCallback(&n, in->notificationCallbackData);
        }
    }
}

/**
 * Gets called from the audio thread.
 */
static void audioThreadUpdateCallback(void* data)
{
    drInstance* in = (drInstance*)data;
    
    //update measured levels
    for (int i = 0; i < MAX_NUM_INPUT_CHANNELS; i++)
    {
        drLevels* ol = &in->inputLevelsAudio[i];
        drLevelMeter* m = &in->inputLevelMeters[i];
        
        ol->peakLevel = m->peak;
        ol->peakLevelEnvelope = m->peakEnvelope;
        ol->rmsLevel = m->rmsLevel;
        
        //TODO: edge detect or something?
        ol->hasClipped = m->clip;
    }
    
    mtx_lock(&in->communicationQueueLock);
    
    //copy measured levels to the main thread
    memcpy(in->inputLevelsShared, in->inputLevelsAudio, sizeof(drLevels) * MAX_NUM_INPUT_CHANNELS);
    
    mtx_unlock(&in->communicationQueueLock);
    
    drControlEvent e;
    while (drLockFreeFIFO_pop(&in->controlEventFIFO, &e))
    {
        drInstance_onAudioThreadControlEvent(in, &e);
    }
}

static void inputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    drInstance* in = (drInstance*)data;
    //pass audio input to analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (in->inputAnalyzerSlots[i].analyzerData != NULL)
        {
            in->inputAnalyzerSlots[i].audioCallback(in->inputAnalyzerSlots[i].analyzerData, inBuffer, numChannels, numFrames);
        }
    }
    
    //record, if requested
    if (in->stateAudioThread == DR_STATE_RECORDING)
    {
        drRecordedChunk c;
        
        const int lastBuffer = in->finishRecordingRequested;
        
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
            c.numFrames = samplesLeft;
            c.numChannels = numChannels;
            c.lastChunk = 0;
            if (lastBuffer && chunkIdx == numChunks - 1)
            {
                c.lastChunk = 1;
            }
            memcpy(c.samples, &inBuffer[sampleIdx], samplesLeft * sizeof(float));
            //printf("sending %d frames of recorded %d channel audio to the main thread\n", samplesLeft, numChannels);
            drLockFreeFIFO_push(&in->inputAudioDataQueue, &c);
            sampleIdx += samplesLeft;
            chunkIdx++;
        }
        
        if (in->finishRecordingRequested)
        {
            in->stateAudioThread = DR_STATE_IDLE;
            drInstance_enqueueNotificationOfType(in, DR_RECORDING_FINISHED);
        }
    }
}

static void outputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    drInstance* in = (drInstance*)data;
    
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

void drInstance_init(drInstance* instance, drNotificationCallback notificationCallback, void* notificationCallbackUserData)
{
    memset(instance, 0, sizeof(drInstance));
    
    //printf("opus_get_version_string %s\n", opus_get_version_string());
    
    
    //remember which thread created the instance to verify that functions get called
    //from the right threads
    instance->mainThread = thrd_current();
    
    //hook up notification callback
    instance->notificationCallback = notificationCallback;
    instance->notificationCallbackData = notificationCallbackUserData;
    
    drCreateEncoder(&instance->recordingSession.encoder);
    
    drLockFreeFIFO_init(&instance->inputAudioDataQueue, kRecordFIFOCapacity, sizeof(drRecordedChunk));
    
    //create notification and control event queues
    drLockFreeFIFO_init(&instance->notificationFIFO, kNotificationFIFOCapacity, sizeof(drNotification));
    drLockFreeFIFO_init(&instance->controlEventFIFO, kControlEventFIFOCapacity, sizeof(drControlEvent));
    
    mtx_init(&instance->communicationQueueLock, mtx_plain);
    
    instance->inputDSPUnit = kwlDSPUnitCreateCustom(instance,
                                                    inputCallback,
                                                    NULL,
                                                    NULL,
                                                    NULL);
    
    instance->outputDSPUnit = kwlDSPUnitCreateCustom(instance,
                                                     outputCallback,
                                                     mainThreadUpdateCallback,
                                                     audioThreadUpdateCallback,
                                                     NULL);
    
    //TODO: pass these as arguments
    const int sampleRate = 44100;
    const int numOutChannels = 2;
    const int numInputChannels = 1;
    const int bufferSize = 512;
    
    //create audio analyzers
    assert(numInputChannels <= MAX_NUM_INPUT_CHANNELS);
    for (int i = 0; i < numInputChannels; i++)
    {
        drLevelMeter_init(&instance->inputLevelMeters[i],
                          i,
                          sampleRate,
                          0.0001f,
                          2.0f,
                          0.5f);
        drInstance_addInputAnalyzer(instance,
                                    &instance->inputLevelMeters[i],
                                    drLevelMeter_processBuffer,
                                    drLevelMeter_deinit);
    }
    
    
    instance->sampleRate = sampleRate;
    
    kwlInitialize(sampleRate, numOutChannels, numInputChannels, bufferSize);
    kwlError initResult = kwlGetError();
    assert(initResult == KWL_NO_ERROR);
    
    kwlDSPUnitAttachToInput(instance->inputDSPUnit);
    kwlDSPUnitAttachToOutput(instance->outputDSPUnit);
}

void drInstance_deinit(drInstance* instance)
{
    assert(drInstance_isOnMainThread(instance));
    //stop the audio system
    kwlDeinitialize();
    kwlError deinitResult = kwlGetError();
    assert(deinitResult == KWL_NO_ERROR);
    
    if (instance->notificationCallback)
    {
        drNotification n;
        n.type = DR_DID_SHUT_DOWN;
        instance->notificationCallback(&n, instance->notificationCallbackData);
    }
    
    //clean up analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (instance->inputAnalyzerSlots[i].analyzerData &&
            instance->inputAnalyzerSlots[i].deinitCallback)
        {
            instance->inputAnalyzerSlots[i].deinitCallback(instance->inputAnalyzerSlots[i].analyzerData);
        }
    }
    
    drLockFreeFIFO_deinit(&instance->inputAudioDataQueue);
    
    //release event queues
    drLockFreeFIFO_deinit(&instance->notificationFIFO);
    
    drLockFreeFIFO_deinit(&instance->controlEventFIFO);
    
    mtx_destroy(&instance->communicationQueueLock);

    //clear the instance
    memset(instance, 0, sizeof(drInstance));
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
    
    //update kowalski, invoking thread communication callbacks
    kwlUpdate(timeStep);
    
    //pump audio data FIFO after the notifiaction FIFO, to make sure
    //a recording started event arrives before the first audio data.
    drRecordedChunk c;
    while (drLockFreeFIFO_pop(&instance->inputAudioDataQueue, &c))
    {
        //printf("%d frames of recorded %d channel audio arrived on the main thread\n", c.numFrames, c.numChannels);
        instance->recordingSession.numRecordedFrames += c.numFrames;
        
        instance->recordingSession.encoder.writeCallback(instance->recordingSession.encoder.encoderData,
                                                         c.numChannels,
                                                         c.numFrames,
                                                         c.samples);
        
        //printf("recorded %d frames on the main thread\n", instance->recordingSession.numRecordedFrames);
        if (c.lastChunk)
        {
            drInstance_finishRecording(instance);
        }
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

void drInstance_initiateRecording(drInstance* instance)
{
    assert(drInstance_isOnMainThread(instance));
    
    char filePath[1024];
    drGetWritableFilePath(filePath, 1024);
    
    instance->recordingSession.encoder.initCallback(instance->recordingSession.encoder.encoderData,
                                                    filePath,
                                                    44100, //TODO
                                                    1); //TODO
    printf("drInstance_initiateRecording\n");
}

void drInstance_finishRecording(drInstance* instance)
{
    assert(drInstance_isOnMainThread(instance));
    printf("drInstance_finishRecording\n");
    
    instance->recordingSession.encoder.finishCallback(instance->recordingSession.encoder.encoderData);

    instance->recordingSession.numRecordedFrames = 0;
}

void drInstance_cancelRecording(drInstance* instance)
{
    assert(drInstance_isOnMainThread(instance));
    printf("drInstance_cancelRecording\n");
    
    instance->recordingSession.encoder.cancelCallback(instance->recordingSession.encoder.encoderData);
    
    instance->recordingSession.numRecordedFrames = 0;
    
}

void drInstance_enqueueNotification(drInstance* instance, const drNotification* notification)
{
    assert(!drInstance_isOnMainThread(instance));
    drLockFreeFIFO_push(&instance->notificationFIFO, notification);
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
    drLockFreeFIFO_push(&instance->controlEventFIFO, (void*)event);
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
        case DR_FINISH_RECORDING:
        {
            if (instance->stateAudioThread == DR_STATE_RECORDING ||
                instance->stateAudioThread == DR_STATE_RECORDING_PAUSED)
            {
                //finish recording requested and we're currently recording.
                //finish up.
                instance->finishRecordingRequested = 1;
            }
            break;
        }
        case DR_CANCEL_RECORDING:
        {
            if (instance->stateAudioThread == DR_STATE_RECORDING ||
                instance->stateAudioThread == DR_STATE_RECORDING_PAUSED)
            {
                //cancel recording requested and we're currently recording.
                //cancel.
                instance->stateAudioThread = DR_STATE_IDLE;
                drInstance_enqueueNotificationOfType(instance, DR_RECORDING_CANCELED);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void drInstance_onMainThreadNotification(drInstance* instance, const drNotification* notification)
{
    assert(drInstance_isOnMainThread(instance));
    
    switch (notification->type)
    {
        case DR_DID_INITIALIZE:
        {
            break;
        }
        case DR_RECORDING_CANCELED:
        {
            drInstance_cancelRecording(instance);
            instance->stateMainThread = DR_STATE_IDLE;
            break;
        }
        case DR_RECORDING_FINISHED:
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
        case DR_RECORDING_STARTED:
        {
            instance->stateMainThread = DR_STATE_RECORDING;
            //initiate recording
            drInstance_initiateRecording(instance);
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

void drInstance_getInputLevels(drInstance* instance, int channel, int logLevels, drLevels* result)
{
    memset(result, 0, sizeof(drLevels));
    
    if (channel >= MAX_NUM_INPUT_CHANNELS)
    {
        return;
    }
    
    drLevels* lSrc = &instance->inputLevelsMain[channel];
    
    result->hasClipped = lSrc->hasClipped;
    result->peakLevel = logLevels ? lin2LogLevel(lSrc->peakLevel) : lSrc->peakLevel;
    result->peakLevelEnvelope = logLevels ? lin2LogLevel(lSrc->peakLevelEnvelope) : lSrc->peakLevelEnvelope;
    result->rmsLevel = logLevels ? lin2LogLevel(lSrc->rmsLevel) : lSrc->rmsLevel;
}
