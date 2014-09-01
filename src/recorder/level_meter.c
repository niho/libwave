#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "level_meter.h"

void drLevelMeter_init(drLevelMeter* meter)
{
    memset(meter, 0, sizeof(drLevelMeter));
}

void drLevelMeter_processBuffer(drLevelMeter* meter,
                                const float* buffer,
                                int numFrames,
                                int offset,
                                int stride)
{
    if (numFrames == 0)
    {
        return;
    }
    
    //RMS http://www.mathworks.se/help/signal/ref/rms.html
    float rms = 0.0f;
    float peak = 0.0f;
    
    for (int i = 0; i < numFrames; i++)
    {
        const float absVal = fabs(buffer[i * stride + offset]);
        rms += absVal * absVal;
        
        if (peak < absVal)
        {
            peak = absVal;
        }
    }
    
    rms = sqrtf(rms / numFrames);
}