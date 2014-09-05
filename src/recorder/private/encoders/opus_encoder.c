#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "opus_encoder.h"


//https://chromium.googlesource.com/chromium/deps/opus/+/3c3902f0ac13428394f14f78f0fab05ef3468d69/doc/trivial_example.c

//TODO: KOLLA IN http://stackoverflow.com/questions/12516650/opus-audio-codec-encoding-for-iphone
//https://review.webrtc.org/10489004/
//http://holdenc.altervista.org/parole/

drError drOpusEncoder_init(void* opusEncoder, const char* filePath, float fs, float numChannels)
{
    assert(numChannels <= 2);
    
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    encoder->numAccumulatedInputFrames = 0;
    int err;
    assert(encoder->encoder == 0);
    //TODO: sample rate must match system rate!
    encoder->encoder = opus_encoder_create(48000, numChannels, OPUS_APPLICATION_AUDIO, &err);
    if (err < 0)
    {
        fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
        //assert(err >= 0 && "failed to create opus encoder");
        return DR_FAILED_TO_INITIALIZE_ENCODER;
    }
    
    err = opus_encoder_ctl(encoder->encoder, OPUS_SET_BITRATE(BITRATE));
    assert(err >= 0 && "failed to set oupus encoder bitrate");
    
    assert(encoder->file == 0);
    encoder->file = fopen(filePath, "w");
    if (encoder->file == 0)
    {
        return DR_FAILED_TO_OPEN_ENCODER_TARGET_FILE;
    }
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    
    //accumulate buffer to encode
    for (int i = 0; i < numFrames; i++)
    {
        for (int c = 0; c < numChannels; c++)
        {
            int destIdx = encoder->numAccumulatedInputFrames * numChannels + c;
            int srcIdx = numChannels * i + c;
            encoder->scratchInputBuffer[destIdx] = buffer[srcIdx];
            encoder->numAccumulatedInputFrames++;
            if (encoder->numAccumulatedInputFrames == FRAME_SIZE)
            {
                encoder->numAccumulatedInputFrames = 0;
                int encodedPacketSize = opus_encode_float(encoder->encoder,
                                                          encoder->scratchInputBuffer,
                                                          FRAME_SIZE,
                                                          encoder->scratchOutputBuffer,
                                                          MAX_PACKET_SIZE);
                if (encodedPacketSize < 0)
                {
                    fprintf(stderr, "encode failed: %s\n", opus_strerror(encodedPacketSize));
                    return DR_FAILED_TO_ENCODE_AUDIO_DATA;
                }
                else
                {
                    printf("encoded opus packet of size %d\n", encodedPacketSize);
                    int n = fwrite(encoder->scratchOutputBuffer, 1, encodedPacketSize, encoder->file);
                    if (n != encodedPacketSize)
                    {
                        return DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA;
                    }
                }
            }
        }
    }
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_finish(void* opusEncoder)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    opus_encoder_destroy(encoder->encoder);
    encoder->encoder = 0;
    
    if (fclose(encoder->file) != 0)
    {
        return DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
    }
    
    encoder->file = 0;
    
    return DR_NO_ERROR;
}

drError drOpusEncoder_cancel(void* opusEncoder)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    opus_encoder_destroy(encoder->encoder);
    encoder->encoder = 0;
    
    if (fclose(encoder->file) != 0)
    {
        return DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE;
    }
    
    encoder->file = 0;
    
    return DR_NO_ERROR;
}
