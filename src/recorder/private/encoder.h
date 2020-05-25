#ifndef WAVE_ENCODER_H
#define WAVE_ENCODER_H

/*! \file */

#include "wave_error_codes.h"
#include "stream.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    typedef WaveError (*WaveEncoderInitCallback)(void* encoderData, WaveStream stream, float sampleRate, float numChannels);
    
    typedef WaveError (*WaveEncoderWriteCallback)(void* encoderData, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    typedef WaveError (*WaveEncoderStopCallback)(void* encoderData);
    
    typedef struct WaveEncoder
    {
        WaveEncoderInitCallback initCallback;
        
        WaveEncoderWriteCallback writeCallback;
        
        WaveEncoderStopCallback stopCallback;
        
        void* encoderData;
    } WaveEncoder;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_ENCODER_H */
