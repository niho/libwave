#include "platform_util.h"
#include "raw_encoder.h"
#include "opus_encoder.h"
#include "mem.h"

#include <Foundation/Foundation.h>

int drGetWritableFilePath(char* buffer, int bufferSize)
{
    NSString *filePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"drtempfile"];
    int pathLen = strlen([filePath UTF8String]);
    if (pathLen > bufferSize - 1)
    {
        return 1;
    }
    
    strcpy(buffer, [filePath UTF8String]);
    
    return 0;
}

void drCreateEncoder(drEncoder* encoder)
{
    //TODO: aac encoder
    if (0)
    {
        //raw
        drRawEncoder* rawEncoder = DR_MALLOC(sizeof(drRawEncoder), "raw encoder");
        encoder->encoderData = rawEncoder;
        encoder->cancelCallback = drRawEncoder_cancel;
        encoder->finishCallback = drRawEncoder_finish;
        encoder->initCallback = drRawEncoder_init;
        encoder->writeCallback = drRawEncoder_write;
    }
    else
    {
        //opus
        drOpusEncoder* oupusEncoder = DR_MALLOC(sizeof(drOpusEncoder), "opus encoder");
        encoder->encoderData = oupusEncoder;
        encoder->cancelCallback = drOpusEncoder_cancel;
        encoder->finishCallback = drOpusEncoder_finish;
        encoder->initCallback = drOpusEncoder_init;
        encoder->writeCallback = drOpusEncoder_write;
    }
}