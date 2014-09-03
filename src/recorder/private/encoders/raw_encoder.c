#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"

#include "raw_encoder.h"

void drRawEncoder_init(void* rawEncoder, const char* filePath, float fs, float numChannels)
{
    printf("drRawEncoder_init, file path is %s\n", filePath);
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    assert(re->file == 0);
    re->file = fopen(filePath, "wb");
    assert(re->file);
}

void drRawEncoder_write(void* rawEncoder, int numChannels, int numFrames, float* buffer)
{
    printf("drRawEncoder_write\n");
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    int bytesWritten = fwrite(buffer, sizeof(float), numFrames * numChannels, re->file);
    assert(numFrames * numChannels == bytesWritten);
}

void drRawEncoder_finish(void* rawEncoder)
{
    printf("drRawEncoder_finish\n");
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    fclose(re->file);
}

void drRawEncoder_cancel(void* rawEncoder)
{
    printf("drRawEncoder_cancel\n");
    drRawEncoder* re = (drRawEncoder*)rawEncoder;
    fclose(re->file);
}