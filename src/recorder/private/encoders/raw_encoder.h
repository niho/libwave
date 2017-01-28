#ifndef WAVE_RAW_ENCODER_H
#define WAVE_RAW_ENCODER_H

#include <stdio.h>

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Writes raw PCM samples to disk. 
     */
    typedef struct WaveRawEncoder
    {
        FILE* file;   
    } WaveRawEncoder;
    
    WaveError wave_raw_encoder_init(void* rawEncoder, const char* filePath, float fs, float numChannels);
    
    WaveError wave_raw_encoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    WaveError wave_raw_encoder_stop(void* rawEncoder);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_RAW_ENCODER_H */
