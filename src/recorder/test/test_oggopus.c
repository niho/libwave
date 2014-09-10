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
    
    const int nChannels = 1;
    const int fs = 48000;
    const float sineDurSec = 0.5;
    
    //TODO
    const char* filePath = "/Users/perarne/code/digger_recorder/opus_test_simple.opus";
    remove(filePath);
    const int sigLength = sineDurSec * fs;
    float* sine440 = generateSine(fs, 440, sigLength);
    float* sine880 = generateSine(fs, 880, sigLength);
    
    drOpusEncoder_init(&opusEncoder, filePath, fs, nChannels);
    
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine440);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine880);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine440);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine880);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine440);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine880);
    
    drOpusEncoder_finish(&opusEncoder);
    
    free(sine440);
    free(sine880);
}

static void testChainedStreamEncode()
{
    drOpusEncoder opusEncoder;
    memset(&opusEncoder, 0, sizeof(drOpusEncoder));
    
    const int nChannels = 1;
    const int fs = 48000;
    const float sineDurSec = 1.0f;
    
    //TODO
    const char* filePath = "/Users/perarne/code/digger_recorder/opus_test_chained.opus";
    remove(filePath);
    
    const int sigLength = sineDurSec * fs;
    float* sine440 = generateSine(fs, 440, sigLength);
    float* sine880 = generateSine(fs, 880, sigLength);
    float* sine1000 = generateSine(fs, 1000, sigLength);
    float* sine1500 = generateSine(fs, 1500, sigLength);
    
    drOpusEncoder_init(&opusEncoder, filePath, fs, nChannels);
    
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine440);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine880);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine440);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine880);
    
    drOpusEncoder_finish(&opusEncoder);
    
    //init with existing file
    drOpusEncoder_init(&opusEncoder, filePath, fs, nChannels);
    
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine1000);
    drOpusEncoder_write(&opusEncoder, nChannels, sigLength, sine1500);
    
    drOpusEncoder_finish(&opusEncoder);
    
    free(sine440);
    free(sine880);
    free(sine1000);
    free(sine1500);
}

void testOggOpus()
{
    testSimpleEncode();
    testChainedStreamEncode();
}
