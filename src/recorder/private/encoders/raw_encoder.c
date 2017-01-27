#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "wave_error_codes.h"

#include "raw_encoder.h"

drError drRawEncoder_init(void* rawEncoder, const char* filePath, float fs, float numChannels)
{
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    assert(re->file == 0);
    re->file = fopen(filePath, "a+");
    if (re->file == 0)
    {
        return DR_FAILED_TO_OPEN_ENCODER_TARGET_FILE;
    }
    
    return DR_NO_ERROR;
}

drError drRawEncoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten)
{
    *numBytesWritten = 0;
    
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    size_t bytesWritten = fwrite(buffer, sizeof(float), numFrames * numChannels, re->file);
    if (numFrames * numChannels != bytesWritten)
    {
        return DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA;
    }
    
    fflush(re->file);
    
    *numBytesWritten = bytesWritten;
    
    return DR_NO_ERROR;
}

drError drRawEncoder_stop(void* rawEncoder)
{
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    if (fclose(re->file) != 0)
    {
        return DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
    }
    
    re->file = 0;
    
    return DR_NO_ERROR;
}
