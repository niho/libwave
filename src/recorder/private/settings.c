#include <string.h>
#include "settings.h"

void drSettings_setDefaults(drSettings* settings)
{
    memset(settings, 0, sizeof(drSettings));
    
    settings->errorFIFOCapacity = 50;
    settings->controlEventFIFOCapacity = 50;
    settings->notificationFIFOCapacity = 50;
    settings->recordFIFOCapacity = 250;
    settings->desiredSampleRate = 44100;
    settings->desiredBufferSizeInFrames = 512;
    settings->desiredNumInputChannels = 1;
    settings->desiredNumOutputChannels = 2;
    
    
    settings->levelMeterAttackTime = 0.0001f;
    settings->levelMeterReleaseTime = 2.0f;
}