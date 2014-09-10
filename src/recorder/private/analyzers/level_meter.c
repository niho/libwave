#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "mem.h"

#include "level_meter.h"

void drLevelMeter_init(drLevelMeter* meter,
                       int channel,
                       float fs,
                       float attackTime,
                       float decayTime,
                       float rmsWindowSizeInSeconds)
{
    memset(meter, 0, sizeof(drLevelMeter));
    meter->channel = channel;
    
    const float thresh = 0.01f;
    
    meter->peakEnvelopeFeedbackAttack = powf(thresh, 1.0f / (fs * attackTime));
    meter->peakEnvelopeFeedbackDecay = powf(thresh, 1.0f / (fs * decayTime));
    
    meter->rmsWindowSize = (int)(fs * rmsWindowSizeInSeconds + 0.5f);
    meter->rmsWindow = (float*)DR_MALLOC(sizeof(float) * meter->rmsWindowSize, "rms window");
    memset(meter->rmsWindow, 0, sizeof(float) * meter->rmsWindowSize);
}

void drLevelMeter_processBuffer(void* levelMeter,
                                const float* inBuffer,
                                int numChannels,
                                int numFrames)
{
    drLevelMeter* meter = (drLevelMeter*)levelMeter;
    
    if (numFrames == 0)
    {
        return;
    }
    
    float peak = 0.0f;
    
    const int channel = meter->channel;
    
    meter->clip = 0.0f;
    
    for (int i = 0; i < numFrames; i++)
    {
        const float val = inBuffer[i * numChannels + channel];
        const float absVal = fabs(val);
        const float sq = absVal * absVal;
        
        if (absVal >= 1.0f)
        {
            meter->clip = 1;
        }
        
        //update rms running sum
        const float lastValue = meter->rmsWindow[meter->rmsWindowPos];
        meter->rmsWindow[meter->rmsWindowPos] = sq;
        meter->runningSquareSum += (sq - lastValue);
        meter->rmsWindowPos = (meter->rmsWindowPos + 1) % meter->rmsWindowSize;
        
        //measure peak per buffer
        if (peak < absVal)
        {
            peak = absVal;
        }
        
        //compute peak level envelope
        const float a = absVal > meter->peakEnvelope ? meter->peakEnvelopeFeedbackAttack : meter->peakEnvelopeFeedbackDecay;
        meter->peakEnvelope = absVal * (1.0f - a) + a * meter->peakEnvelope;
    }
    
    meter->peak = peak;
    
    //make sure rounding errors have not caused the running sum to be negative
    meter->runningSquareSum = fmaxf(0.0f, meter->runningSquareSum);
    
    //refresh rms value
    meter->rmsLevel = sqrtf(meter->runningSquareSum / meter->rmsWindowSize);
    assert(!isnan(meter->rmsLevel));
}

void drLevelMeter_deinit(void* levelMeter)
{
    drLevelMeter* meter = (drLevelMeter*)levelMeter;
    DR_FREE(meter->rmsWindow);
    memset(meter, sizeof(drLevelMeter), 0);
}
