#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "kowalski.h"

#include "digger_recorder.h"

typedef struct drInstance
{
    float sampleRate;
    kwlDSPUnitHandle inputDSPUnit;
    kwlDSPUnitHandle outputDSPUnit;
    float inputPeakValue;
    
} drInstance;

static drInstance* instance = NULL;

void engineThreadUpdateCallback(void* data)
{
    
}

void audioThreadUpdateCallback(void* data)
{
    
}

void inputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
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

void outputCallback(float* inBuffer, int numChannels, int numFrames, void* data)
{
    for (int i = 0; i < numFrames; i++)
    {
        //inBuffer[numChannels * i] = 0.2f * (-1 + 0.0002f * (rand() % 10000));
        inBuffer[numChannels * i + 1] = 0.2f * (-1 + 0.0002f * (rand() % 10000));
    }
}

drError drInitialize(drEventCallback eventCallback, void* eventCallbackUserData)
{
    if (instance)
    {
        return DR_ALREADY_INITIALIZED;
    }
    
    instance = (drInstance*)malloc(sizeof(drInstance));
    memset(instance, 0, sizeof(drInstance));
    
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
    
    return DR_NO_ERROR;
}

drError drDeinitialize()
{
    if (!instance)
    {
        return DR_NOT_INITIALIZED;
    }
    
    kwlDeinitialize();
    kwlError deinitResult = kwlGetError();
    assert(deinitResult == KWL_NO_ERROR);
    
    memset(instance, 0, sizeof(drInstance));
    free(instance);
    
    return DR_NO_ERROR;
}

drError drUpdate(float timeStep)
{
    kwlUpdate(timeStep);
    return DR_NO_ERROR;
}

