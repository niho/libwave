#include "platform_util.h"
#include "raw_encoder.h"
#include "opus_encoder.h"
#include "ios_aac_encoder.h"
#include "mem.h"

#include <Foundation/Foundation.h>

void drCreateEncoder(drEncoder* encoder)
{
    //TODO: aac encoder
    if (0)
    {
        //raw
        drRawEncoder* rawEncoder = DR_MALLOC(sizeof(drRawEncoder), "raw encoder");
        memset(rawEncoder, 0, sizeof(drRawEncoder));
        encoder->encoderData = rawEncoder;
        encoder->stopCallback = drRawEncoder_stop;
        encoder->initCallback = drRawEncoder_init;
        encoder->writeCallback = drRawEncoder_write;
    }
    else if (1)
    {
        //opus
        drOpusEncoder* opusEncoder = DR_MALLOC(sizeof(drOpusEncoder), "opus encoder");
        memset(opusEncoder, 0, sizeof(drOpusEncoder));
        encoder->encoderData = opusEncoder;
        encoder->stopCallback = drOpusEncoder_stop;
        encoder->initCallback = drOpusEncoder_init;
        encoder->writeCallback = drOpusEncoder_write;
    }
    else
    {
        //ios AAC
        driOSAACEncoder* aacEncoder = DR_MALLOC(sizeof(driOSAACEncoder), "AAC encoder");
        memset(aacEncoder, 0, sizeof(driOSAACEncoder));
        encoder->encoderData = aacEncoder;
        encoder->stopCallback = driOSAACEncoder_stopCallback;
        encoder->initCallback = driOSAACEncoder_initCallback;
        encoder->writeCallback = driOSAACEncoder_writeCallback;
    }
}