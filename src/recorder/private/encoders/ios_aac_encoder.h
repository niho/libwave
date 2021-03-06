#ifndef WAVE_IOS_AAC_ENCODER_H
#define WAVE_IOS_AAC_ENCODER_H

#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
#define WAVE_AAC_PCM_BUFFER_SIZE_IN_FRAMES (1 << 12)
#define WAVE_AAC_OUTPUT_BUFFER_SIZE (1 << 15)
    
    typedef struct WaveiOSAACEncoder
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
        /** */
        int numBytesWrittenSinceLastCheck;
        
    } WaveiOSAACEncoder;
    
    WaveError wave_ios_aac_encoder_init_callback(void* encoderData,
                                         const char* filePath,
                                         float fs,
                                         float numChannels);
    
    WaveError wave_ios_aac_encoder_write_callback(void* encoderData,
                                          int numChannels,
                                          int numFrames,
                                          float* buffer,
                                          int* numBytesWritten);
    
    WaveError wave_ios_aac_encoder_stop_callback(void* encoderData);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_IOS_AAC_ENCODER_H */
