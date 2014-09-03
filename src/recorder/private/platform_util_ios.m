#include "platform_util.h"

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