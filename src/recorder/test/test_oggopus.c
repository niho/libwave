#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <private/encoders/opus_encoder.h>

#include "test_oggopus.h"

static float* generateSine(int fs, int f, int nSamples)
{
    float* s = (float*)malloc(nSamples * sizeof(float));
    for (int i = 0; i < nSamples; i++)
    {
        s[i] = sinf(2 * M_PI * f * i / (float)fs);
    }
    
    return s;
}

static void testSimpleEncode()
{
    drOpusEncoder opusEncoder;
    memset(&opusEncoder, 0, sizeof(drOpusEncoder));
    
    //TODO
    const int nChannels = 1;
    const int fs = 48000;
    const int sineDurSec = 5;
    const char* filePath = "/Users/perarne/code/digger_recorder/unit_test.opus";
    const int sigLength = sineDurSec * fs;
    float* sine440 = generateSine(fs, 440, sigLength);
    float* sine880 = generateSine(fs, 880, sigLength);
    
    drOpusEncoder_init(&opusEncoder, filePath, fs, nChannels);
    
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine440);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine880);
    
    drOpusEncoder_finish(&opusEncoder);
    
    free(sine440);
    free(sine880);
}

void testOggOpus()
{
    testSimpleEncode();
}
