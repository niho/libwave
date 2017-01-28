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
    WaveOpusEncoder opusEncoder;
    memset(&opusEncoder, 0, sizeof(WaveOpusEncoder));
    
    const int nChannels = 1;
    const int fs = 48000;
    const float sineDurSec = 0.5;
    
    //TODO
    const char* filePath = "opus_test_simple.opus";
    remove(filePath);
    const int sigLength = sineDurSec * fs;
    float* sine440 = generateSine(fs, 440, sigLength);
    float* sine880 = generateSine(fs, 880, sigLength);
    
    wave_opus_encoder_init(&opusEncoder, filePath, fs, nChannels);
    
    int n = 0;
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine440, &n);
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine880, &n);
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine440, &n);
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine880, &n);
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine440, &n);
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine880, &n);
    
    wave_opus_encoder_stop(&opusEncoder);
    
    free(sine440);
    free(sine880);
}

static void testChainedStreamEncode()
{
    WaveOpusEncoder opusEncoder;
    memset(&opusEncoder, 0, sizeof(WaveOpusEncoder));
    
    const int nChannels = 1;
    const int fs = 48000;
    const float sineDurSec = 1.0f;
    
    //TODO
    const char* filePath = "opus_test_chained.opus";
    remove(filePath);
    
    const int sigLength = sineDurSec * fs;
    float* sine440 = generateSine(fs, 440, sigLength);
    float* sine1500 = generateSine(fs, 1500, sigLength);
    int n = 0;
    wave_opus_encoder_init(&opusEncoder, filePath, fs, nChannels);
    
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine440, &n);
    
    wave_opus_encoder_stop(&opusEncoder);
    
    //init with existing file
    wave_opus_encoder_init(&opusEncoder, filePath, fs, nChannels);
    
    wave_opus_encoder_write(&opusEncoder, nChannels, sigLength, sine1500, &n);
    
    wave_opus_encoder_stop(&opusEncoder);
    
    free(sine440);
    free(sine1500);
}

void testOggOpus()
{
    testSimpleEncode();
    testChainedStreamEncode();
}
