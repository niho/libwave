#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "level_meter.h"

void drLevelMeter_init(drLevelMeter* meter, int channel)
{
    memset(meter, 0, sizeof(drLevelMeter));
    meter->channel = channel;
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
    
    //RMS http://www.mathworks.se/help/signal/ref/rms.html
    float rms = 0.0f;
    float peak = 0.0f;
    
    const int channel = meter->channel;
    
    for (int i = 0; i < numFrames; i++)
    {
        const float absVal = fabs(inBuffer[i * numChannels + channel]);
        rms += absVal * absVal;
        
        if (peak < absVal)
        {
            peak = absVal;
        }
    }
    
    rms = sqrtf(rms / numFrames);
}