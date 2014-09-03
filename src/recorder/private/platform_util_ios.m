#include "platform_util.h"
#include "raw_encoder.h"

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
    drRawEncoder* rawEncoder = malloc(sizeof(drRawEncoder));
    encoder->encoderData = rawEncoder;
    encoder->cancelCallback = drRawEncoder_cancel;
    encoder->finishCallback = drRawEncoder_finish;
    encoder->initCallback = drRawEncoder_init;
    encoder->writeCallback = drRawEncoder_write;
}