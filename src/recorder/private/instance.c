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
    
}

static void audioThreadUpdateCallback(void* data)
{
    
}

static void inputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    float maxAbs = 0;
    
    for (int i = 0; i < numFrames; i++)
    {
        const float val = inBuffer[i * numChannels];
        
        if (val > fabsf(maxAbs))
        {
            maxAbs = val;
        }
    }
    
    drInstance* in = (drInstance*)data;
    in->inputPeakValue = maxAbs;
    
    printf("input peak value %f\n", in->inputPeakValue);
}

static void outputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    for (int i = 0; i < numFrames; i++)
    {
        //inBuffer[numChannels * i] = 0.2f * (-1 + 0.0002f * (rand() % 10000));
        inBuffer[numChannels * i + 1] = 0.2f * (-1 + 0.0002f * (rand() % 10000));
    }
}

void drInstance_enqueuEventFromAudioToMainThread(drInstance* instance, const drEvent* event)
{
    //TODO: pass instance as param
    stfMessageQueue_addMessage(instance->outgoingEventQueueAudio, (void*)event);
}

void drInstance_init(drInstance* instance)
{
    instance = (drInstance*)malloc(sizeof(drInstance));
    memset(instance, 0, sizeof(drInstance));
    
    instance->outgoingEventQueueShared = stfMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drEvent));
    instance->outgoingEventQueueAudio = stfMessageQueue_new(kEventQueueCapactity,
                                                             sizeof(drEvent));
    instance->outgoingEventQueueMain = stfMessageQueue_new(kEventQueueCapactity,
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
    
    instance->sampleRate = sampleRate;
    
    kwlInitialize(sampleRate, numOutChannels, numInputChannels, bufferSize);
    kwlError initResult = kwlGetError();
    assert(initResult == KWL_NO_ERROR);
    
    kwlDSPUnitAttachToInput(instance->inputDSPUnit);
    kwlDSPUnitAttachToOutput(instance->outputDSPUnit);
}

void drInstance_deinit(drInstance* instance)
{
    kwlDeinitialize();
    kwlError deinitResult = kwlGetError();
    assert(deinitResult == KWL_NO_ERROR);
    
    stfMessageQueue_delete(instance->outgoingEventQueueAudio);
    stfMessageQueue_delete(instance->outgoingEventQueueMain);
    stfMessageQueue_delete(instance->outgoingEventQueueShared);
    mtx_destroy(&instance->sharedEventQueueLock);
    memset(instance, 0, sizeof(drInstance));
    free(instance);
}
