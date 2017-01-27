#include <string.h>
#include "wave_settings.h"

void wave_settings_init(WaveSettings* settings)
{
    memset(settings, 0, sizeof(WaveSettings));
    
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
    
    settings->encoderFormat = WAVE_ENCODER_FORMAT_OPUS;
}
