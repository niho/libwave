#ifndef DR_IOS_AAC_ENCODER_H
#define DR_IOS_AAC_ENCODER_H

#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    typedef struct driOSAACEncoder
    {
        //ExtAudioFileRef destAudioFile;
        FILE* file;
        AudioFileID destAudioFile;
    } driOSAACEncoder;
    
    drError driOSAACEncoder_initCallback(void* encoderData, const char* filePath, float fs, float numChannels);
    
    drError driOSAACEncoder_writeCallback(void* encoderData, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    drError driOSAACEncoder_stopCallback(void* encoderData);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_IOS_AAC_ENCODER_H */