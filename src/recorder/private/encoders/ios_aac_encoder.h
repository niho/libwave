#ifndef DR_IOS_AAC_ENCODER_H
#define DR_IOS_AAC_ENCODER_H

#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
#define DR_AAC_PCM_BUFFER_SIZE_IN_FRAMES 1003
#define DR_AAC_OUTPUT_BUFFER_SIZE (1 << 15)
    
    typedef struct driOSAACEncoder
    {
        /** The file to write to. */
        FILE* file;
        /** Audio file handle to use in the AudioFile API. */
        AudioFileID destAudioFile;
        /** Converter responsible for converting PCM to AAC. */
        AudioConverterRef audioConverter;
        /** */
        float* pcmBuffer;
        /** */
        int pcmBufferWritePos;
        /** */
        int numPcmFramesLeftToDeliverToEncoder;
        /** */
        unsigned char* aacOutputBuffer;
        /** */
        AudioStreamPacketDescription* aacOutputPacketDescriptions;
        /* */
        int maxNumOutputPackets;
        /* */
        int outputFilePos;
        
    } driOSAACEncoder;
    
    drError driOSAACEncoder_initCallback(void* encoderData,
                                         const char* filePath,
                                         float fs,
                                         float numChannels);
    
    drError driOSAACEncoder_writeCallback(void* encoderData,
                                          int numChannels,
                                          int numFrames,
                                          float* buffer,
                                          int* numBytesWritten);
    
    drError driOSAACEncoder_stopCallback(void* encoderData);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_IOS_AAC_ENCODER_H */