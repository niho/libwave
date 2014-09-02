#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "mem.h"

#include "level_meter.h"

void drLevelMeter_init(drLevelMeter* meter, int channel)
{
    memset(meter, 0, sizeof(drLevelMeter));
    meter->channel = channel;
    
    meter->rmsWindowSize = 1000;
    meter->rmsWindow = (float*)DR_MALLOC(sizeof(float) * meter->rmsWindowSize, "rms window");
    memset(meter->rmsWindow, 0, sizeof(float) * meter->rmsWindowSize);
}

void drLevelMeter_processBuffer(void* levelMeter,
                                float* inBuffer,
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
    
    for (int i = 0; i < numFrames; i++)
    {
        const float val = inBuffer[i * numChannels + channel];
        const float absVal = fabs(val);
        const float sq = absVal * absVal;
        
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
    }
    
    //refresh rms value
    meter->rmsLevel = sqrtf(meter->runningSquareSum / meter->rmsWindowSize);
}

void drLevelMeter_deinit(void* levelMeter)
{
    drLevelMeter* meter = (drLevelMeter*)levelMeter;
    DR_FREE(meter->rmsWindow);
}