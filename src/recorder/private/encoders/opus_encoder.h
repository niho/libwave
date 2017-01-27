#ifndef DR_OPUS_ENCODER_H
#define DR_OPUS_ENCODER_H

/*! \file */

#include <stdio.h>
#include <ogg/ogg.h>

#include "opus.h"
#include "wave_error_codes.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    //TODO make these tweakable?
    #define BITRATE 64000
    #define FRAME_SIZE 960
    #define MAX_PACKET_SIZE (3*1276)
    
    typedef struct drOpusEncoder
    {
        OpusEncoder* encoder;
        ogg_stream_state oggStreamState;
        FILE* file;
        unsigned char scratchOutputBuffer[MAX_PACKET_SIZE];
        int numAccumulatedInputFrames;
        float scratchInputBuffer[2 * FRAME_SIZE]; //max 2 channels
        int packetsWritten;
        ogg_packet* oggHeaderPacket0;
        ogg_packet* oggHeaderPacket1;
        int hasWrittenHeaderPackets;

    } drOpusEncoder;
    
    WaveError drOpusEncoder_init(void* opusEncoder, const char* filePath, float fs, float numChannels);
    
    WaveError drOpusEncoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    WaveError drOpusEncoder_stop(void* opusEncoder);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_OPUS_ENCODER_H */
