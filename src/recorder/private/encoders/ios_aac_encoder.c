#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "assert.h"
#include "error_codes.h"
#include "mem.h"

#include "ios_aac_encoder.h"

//TODO: remove
//http://stackoverflow.com/questions/4956338/how-can-i-get-aac-encoding-with-extaudiofile-on-ios-to-work?rq=1
//http://stackoverflow.com/questions/12828159/using-extaudiofilewrite-to-write-at-the-end-of-a-file

/*
static float* generateSine(int fs, int f, int nSamples)
{
    float* s = (float*)malloc(nSamples * sizeof(float));
    for (int i = 0; i < nSamples; i++)
    {
        s[i] = sinf(2 * M_PI * f * i / (float)fs);
    }
    
    return s;
}*/

#define kNoMorePCMDataErrorCode (0x666beef)

static void ensureNoAudioConverterError(OSStatus result)
{
    if (result == noErr)
    {
        return;
    }
    
    switch (result)
    {
        case kAudioConverterErr_FormatNotSupported:
        {
            assert(0 && "kAudioConverterErr_FormatNotSupported");
            break;
        }
        case kAudioConverterErr_OperationNotSupported:
        {
            assert(0 && "kAudioConverterErr_OperationNotSupported");
            break;
        }
        case kAudioConverterErr_PropertyNotSupported:
        {
            assert(0 && "kAudioConverterErr_PropertyNotSupported");
            break;
        }
        case kAudioConverterErr_InvalidInputSize:
        {
            assert(0 && "kAudioConverterErr_InvalidInputSize");
            break;
        }
        case kAudioConverterErr_InvalidOutputSize:
        {
            // e.g. byte size is not a multiple of the frame size
            assert(0 && "kAudioConverterErr_InvalidOutputSize");
            break;
        }
        case kAudioConverterErr_UnspecifiedError:
        {
            assert(0 && "kAudioConverterErr_UnspecifiedError");
            break;
        }
        case kAudioConverterErr_BadPropertySizeError:
        {
            assert(0 && "kAudioConverterErr_BadPropertySizeError");
            break;
        }
        case kAudioConverterErr_RequiresPacketDescriptionsError:
        {
            assert(0 && "kAudioConverterErr_RequiresPacketDescriptionsError");
            break;
        }
        case kAudioConverterErr_InputSampleRateOutOfRange:
        {
            assert(0 && "kAudioConverterErr_InputSampleRateOutOfRange");
            break;
        }
        case kAudioConverterErr_OutputSampleRateOutOfRange:
        {
            assert(0 && "kAudioConverterErr_OutputSampleRateOutOfRange");
            break;
        }
        
        case kAudioConverterErr_HardwareInUse:
        {
            /*
             iOS only: Returned from AudioConverterFillComplexBuffer if the underlying hardware codec has
             become unavailable, probably due to an interruption. In this case, your application
             must stop calling AudioConverterFillComplexBuffer. If the converter can resume from an
             interruption (see kAudioConverterPropertyCanResumeFromInterruption), you must
             wait for an EndInterruption notification from AudioSession, and call AudioSessionSetActive(true)
             before resuming.
             */
            assert(0 && "kAudioConverterErr_HardwareInUse");
            break;
        }
        case kAudioConverterErr_NoHardwarePermission:
        {
            /*
             iOS only: Returned from AudioConverterNew if the new converter would use a hardware codec
             which the application does not have permission to use.
             */
            assert(0 && "kAudioConverterErr_NoHardwarePermission");
            break;
        }
        case -50:
        {
            assert(0 && "param error");
            break;
        }
        default:
        {
            break;
        }
    }
    
    assert(0 && "unknown AudioConverter error");
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
    
    assert(0 && "unknown AudioFile error");
}

//-----------------------------------------------------------------------------
/*!
 @typedef    AudioConverterComplexInputDataProc
 @abstract   Callback function for supplying input data to AudioConverterFillComplexBuffer.
 
 @param      inAudioConverter
 The AudioConverter requesting input.
 @param      ioNumberDataPackets
 On entry, the minimum number of packets of input audio data the converter
 would like in order to fulfill its current FillBuffer request. On exit, the
 number of packets of audio data actually being provided for input, or 0 if
 there is no more input.
 @param      ioData
 On exit, the members of ioData should be set to point to the audio data
 being provided for input.
 @param      outDataPacketDescription
 If non-null, on exit, the callback is expected to fill this in with
 an AudioStreamPacketDescription for each packet of input data being provided.
 @param      inUserData
 The inInputDataProcUserData parameter passed to AudioConverterFillComplexBuffer().
 @result     An OSStatus result code.
 
 @discussion
 This callback function supplies input to AudioConverterFillComplexBuffer.
 
 The AudioConverter requests a minimum number of packets (*ioNumberDataPackets).
 The callback may return one or more packets. If this is less than the minimum,
 the callback will simply be called again in the near future.
 
 The callback manipulates the members of ioData to point to one or more buffers
 of audio data (multiple buffers are used with non-interleaved PCM data). The
 callback is responsible for not freeing or altering this buffer until it is
 called again.
 
 If the callback returns an error, it must return zero packets of data.
 AudioConverterFillComplexBuffer will stop producing output and return whatever
 output has already been produced to its caller, along with the error code. This
 mechanism can be used when an input proc has temporarily run out of data, but
 has not yet reached end of stream.
 */
static OSStatus audioConverterComplexInputDataProc(AudioConverterRef inAudioConverter,
                                                   UInt32* ioNumberDataPackets,
                                                   AudioBufferList* ioData,
                                                   AudioStreamPacketDescription** outDataPacketDescription,
                                                   void* inUserData)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)inUserData;
    assert(!outDataPacketDescription);
    
    if (encoder->numPcmFramesLeftToDeliverToEncoder == 0)
    {
        printf("input callback: no more PCM packets to deliver to encoder\n");
        *ioNumberDataPackets = 0;
        return kNoMorePCMDataErrorCode;
    }
    
    int numPacketsToDeliver = *ioNumberDataPackets;
    if (numPacketsToDeliver > encoder->numPcmFramesLeftToDeliverToEncoder)
    {
        numPacketsToDeliver = encoder->numPcmFramesLeftToDeliverToEncoder;
    }
    
    
    printf("input callback: %d pcm packets requested, %d delivered\n",
           *ioNumberDataPackets,
           numPacketsToDeliver);
    
    *ioNumberDataPackets = numPacketsToDeliver;
    ioData->mNumberBuffers = 1;
    const int nChannels = 1;//TODO
    ioData->mBuffers[0].mNumberChannels = nChannels;
    ioData->mBuffers[0].mDataByteSize = nChannels * (*ioNumberDataPackets) * sizeof(float);
    ioData->mBuffers[0].mData = encoder->pcmBuffer;
    
    encoder->numPcmFramesLeftToDeliverToEncoder -= numPacketsToDeliver;
    
    assert(encoder->numPcmFramesLeftToDeliverToEncoder >= 0);
    
    return noErr;
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
    
    //pcm scratch buffer
    const int scratchBufferSize = numChannels * DR_AAC_PCM_BUFFER_SIZE_IN_FRAMES * sizeof(float);
    encoder->pcmBuffer = (float*)DR_MALLOC(scratchBufferSize, "AAC scratch buffer");
    memset(encoder->pcmBuffer, 0, scratchBufferSize);
    
    //input format
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
    
    //output format
    AudioStreamBasicDescription outputFormat;
    memset(&outputFormat, 0, sizeof(AudioStreamBasicDescription));
    outputFormat.mFormatID = kAudioFormatMPEG4AAC;
    outputFormat.mSampleRate = fs;
    outputFormat.mChannelsPerFrame = numChannels;
    //use AudioFormat API to fill out the rest of the description
    UInt32 size = sizeof(outputFormat);
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo,
                                             0,
                                             NULL,
                                             &size,
                                             &outputFormat);
    assert(result == 0);
    
    //check if the target file exists. assume it does't if it cant be opened
    //TODO: a better check?
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
    
    //create audio converter
    OSStatus createResult = AudioConverterNew(&sourceFormat,
                                              &outputFormat,
                                              &encoder->audioConverter);
    ensureNoAudioConverterError(createResult);
    
    //set converter bit rate
    const UInt32 outputBitRate = 64000;
    OSStatus setBitrateResult = AudioConverterSetProperty(encoder->audioConverter,
                                                          kAudioConverterEncodeBitRate,
                                                          sizeof(outputBitRate),
                                                          &outputBitRate);
    ensureNoAudioConverterError(setBitrateResult);
    
    //get the maximum output packet size and allocate output buffers accordingly
    UInt32 maxOutputPacketSize = 0;
    UInt32 propSize = sizeof(maxOutputPacketSize);
    OSStatus maxSizeResult = AudioConverterGetProperty(encoder->audioConverter,
                                                       kAudioConverterPropertyMaximumOutputPacketSize,
                                                       &propSize,
                                                       &maxOutputPacketSize);
    ensureNoAudioConverterError(maxSizeResult);
    assert(maxOutputPacketSize > 0 && "maximum AAC output packet size should be positive");
    encoder->aacOutputBuffer = (unsigned char*)DR_MALLOC(DR_AAC_OUTPUT_BUFFER_SIZE, "AAC output buffer");
    memset(encoder->aacOutputBuffer, 0, DR_AAC_OUTPUT_BUFFER_SIZE);
    encoder->maxNumOutputPackets = DR_AAC_OUTPUT_BUFFER_SIZE / maxOutputPacketSize;
    encoder->aacOutputPacketDescriptions =
            (AudioStreamPacketDescription*)DR_MALLOC(sizeof(AudioStreamPacketDescription) *
                                                     encoder->maxNumOutputPackets, "AAC packet descriptions");
    memset(encoder->aacOutputPacketDescriptions,
           0,
           sizeof(AudioStreamPacketDescription) * encoder->maxNumOutputPackets);

    return DR_NO_ERROR;
}

drError driOSAACEncoder_writeCallback(void* encoderData,
                                      int numChannels,
                                      int numFrames,
                                      float* buffer,
                                      int* numBytesWritten)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)encoderData;
    //TODO: optimize these loops
    
    //accumulate pcm data in the scratch buffer and do encoding
    //when it's full
    for (int i = 0; i < numFrames; i++)
    {
        for (int c = 0; c < numChannels; c++)
        {
            encoder->pcmBuffer[encoder->pcmBufferWritePos * numChannels + c] = buffer[i * numChannels + c];
        }
        
        encoder->pcmBufferWritePos++;
        if (encoder->pcmBufferWritePos == DR_AAC_PCM_BUFFER_SIZE_IN_FRAMES)
        {
            printf("accumulated %d pcm frames, about to convert to AAC\n",
                   encoder->pcmBufferWritePos);
            
            //convert audio data
            AudioBufferList fillBufList;
            fillBufList.mNumberBuffers = 1;
            fillBufList.mBuffers[0].mNumberChannels = numChannels;
            fillBufList.mBuffers[0].mDataByteSize = DR_AAC_OUTPUT_BUFFER_SIZE;
            fillBufList.mBuffers[0].mData = encoder->aacOutputBuffer;
            
            UInt32 numPackets = encoder->maxNumOutputPackets;
            encoder->numPcmFramesLeftToDeliverToEncoder = DR_AAC_PCM_BUFFER_SIZE_IN_FRAMES;
            OSStatus fillResult = AudioConverterFillComplexBuffer(encoder->audioConverter,
                                                                  audioConverterComplexInputDataProc,
                                                                  encoder,
                                                                  &numPackets,
                                                                  &fillBufList,
                                                                  encoder->aacOutputPacketDescriptions);
            if (fillResult == kNoMorePCMDataErrorCode)
            {
                
            }
            else
            {
                ensureNoAudioConverterError(fillResult);
            }
            printf("converted pcm to %d packets of AAC", numPackets);
            
            //write packets
            if (numPackets > 0)
            {
                UInt32 inNumBytes = fillBufList.mBuffers[0].mDataByteSize;
                printf("about to write %d bytes of AAC data\n", inNumBytes);
                OSStatus writeResult = AudioFileWritePackets(encoder->destAudioFile,
                                                             0,
                                                             inNumBytes,
                                                             encoder->aacOutputPacketDescriptions,
                                                             encoder->outputFilePos,
                                                             &numPackets,
                                                             encoder->aacOutputBuffer);
                ensureNoAudioFileError(writeResult);
                
                encoder->outputFilePos += numPackets;
            }
            
            //reset frame counter
            encoder->pcmBufferWritePos = 0;
        }
    }
    
    return DR_NO_ERROR;
}

drError driOSAACEncoder_stopCallback(void* encoderData)
{
    driOSAACEncoder* encoder = (driOSAACEncoder*)encoderData;
    
    OSStatus closeResult = AudioFileClose(encoder->destAudioFile);
    ensureNoAudioFileError(closeResult);
    
    OSStatus disposeResult = AudioConverterDispose(encoder->audioConverter);
    ensureNoAudioConverterError(disposeResult);
    
    DR_FREE(encoder->pcmBuffer);
    
    //TODO: close file
    
    return DR_NO_ERROR;
}
