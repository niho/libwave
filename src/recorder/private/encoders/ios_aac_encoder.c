#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <AudioToolbox/AudioToolbox.h>


#include "assert.h"
#include "error_codes.h"

#include "ios_aac_encoder.h"

//http://stackoverflow.com/questions/4956338/how-can-i-get-aac-encoding-with-extaudiofile-on-ios-to-work?rq=1
//http://stackoverflow.com/questions/12828159/using-extaudiofilewrite-to-write-at-the-end-of-a-file

static float* generateSine(int fs, int f, int nSamples)
{
    float* s = (float*)malloc(nSamples * sizeof(float));
    for (int i = 0; i < nSamples; i++)
    {
        s[i] = sinf(2 * M_PI * f * i / (float)fs);
    }
    
    return s;
}

drError driOSAACEncoder_initCallback(void* encoderData, const char* filePath, float fs, float numChannels)
{
    UInt32 size;
    
    const int testSignalLength = 100007;
    float* testSignal = generateSine(fs, 440, testSignalLength);
    
    // Get the source data format
    AudioStreamBasicDescription sourceFormat;
    memset(&sourceFormat, 0, sizeof(AudioStreamBasicDescription));
    sourceFormat.mBitsPerChannel = 32;
    sourceFormat.mBytesPerFrame = 4 * numChannels;
    sourceFormat.mBytesPerPacket = sourceFormat.mBytesPerFrame;
    sourceFormat.mChannelsPerFrame = numChannels;
    sourceFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    sourceFormat.mFormatID = kAudioFormatLinearPCM;
    sourceFormat.mFramesPerPacket = 1;
    sourceFormat.mSampleRate = fs;
    
    // Define the output format (AAC).
    AudioStreamBasicDescription outputFormat;
    memset(&outputFormat, 0, sizeof(AudioStreamBasicDescription));
    outputFormat.mFormatID = kAudioFormatMPEG4AAC;
    outputFormat.mSampleRate = fs;
    outputFormat.mChannelsPerFrame = numChannels;
    
    // Use AudioFormat API to fill out the rest of the description.
    size = sizeof(outputFormat);
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outputFormat);
    assert(result == 0);
    
    // Make a destination audio file with this output format.
    ExtAudioFileRef destAudioFile;
    CFStringRef destPathStr = CFStringCreateWithCString(NULL, filePath, kCFStringEncodingUTF8);
    CFURLRef destUrl = CFURLCreateWithFileSystemPath(NULL,
                                                     destPathStr,
                                                     kCFURLPOSIXPathStyle,
                                                     0);

    result = ExtAudioFileCreateWithURL(destUrl,
                                                kAudioFileM4AType,
                                                &outputFormat,
                                                NULL,
                                                kAudioFileFlags_EraseFile,
                                                &destAudioFile);
    assert(result == 0);
    
    
    // Set the client format in source and destination file.
    size = sizeof(sourceFormat);
    result = ExtAudioFileSetProperty(destAudioFile, kExtAudioFileProperty_ClientDataFormat, size, &sourceFormat);
    assert(result == 0);
    

    // Make a buffer
    int bufferSizeInFrames = 8000;
    int bufferSize = (bufferSizeInFrames * sourceFormat.mBytesPerFrame);
    UInt8 * buffer = (UInt8 *)malloc(bufferSize);
    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0].mNumberChannels = sourceFormat.mChannelsPerFrame;
    bufferList.mBuffers[0].mData = buffer;
    bufferList.mBuffers[0].mDataByteSize = (bufferSize);
    
    int frame = 0;
    while( TRUE )
    {
        // Try to fill the buffer to capacity.
        int i = 0;
        for (i = 0; i < bufferSizeInFrames; i++)
        {
            if (frame == testSignalLength)
            {
                break;
            }
            
            float* tb = (float*)bufferList.mBuffers[0].mData;
            tb[i] = testSignal[frame];
            frame++;
            bufferList.mBuffers[0].mDataByteSize = 4 * i + 4;
            
        }

        
        // Write.
        OSStatus wr = ExtAudioFileWrite(destAudioFile, i, &bufferList);
        assert(wr == 0);
        
        if (frame == testSignalLength)
        {
            break;
        }
    }
    
    free( buffer );

    
    // Close the files.
    ExtAudioFileDispose( destAudioFile );
    return DR_NO_ERROR;
}

drError driOSAACEncoder_writeCallback(void* encoderData, int numChannels, int numFrames, float* buffer, int* numBytesWritten)
{
    
    return DR_NO_ERROR;
}

drError driOSAACEncoder_stopCallback(void* encoderData)
{
    return DR_NO_ERROR;
}