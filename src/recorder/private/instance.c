#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "instance.h"
#include "digger_recorder.h"

#define kEventQueueCapactity (50)

/**
 * Gets called from the main thread.
 */
static void mainThreadUpdateCallback(void* data)
{
    drInstance* in = (drInstance*)data;
    
    
    mtx_lock(&in->communicationQueueLock);
    
    //move any incoming messages to memory shared between the main
    //and the audio threads
    drMessageQueue_moveMessages(in->controlEventQueueMain, in->controlEventQueueShared);
    
    //move any outgoing messages to memory only accessed from the main thread
    drMessageQueue_moveMessages(in->outgoingEventQueueShared, in->outgoingEventQueueMain);
    
    mtx_unlock(&in->communicationQueueLock);
    
    //invoke the event callback for any incoming events on the main thread
    if (in->notificationCallback)
    {
        const int numEvents = drMessageQueue_getNumMessages(in->outgoingEventQueueMain);
        for (int i = 0; i < numEvents; i++)
        {
            const drNotification* e = (const drNotification*)drMessageQueue_getMessage(in->outgoingEventQueueMain, i);
            drInstance_onMainThreadEvent(in, e);
            in->notificationCallback(e, in->notificationCallbackData);
        }
        
        drMessageQueue_clear(in->outgoingEventQueueMain);
    }
}

/**
 * Gets called from the audio thread.
 */
static void audioThreadUpdateCallback(void* data)
{
    drInstance* in = (drInstance*)data;
    
    mtx_lock(&in->communicationQueueLock);
    
    //move any incoming messages to memory only accessed from the audio thread
    drMessageQueue_moveMessages(in->controlEventQueueShared, in->controlEventQueueAudio);
    
    //move any outgoing messages to memory shared between the main
    //and the audio threads
    drMessageQueue_moveMessages(in->outgoingEventQueueAudio, in->outgoingEventQueueShared);
    
    mtx_unlock(&in->communicationQueueLock);
    
    const int numEvents = drMessageQueue_getNumMessages(in->controlEventQueueAudio);
    for (int i = 0; i < numEvents; i++)
    {
        const drNotification* e = (const drNotification*)drMessageQueue_getMessage(in->controlEventQueueAudio, i);
        drInstance_onAudioThreadEvent(in, e);
    }
    
    drMessageQueue_clear(in->outgoingEventQueueMain);
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
}

static void outputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    drInstance* in = (drInstance*)data;
    
    if (in->firstSampleHasPlayed == 0)
    {
        drNotification e;
        e.type = DR_DID_START_AUDIO_STREAM;
        drInstance_enqueueEventFromAudioToMainThread(in, &e);
        in->firstSampleHasPlayed = 1;
    }
    
    for (int i = 0; i < numFrames; i++)
    {
        //inBuffer[numChannels * i] = 0.2f * (-1 + 0.0002f * (rand() % 10000));
        inBuffer[numChannels * i + 1] = 0.2f * (-1 + 0.0002f * (rand() % 10000));
    }
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

void drInstance_enqueueEventFromAudioToMainThread(drInstance* instance, const drNotification* event)
{
    drMessageQueue_addMessage(instance->outgoingEventQueueAudio, (void*)event);
}

void drInstance_enqueueEventFromMainToAudioThread(drInstance* instance, const drControlEvent* event)
{
    drMessageQueue_addMessage(instance->controlEventQueueMain, (void*)event);
}

void drInstance_init(drInstance* instance, drNotificationCallback notificationCallback, void* notificationCallbackUserData)
{
    memset(instance, 0, sizeof(drInstance));
    
    
    instance->notificationCallback = notificationCallback;
    instance->notificationCallbackData = notificationCallbackUserData;
    
    instance->outgoingEventQueueShared = drMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drNotification));
    instance->outgoingEventQueueAudio = drMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drNotification));
    instance->outgoingEventQueueMain = drMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drNotification));
    
    instance->controlEventQueueShared = drMessageQueue_new(kEventQueueCapactity,
                                                            sizeof(drControlEvent));
    instance->controlEventQueueAudio = drMessageQueue_new(kEventQueueCapactity,
                                                           sizeof(drControlEvent));
    instance->controlEventQueueMain = drMessageQueue_new(kEventQueueCapactity,
                                                          sizeof(drControlEvent));
    
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
    assert(numOutChannels <= MAX_NUM_OUTPUT_CHANNELS);
    for (int i = 0; i < numOutChannels; i++)
    {
        drLevelMeter_init(&instance->inputLevelMeters[i], i);
        drInstance_addInputAnalyzer(instance,
                                    &instance->inputLevelMeters[i],
                                    drLevelMeter_processBuffer,
                                    drLevelMeter_deinit);
    }
    
    assert(numInputChannels <= MAX_NUM_INPUT_CHANNELS);
    for (int i = 0; i < numInputChannels; i++)
    {
        drLevelMeter_init(&instance->outputLevelMeters[i], i);
        drInstance_addInputAnalyzer(instance,
                                    &instance->outputLevelMeters[i],
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
    //stop the audio system
    kwlDeinitialize();
    kwlError deinitResult = kwlGetError();
    assert(deinitResult == KWL_NO_ERROR);
    
    //clean up analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (instance->inputAnalyzerSlots[i].analyzerData &&
            instance->inputAnalyzerSlots[i].deinitCallback)
        {
            instance->inputAnalyzerSlots[i].deinitCallback(instance->inputAnalyzerSlots[i].analyzerData);
        }
    }
    
    //release event queues
    drMessageQueue_delete(instance->outgoingEventQueueAudio);
    drMessageQueue_delete(instance->outgoingEventQueueMain);
    drMessageQueue_delete(instance->outgoingEventQueueShared);
    
    drMessageQueue_delete(instance->controlEventQueueAudio);
    drMessageQueue_delete(instance->controlEventQueueMain);
    drMessageQueue_delete(instance->controlEventQueueShared);
    
    mtx_destroy(&instance->communicationQueueLock);

    //clear the instance
    memset(instance, 0, sizeof(drInstance));
}

void drInstance_update(drInstance* instance, float timeStep)
{
    //update kowalski, invoking thread communication callbacks
    kwlUpdate(timeStep);
}

void drInstance_onAudioThreadEvent(drInstance* instance, const drNotification* event)
{
    
}

void drInstance_onMainThreadEvent(drInstance* instance, const drNotification* event)
{

}

static float lin2LogLevel(float lin)
{
    //TODO: tweak this
    return powf(lin, 0.3f);
}

void drInstance_getInputLevels(drInstance* instance, int channel, int logLevels, drLevels* result)
{
    assert(0 && "TODO: copy data between threads etc");
    
    memset(result, 0, sizeof(drLevels));
    
    if (channel >= MAX_NUM_INPUT_CHANNELS)
    {
        return;
    }
    
    drLevelMeter* lm = &instance->inputLevelMeters[channel];
    
    if (logLevels)
    {
        result->peakLevel = lin2LogLevel(lm->peak);
        result->peakLevelEnvelope = lin2LogLevel(lm->peakEnvelope);
        result->rmsLevel = lin2LogLevel(lm->rms);
    }
    else
    {
        result->peakLevel = lm->peak;
        result->peakLevelEnvelope = lm->peakEnvelope;
        result->rmsLevel = lm->rms;
    }
    
    result->hasClipped = lm->clip;
    
}
