#ifndef DR_OPUS_ENCODER_H
#define DR_OPUS_ENCODER_H

/*! \file */

#include "opus.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    typedef struct drOpusEncoder
    {
        OpusEncoder* encoder;
        FILE* file;

    } drOpusEncoder;
    
    void drOpusEncoder_init(void* opusEncoder, const char* filePath, float fs, float numChannels);
    
    void drOpusEncoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer);
    
    void drOpusEncoder_finish(void* opusEncoder);
    
    void drOpusEncoder_cancel(void* opusEncoder);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_OPUS_ENCODER_H */