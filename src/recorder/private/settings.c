#include <string.h>
#include "settings.h"

void drSettings_setDefaults(drSettings* settings)
{
    memset(settings, 0, sizeof(drSettings));
    
    settings->errorFIFOCapacity = 50;
    settings->realtimeDataFIFOCapacity = 50;
    settings->controlEventFIFOCapacity = 50;
    settings->notificationFIFOCapacity = 50;
    settings->recordFIFOCapacity = 250;
    settings->desiredSampleRate = 48000;
    settings->desiredBufferSizeInFrames = 512;
    settings->desiredNumInputChannels = 1;
    settings->desiredNumOutputChannels = 2;
    
    
    settings->levelMeterAttackTime = 0.0001f;
    settings->levelMeterReleaseTime = 2.0f;
    settings->rmsWindowSizeInSeconds = 0.0f;
    
    settings->encoderFormat = DR_ENCODER_FORMAT_OPUS;
}