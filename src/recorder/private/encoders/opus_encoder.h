#ifndef WAVE_OPUS_ENCODER_H
#define WAVE_OPUS_ENCODER_H

/*! \file */

#include <stdio.h>
#include <ogg/ogg.h>

#include "opus.h"
#include "wave_error_codes.h"
#include "stream.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    //TODO make these tweakable?
    #define BITRATE 64000
    #define FRAME_SIZE 960
    #define MAX_PACKET_SIZE (3*1276)
    
    typedef struct WaveOpusEncoder
    {
        OpusEncoder* encoder;
        ogg_stream_state oggStreamState;
        WaveStream stream;
        unsigned char scratchOutputBuffer[MAX_PACKET_SIZE];
        int numAccumulatedInputFrames;
        float scratchInputBuffer[2 * FRAME_SIZE]; //max 2 channels
        int packetsWritten;
        ogg_packet* oggHeaderPacket0;
        ogg_packet* oggHeaderPacket1;
        int hasWrittenHeaderPackets;

    } WaveOpusEncoder;
    
    WaveError wave_opus_encoder_init(void* opusEncoder, WaveStream stream, float sampleRate, float numChannels);
    
    WaveError wave_opus_encoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer, int* numBytesWritten);
    
    WaveError wave_opus_encoder_stop(void* opusEncoder);
    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_OPUS_ENCODER_H */
