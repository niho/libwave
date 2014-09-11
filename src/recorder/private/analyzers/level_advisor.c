#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "instance.h"
#include "assert.h"
#include "mem.h"

#include "level_advisor.h"

void drLevelAdvisor_init(drLevelAdvisor* advisor,
                         drInstance* instance,
                         int channel,
                         float fs)
{
    memset(advisor, 0, sizeof(drLevelAdvisor));
 
    drLevelMeter_init(&advisor->levelMeter, channel, fs, 0.0001f, 2.0f, 0.0f);
    
    advisor->instance = instance;
    
    advisor->loudWarningDelayInFrames = 0.5f * fs;
    advisor->loudThreshold = 0.6f;
    
    advisor->quietWarningDelayInFrames = 3.0f * fs;
    advisor->quietThreshold = 0.2f;
}

void drLevelAdvisor_processBuffer(void* advisorPtr, const float* inBuffer, int numChannels, int numFrames)
{
    drLevelAdvisor* advisor = (drLevelAdvisor*)advisorPtr;
    
    if (advisor->instance->stateAudioThread != DR_STATE_RECORDING)
    {
        advisor->loudWarningTimer = 0;
        advisor->quietWarningTimer = 0;
        return;
    }
    
    drLevelMeter_processBuffer(&advisor->levelMeter, inBuffer, numChannels, numFrames);
    
    const float level = advisor->levelMeter.peakEnvelope;
    
    if (level < advisor->quietThreshold)
    {
        //quiet zone
        advisor->loudWarningTimer = 0;
        advisor->quietWarningTimer += numFrames;
        if (advisor->quietWarningTimer - numFrames <= advisor->quietWarningDelayInFrames &&
            advisor->quietWarningTimer > advisor->quietWarningDelayInFrames)
        {
            //fire quiet warning
            //printf("warning, been below %f for %d frames\n", advisor->quietThreshold, advisor->quietWarningDelayInFrames);
            drInstance_enqueueNotificationOfType(advisor->instance, DR_LEVEL_LOW_WARNING);
        }
    }
    else if (level > advisor->loudThreshold)
    {
        //loud zone
        advisor->loudWarningTimer += numFrames;
        advisor->quietWarningTimer = 0;
        if (advisor->loudWarningTimer - numFrames <= advisor->loudWarningDelayInFrames &&
            advisor->loudWarningTimer > advisor->loudWarningDelayInFrames)
        {
            //fire loud warning
            //printf("warning, been above %f for %d frames\n", advisor->loudThreshold, advisor->loudWarningDelayInFrames);
            drInstance_enqueueNotificationOfType(advisor->instance, DR_LEVEL_HIGH_WARNING);
        }
    }
    else
    {
        //safe zone
        advisor->loudWarningTimer = 0;
        advisor->quietWarningTimer = 0;
    }
}

void drLevelAdvisor_deinit(void* advisorPtr)
{
    drLevelAdvisor* advisor = (drLevelAdvisor*)advisorPtr;
    drLevelMeter_deinit(&advisor->levelMeter);
}
