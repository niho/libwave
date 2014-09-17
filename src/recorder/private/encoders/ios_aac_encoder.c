#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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

static void ensureNoAudioFileError(OSStatus result)
{
    if (result == noErr)
    {
        return;
    }
    
    switch (result)
    {
        case kAudioFileUnspecifiedError://	'wht?':
        {
            assert(0 && "An unspecified error has occurred.");
            break;
        }
        case kAudioFileUnsupportedFileTypeError://	‘typ?'
        {
            assert(0 && "The file type is not supported.:");
            break;
        }
        case kAudioFileUnsupportedDataFormatError://	''fmt?'
        {
            assert(0 && "The data format is not supported by this file type.:");
            break;
        }
        case kAudioFileInvalidChunkError://	'chk?':
        {
            assert(0 && "Either the chunk does not exist in the file or it is not supported by the file.");
            break;
        }
        case kAudioFileNotOptimizedError: //'optm'
        {
            assert(0 && "The chunks following the audio data chunk are preventing the extension of the audio data chunk. To write more data, you must optimize the file.");
            break;
        }
        case kAudioFilePermissionsError:// 'prm?'
        {
            assert(0 && "The operation violated the file permissions. For example, an attempt was made to write to a file opened with the kAudioFileReadPermission constant.");
            break;
        }
        case kAudioFileBadPropertySizeError://	'!siz':
        {
            assert(0 && "The size of the property data was not correct.");
            break;
        }
        case kAudioFileUnsupportedPropertyError: //'pty?'
        {
            assert(0 && "The property is not supported.");
            break;
        }
        case kAudioFileDoesNotAllow64BitDataSizeError://	'off?':
        {
            assert(0 && "The file offset was too large for the file type. The AIFF and WAVE file format types have 32-bit file size limits.");
            break;
        }
        case kAudioFileInvalidPacketOffsetError://	'pck?':
        {
            assert(0 && "A packet offset was past the end of the file, or not at the end of the file when a VBR format was written, or a corrupt packet size was read when the packet table was built.");
            break;
        }
        case kAudioFileInvalidFileError://	‘dta?':
        {
            assert(0 && "The file is malformed, or otherwise not a valid instance of an audio file of its type.");
            break;
        }
        case kAudioFileOperationNotSupportedError: //0x6F703F3F
        {
            assert(0 && "The operation cannot be performed. For example, setting the kAudioFilePropertyAudioDataByteCount constant to increase the size of the audio data in a file is not a supported operation. Write the data instead.");
            break;
        }
        case kAudioFileNotOpenError: //-38
        {
            assert(0 && "The file is closed.");
            break;
        }
        case kAudioFileEndOfFileError: //-39
        {
            assert(0 && "End of file.");
            break;
        }
        case kAudioFilePositionError://	-40
        {
            assert(0 && "Invalid file position.");
            break;
        }
        case kAudioFileFileNotFoundError: //
        {
            assert(0 && "kAudioFileFileNotFoundError");
            break;
        }
            
        default:
            break;
    }
    
    assert(0 && "unknown error");
}


/*!
 @typedef	AudioFile_ReadProc
 @abstract   A callback for reading data. used with AudioFileOpenWithCallbacks or AudioFileInitializeWithCallbacks.
 @discussion a function that will be called when AudioFile needs to read data.
 @param      inClientData	A pointer to the client data as set in the inClientData parameter to AudioFileXXXWithCallbacks.
 @param      inPosition		An offset into the data from which to read.
 @param      requestCount	The number of bytes to read.
 @param      buffer			The buffer in which to put the data read.
 @param      actualCount		The callback should set this to the number of bytes successfully read.
 @result						The callback should return noErr on success, or an appropriate error code on failure.
 */
static OSStatus audioFileReadProc(void* inClientData,
                                  SInt64 inPosition,
                                  UInt32 requestCount,
                                  void* buffer,
                                  UInt32* actualCount)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)inClientData;
    
    size_t pos = ftell(encoder->file);
    if (pos != inPosition)
    {
        fseek(encoder->file, (long)inPosition, SEEK_SET);
    }
    
    int bytesRead = fread(buffer, 1, requestCount, encoder->file);
    *actualCount = bytesRead < 0 ? 0 : bytesRead;
    
    return noErr;
}

/*!
 @typedef	AudioFile_WriteProc
 @abstract   A callback for writing data. used with AudioFileOpenWithCallbacks or AudioFileInitializeWithCallbacks.
 @discussion a function that will be called when AudioFile needs to write data.
 @param      inClientData	A pointer to the client data as set in the inClientData parameter to AudioFileXXXWithCallbacks.
 @param      inPosition		An offset into the data from which to read.
 @param      requestCount	The number of bytes to write.
 @param      buffer			The buffer containing the data to write.
 @param      actualCount		The callback should set this to the number of bytes successfully written.
 @result						The callback should return noErr on success, or an appropriate error code on failure.
 */
static OSStatus audioFileWriteProc(void* inClientData,
                                   SInt64 inPosition,
                                   UInt32 requestCount,
                                   const void* buffer,
                                   UInt32* actualCount)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)inClientData;
    
    size_t pos = ftell(encoder->file);
    if (pos != inPosition)
    {
        fseek(encoder->file, (long)inPosition, SEEK_SET);
    }
    
    int bytesWritten = fwrite(buffer, 1, requestCount, encoder->file);
    *actualCount = bytesWritten < 0 ? 0 : bytesWritten;
    fflush(encoder->file);
    
    return noErr;
}

/*!
 @typedef	AudioFile_GetSizeProc
 @abstract   A callback for getting the size of the file data. used with AudioFileOpenWithCallbacks or AudioFileInitializeWithCallbacks.
 @discussion a function that will be called when AudioFile needs to determine the size of the file data. This size is for all of the
 data in the file, not just the audio data.
 @param      inClientData	A pointer to the client data as set in the inClientData parameter to AudioFileXXXWithCallbacks.
 @result						The callback should return the size of the data.
 */
static SInt64 audioFileGetSizeProc(void* inClientData)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)inClientData;
    
    size_t pos = ftell(encoder->file);
    fseek(encoder->file, 0, SEEK_END);
    size_t size = ftell(encoder->file);
    fseek(encoder->file, SEEK_SET, pos);
    
    return size;
}

/*!
 @typedef	AudioFile_SetSizeProc
 @abstract   A callback for setting the size of the file data. used with AudioFileOpenWithCallbacks or AudioFileInitializeWithCallbacks.
 @discussion a function that will be called when AudioFile needs to set the size of the file data. This size is for all of the
 data in the file, not just the audio data. This will only be called if the file is written to.
 @param      inClientData	A pointer to the client data as set in the inClientData parameter to AudioFileXXXWithCallbacks.
 @result						The callback should return the size of the data.
 */
static OSStatus audioFileSetSizeProc(void* inClientData,
                                     SInt64 inSize)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)inClientData;
    return 0;
}

drError driOSAACEncoder_initCallback(void* encoderData, const char* filePath, float fs, float numChannels)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)encoderData;
    
    FILE* f = fopen(filePath, "r");
    const int fileExists = f != 0;
    fclose(f);
    
    if (fileExists)
    {
        //try opening the file
        //https://developer.apple.com/library/ios/qa/qa1676/_index.html
        encoder->file = fopen(filePath, "a+");
        OSStatus openResult = AudioFileOpenWithCallbacks(encoder,
                                                         audioFileReadProc,
                                                         audioFileWriteProc,
                                                         audioFileGetSizeProc,
                                                         audioFileSetSizeProc,
                                                         kAudioFileCAFType,
                                                         &encoder->destAudioFile);
        ensureNoAudioFileError(openResult);
        if (openResult != 0)
        {
            return DR_FAILED_TO_INITIALIZE_ENCODER;
        }
    }
    else
    {
        // Define the output format (AAC).
        encoder->file = fopen(filePath, "w");
        AudioStreamBasicDescription outputFormat;
        memset(&outputFormat, 0, sizeof(AudioStreamBasicDescription));
        outputFormat.mFormatID = kAudioFormatMPEG4AAC;
        outputFormat.mSampleRate = fs;
        outputFormat.mChannelsPerFrame = numChannels;
        
        OSStatus initResult = AudioFileInitializeWithCallbacks(encoder,
                                                               audioFileReadProc,
                                                               audioFileWriteProc,
                                                               audioFileGetSizeProc,
                                                               audioFileSetSizeProc,
                                                               kAudioFileCAFType,
                                                               &outputFormat,
                                                               0,
                                                               &encoder->destAudioFile);
        
        ensureNoAudioFileError(initResult);
    }
    
    
    
    /*
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
    
    
    

    // Use AudioFormat API to fill out the rest of the description.
    size = sizeof(outputFormat);
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outputFormat);
    assert(result == 0);
    
    // Make a destination audio file with this output format.
    
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

    
    */
    return DR_NO_ERROR;
}

drError driOSAACEncoder_writeCallback(void* encoderData,
                                      int numChannels,
                                      int numFrames,
                                      float* buffer,
                                      int* numBytesWritten)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)encoderData;
    const int numBytes = numChannels * numFrames * sizeof(float);
    AudioStreamPacketDescription d;
    d.mDataByteSize = numBytes;
    d.mStartOffset = 0;
    d.mVariableFramesInPacket = 0;
    UInt32 nPackets = 1;
    OSStatus writeResult = AudioFileWritePackets(encoder->destAudioFile,
                                                 0,
                                                 numBytes,
                                                 NULL,
                                                 0,
                                                 &nPackets,
                                                 buffer);
    ensureNoAudioFileError(writeResult);
    return DR_NO_ERROR;
}

drError driOSAACEncoder_stopCallback(void* encoderData)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)encoderData;
    OSStatus closeResult = AudioFileClose(encoder->destAudioFile);
    ensureNoAudioFileError(closeResult);
    
    return DR_NO_ERROR;
}