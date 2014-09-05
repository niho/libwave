#ifndef DR_ENCODER_H
#define DR_ENCODER_H

/*! \file */

#include "error_codes.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    typedef drError (*drEncoderInitCallback)(void* encoderData, const char* filePath, float fs, float numChannels);
    
    typedef drError (*drEncoderWriteCallback)(void* encoderData, int numChannels, int numFrames, float* buffer);
    
    typedef drError (*drEncoderFinishCallback)(void* encoderData);
    
    typedef drError (*drEncoderCancelCallback)(void* encoderData);
    
    typedef struct drEncoder
    {
        drEncoderInitCallback initCallback;
        drEncoderWriteCallback writeCallback;
        drEncoderFinishCallback finishCallback;
        drEncoderCancelCallback cancelCallback;
        void* encoderData;
    } drEncoder;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_ENCODER_H */