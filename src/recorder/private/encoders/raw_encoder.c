#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "wave_error_codes.h"
#include "stream.h"

#include "raw_encoder.h"

WaveError wave_raw_encoder_init(void* rawEncoder, WaveStream stream, float sampleRate, float numChannels)
{
    WaveRawEncoder* re = (WaveRawEncoder*)rawEncoder;
    
    re->stream = stream;
    
    return WAVE_NO_ERROR;
}

WaveError wave_raw_encoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten)
{
    *numBytesWritten = 0;
    
    WaveRawEncoder* re = (WaveRawEncoder*)rawEncoder;
    
    int bytesWritten = sizeof(float) * numFrames * numChannels;
    re->stream.write(buffer, bytesWritten, re->stream.userData);
    
    *numBytesWritten = bytesWritten;
    
    return WAVE_NO_ERROR;
}

WaveError wave_raw_encoder_stop(void* rawEncoder)
{
    WaveRawEncoder* re = (WaveRawEncoder*)rawEncoder;
    
    memset(re, 0, sizeof(WaveRawEncoder));
    
    return WAVE_NO_ERROR;
}
