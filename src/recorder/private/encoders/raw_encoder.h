#ifndef DR_RAW_ENCODER_H
#define DR_RAW_ENCODER_H

#include <stdio.h>

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    typedef struct drRawEncoder
    {
        FILE* file;   
    } drRawEncoder;
    
    void drRawEncoder_init(void* rawEncoder, const char* filePath, float fs, float numChannels);
    
    void drRawEncoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer);
    
    void drRawEncoder_finish(void* rawEncoder);
    
    void drRawEncoder_cancel(void* rawEncoder);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_RAW_ENCODER_H */