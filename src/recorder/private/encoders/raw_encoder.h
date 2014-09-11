#ifndef DR_RAW_ENCODER_H
#define DR_RAW_ENCODER_H

#include <stdio.h>

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Writes raw PCM samples to disk. 
     */
    typedef struct drRawEncoder
    {
        FILE* file;   
    } drRawEncoder;
    
    drError drRawEncoder_init(void* rawEncoder, const char* filePath, float fs, float numChannels);
    
    drError drRawEncoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    drError drRawEncoder_stop(void* rawEncoder);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_RAW_ENCODER_H */