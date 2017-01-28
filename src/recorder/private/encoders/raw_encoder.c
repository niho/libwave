#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "wave_error_codes.h"

#include "raw_encoder.h"

WaveError wave_raw_encoder_init(void* rawEncoder, const char* filePath, float fs, float numChannels)
{
    WaveRawEncoder* re = (WaveRawEncoder*)rawEncoder;
    assert(re->file == 0);
    re->file = fopen(filePath, "a+");
    if (re->file == 0)
    {
        return WAVE_FAILED_TO_OPEN_ENCODER_TARGET_FILE;
    }
    
    return WAVE_NO_ERROR;
}

WaveError wave_raw_encoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten)
{
    *numBytesWritten = 0;
    
    WaveRawEncoder* re = (WaveRawEncoder*)rawEncoder;
    size_t bytesWritten = fwrite(buffer, sizeof(float), numFrames * numChannels, re->file);
    if (numFrames * numChannels != bytesWritten)
    {
        return WAVE_FAILED_TO_WRITE_ENCODED_AUDIO_DATA;
    }
    
    fflush(re->file);
    
    *numBytesWritten = bytesWritten;
    
    return WAVE_NO_ERROR;
}

WaveError wave_raw_encoder_stop(void* rawEncoder)
{
    WaveRawEncoder* re = (WaveRawEncoder*)rawEncoder;
    if (fclose(re->file) != 0)
    {
        return WAVE_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
    }
    
    re->file = 0;
    
    return WAVE_NO_ERROR;
}
