#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "opus_encoder.h"


//https://chromium.googlesource.com/chromium/deps/opus/+/3c3902f0ac13428394f14f78f0fab05ef3468d69/doc/trivial_example.c

/*The frame size is hardcoded for this sample code but it doesn't have to be*/
#define FRAME_SIZE 960

#define BITRATE 64000
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)

void drOpusEncoder_init(void* opusEncoder, const char* filePath, float fs, float numChannels)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    int err;
    assert(encoder->encoder == 0);
    //TODO: sample rate must match system rate!
    encoder->encoder = opus_encoder_create(48000, numChannels, OPUS_APPLICATION_AUDIO, &err);
    if (err < 0)
    {
        fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
        assert(err >= 0 && "failed to create opus encoder");
    }
    
    err = opus_encoder_ctl(encoder->encoder, OPUS_SET_BITRATE(BITRATE));
    assert(err >= 0 && "failed to set oupus encoder bitrate");
    
    assert(encoder->file == 0);
    encoder->file = fopen(filePath, "w");
    assert(encoder->file && "failed to open opus output file");
}

void drOpusEncoder_write(void* opusEncoder, int numChannels, int numFrames, float* buffer)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    //opus_encode_float(encoder->encoder, buffer, num, <#unsigned char *data#>, <#opus_int32 max_data_bytes#>)
}

void drOpusEncoder_finish(void* opusEncoder)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    fclose(encoder->file);
    encoder->file = 0;
    opus_encoder_destroy(encoder->encoder);
    encoder->encoder = 0;
}

void drOpusEncoder_cancel(void* opusEncoder)
{
    drOpusEncoder* encoder = (drOpusEncoder*)opusEncoder;
    fclose(encoder->file);
    encoder->file = 0;
    opus_encoder_destroy(encoder->encoder);
    encoder->encoder = 0;
}