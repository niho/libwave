#ifndef DR_ENCODER_H
#define DR_ENCODER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    typedef void (*drEncoderInitCallback)(void* encoderData, const char* filePath, float fs, float numChannels);
    
    typedef void (*drEncoderWriteCallback)(void* encoderData, int numChannels, int numFrames, float* buffer);
    
    typedef void (*drEncoderFinishCallback)(void* encoderData);
    
    typedef void (*drEncoderCancelCallback)(void* encoderData);
    
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