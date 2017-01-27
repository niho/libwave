#ifndef DR_ENCODER_H
#define DR_ENCODER_H

/*! \file */

#include "wave_error_codes.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    typedef WaveError (*drEncoderInitCallback)(void* encoderData, const char* filePath, float fs, float numChannels);
    
    typedef WaveError (*drEncoderWriteCallback)(void* encoderData, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    typedef WaveError (*drEncoderStopCallback)(void* encoderData);
    
    typedef struct drEncoder
    {
        drEncoderInitCallback initCallback;
        
        drEncoderWriteCallback writeCallback;
        
        drEncoderStopCallback stopCallback;
        
        void* encoderData;
    } drEncoder;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_ENCODER_H */
