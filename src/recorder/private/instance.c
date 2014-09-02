#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "instance.h"
#include "digger_recorder.h"

#define kEventQueueCapactity (50)

static void engineThreadUpdateCallback(void* data)
{
    drInstance* in = (drInstance*)data;
    
    mtx_lock(&in->sharedEventQueueLock);
    drMessageQueue_moveMessages(in->outgoingEventQueueShared, in->outgoingEventQueueMain);
    mtx_unlock(&in->sharedEventQueueLock);
    
    //invoke the event callback for any incoming events
    if (in->eventCallback)
    {
        const int numEvents = drMessageQueue_getNumMessages(in->outgoingEventQueueMain);
        for (int i = 0; i < numEvents; i++)
        {
            const drEvent* e = (const drEvent*)drMessageQueue_getMessage(in->outgoingEventQueueMain, i);
            in->eventCallback(e, in->eventCallbackData);
        }
        
        drMessageQueue_clear(in->outgoingEventQueueMain);
    }
}

static void audioThreadUpdateCallback(void* data)
{
    drInstance* in = (drInstance*)data;
    
    const int numMessages = drMessageQueue_getNumMessages(in->outgoingEventQueueAudio);
    if (numMessages)
    {
        mtx_lock(&in->sharedEventQueueLock);
        drMessageQueue_moveMessages(in->outgoingEventQueueAudio, in->outgoingEventQueueShared);
        mtx_unlock(&in->sharedEventQueueLock);
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
}

static void outputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    drInstance* in = (drInstance*)data;
    
    if (in->firstSampleHasPlayed == 0)
    {
        drEvent e;
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

void drInstance_enqueueEventFromAudioToMainThread(drInstance* instance, const drEvent* event)
{
    //TODO: pass instance as param
    drMessageQueue_addMessage(instance->outgoingEventQueueAudio, (void*)event);
}

void drInstance_init(drInstance* instance, drEventCallback eventCallback, void* eventCallbackUserData)
{
    memset(instance, 0, sizeof(drInstance));
    
    
    instance->eventCallback = eventCallback;
    instance->eventCallbackData = eventCallbackUserData;
    
    instance->outgoingEventQueueShared = drMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drEvent));
    instance->outgoingEventQueueAudio = drMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drEvent));
    instance->outgoingEventQueueMain = drMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drEvent));
    
    mtx_init(&instance->sharedEventQueueLock, mtx_plain);
    
    instance->inputDSPUnit = kwlDSPUnitCreateCustom(instance,
                                                    inputCallback,
                                                    NULL,
                                                    NULL,
                                                    NULL);
    
    instance->outputDSPUnit = kwlDSPUnitCreateCustom(instance,
                                                     outputCallback,
                                                     engineThreadUpdateCallback,
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
    mtx_destroy(&instance->sharedEventQueueLock);

    //clear the instance
    memset(instance, 0, sizeof(drInstance));
}

void drInstance_update(drInstance* instance, float timeStep)
{
    //update kowalski, invoking thread communication callbacks
    kwlUpdate(timeStep);
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
