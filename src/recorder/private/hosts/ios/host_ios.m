
#include <assert.h>

#include "instance.h"
#include "host_ios.h"

#include <AudioToolbox/AudioToolbox.h>

AudioStreamBasicDescription outputFormat;
AudioStreamBasicDescription inputFormat;
/** The Remote I/O unit */
AudioComponentInstance auComponentInstance;
/** A buffer list for storing input samples. */
AudioBufferList inputBufferList;
/** The size in bytes of the input sample buffer. */
int inputBufferByteSize = -1;

//TODO: fix this!
static drInstance* s_instance = NULL;

#define DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES 2048
float inputScratchBuffer[DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES];
float outputScratchBuffer[DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES];

/**
 * Converts a buffer of floats to a buffer of signed shorts. The floats are assumed
 * to be in the range [-1, 1].
 * @param sourceBuffer The buffer containing the values to convert.
 * @param targetBuffer The buffer to write converted samples to.
 * @param size The size of the source and target buffers.
 */
static inline void drFloatToInt16(float* sourceBuffer, short* targetBuffer, int size)
{
    assert(sourceBuffer != NULL);
    assert(targetBuffer != NULL);
    
    int i = 0;
    while (i < size)
    {
        targetBuffer[i] = (short)(32767 * sourceBuffer[i]);
        i++;
    }
}

/**
 * Converts a buffer of signed short values to a buffer of floats
 * in the range [-1, 1].
 * @param sourceBuffer The buffer containing the values to convert.
 * @param targetBuffer The buffer to write converted samples to.
 * @param size The size of the source and target buffers.
 */
static inline void drInt16ToFloat(short* sourceBuffer, float* targetBuffer, int size)
{
    assert(sourceBuffer != NULL);
    assert(targetBuffer != NULL);
    
    int i = 0;
    while (i < size)
    {
        targetBuffer[i] = (float)(sourceBuffer[i] / 32768.0);
        i++;
    }
}

drError drInstance_hostSpecificInit(drInstance* instance)
{
    //TODO: uuuh...
    assert(s_instance == NULL);
    s_instance = instance;
    
    /**
     * Create the remote IO instance once.
     */
    drCreateRemoteIOInstance();
    
    /*
     * Initialize the audio session
     */
    drInitAudioSession();
    
    /*
     * Activates audio session and starts RemoteIO unit if successful.
     */
    drResume();
    
    return DR_NO_ERROR;
}

drError drInstance_hostSpecificDeinit(drInstance* instance)
{
    drStopAndDeinitRemoteIO();
    
    AudioComponentInstanceDispose(auComponentInstance);
    
    //TODO: uuuh...
    assert(s_instance != NULL);
    s_instance = NULL;
    
    return DR_NO_ERROR;
}

void drAudioSessionInterruptionCallback(void *inClientData,  UInt32 inInterruptionState)
{
    if (inInterruptionState == kAudioSessionBeginInterruption)
    {
        //printf("* audio session interruption callback: begin interruption\n");
        drSuspend();
    }
    else if (inInterruptionState == kAudioSessionEndInterruption)
    {
        //printf("* audio session interruption callback: end interruption\n");
        drResume();
    }
    else 
    {
        assert(0 && "unknown interruption state");
    }
    //drDebugPrintAudioSessionInfo();
}


void drInputAvailableChangeCallback(void *inUserData,
                              AudioSessionPropertyID inPropertyID,
                              UInt32 inPropertyValueSize,
                              const void *inPropertyValue)
{
    //printf("* input availability changed. availability=%d\n", (*(int*)inPropertyValue));
    //drDebugPrintAudioSessionInfo();
}

OSStatus drCoreAudioInputCallback(void *inRefCon,
                                AudioUnitRenderActionFlags *ioActionFlags,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inBusNumber,
                                UInt32 inNumberFrames,
                                AudioBufferList *ioData)
{
    inputBufferList.mBuffers[0].mDataByteSize = inputBufferByteSize;
    //fill the already allocated input buffer list with samples
    OSStatus status;    
    status = AudioUnitRender(auComponentInstance, 
                             ioActionFlags, 
                             inTimeStamp, 
                             inBusNumber, 
                             inNumberFrames, 
                             &inputBufferList);
    assert(status == 0);
    

    drInstance* instance = (drInstance*)inRefCon;

    const int numChannels = inputBufferList.mBuffers[0].mNumberChannels;
    short* buffer = (short*) inputBufferList.mBuffers[0].mData;
    int currFrame = 0;
    while (currFrame < inNumberFrames)
    {
        int numFramesToMix = inNumberFrames - currFrame;
        if (numFramesToMix > DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES)
        {
            numFramesToMix = DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES;
        }
        
        /*Convert input buffer samples to floats*/
        drInt16ToFloat(&buffer[currFrame * numChannels], 
                        inputScratchBuffer,
                        numFramesToMix * numChannels);
            
        /*Pass the converted buffer to the instance*/
        drInstance_audioInputCallback(instance,
                                      &(inputScratchBuffer)[currFrame * numChannels],
                                      numChannels, numFramesToMix);
        
        currFrame += numFramesToMix;
    }
    
    return noErr;
}

OSStatus drCoreAudioOutputCallback(void *inRefCon,
                                   AudioUnitRenderActionFlags *ioActionFlags,
                                   const AudioTimeStamp *inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumberFrames,
                                   AudioBufferList *ioData)
{   
//#define KWL_DEBUG_CA_DEADLINE
#ifdef KWL_DEBUG_CA_DEADLINE
    static double prevDelta = 0.0;
    static double ht = 0.0;
    double delta = inTimeStamp->mSampleTime - ht;
    ht = inTimeStamp->mSampleTime;
    if (delta > inNumberFrames && prevDelta > 0.0)
    {
        printf("missed deadline, time since prev callback: %f samples, curr buffer size %d samples\n", delta, inNumberFrames);
        //drDebugPrintAudioSessionInfo();
        //drDebugPrintRemoteIOInfo();
    }
    prevDelta = delta;
#endif
    
    drInstance* instance = (drInstance*)inRefCon;
    
    const int numChannels = ioData->mBuffers[0].mNumberChannels;
    short* buffer = (short*) ioData->mBuffers[0].mData;
    int currFrame = 0;
    while (currFrame < inNumberFrames)
    {
        int numFramesToMix = inNumberFrames - currFrame;
        if (numFramesToMix > DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES)
        {
            numFramesToMix = DR_IOS_TEMP_BUFFER_SIZE_IN_FRAMES;
        }
    
        /*prepare a new buffer*/
        drInstance_audioOutputCallback(instance, outputScratchBuffer, numChannels, numFramesToMix);

        drFloatToInt16(outputScratchBuffer,
                        &buffer[currFrame * numChannels], 
                        numFramesToMix * numChannels);
        currFrame += numFramesToMix;
    }
    
    return noErr;
}

void drStopAndDeinitRemoteIO()
{
    OSStatus status = AudioOutputUnitStop(auComponentInstance);
    assert(status == noErr);
    
    status = AudioUnitUninitialize(auComponentInstance);
    assert(status == noErr);
    
    free(inputBufferList.mBuffers[0].mData);
    inputBufferList.mBuffers[0].mData = NULL;
}

void drCreateRemoteIOInstance()
{
    /*create audio component description*/
    AudioComponentDescription auDescription;
    
    auDescription.componentType          = kAudioUnitType_Output;
    auDescription.componentSubType       = kAudioUnitSubType_RemoteIO;
    auDescription.componentManufacturer  = kAudioUnitManufacturer_Apple;
    auDescription.componentFlags         = 0;
    auDescription.componentFlagsMask     = 0;
    
    /*get a component reference*/
    AudioComponent auComponent = AudioComponentFindNext(NULL, &auDescription);
    
    /*get the actual instance*/
    OSStatus status = AudioComponentInstanceNew(auComponent, &auComponentInstance);
    assert(status == noErr);
}



void drInitAndStartRemoteIO()
{
	//make sure the audio unit is not initialized more than once.
	//some of the operations below depend on the unit not being
	//initialized.
    drStopAndDeinitRemoteIO();
    
    const int bitsPerSample = 16; 
    const int numInChannels = s_instance->settings.desiredNumInputChannels;
    const int numOutChannels = s_instance->settings.desiredNumOutputChannels;
    float sampleRate = s_instance->settings.desiredSampleRate;
    
    const unsigned int OUTPUT_BUS_ID = 0;
    const unsigned int INPUT_BUS_ID = 1;
    
    OSStatus status = 0;
    
    /*Enable recording if requested*/
    if (numInChannels > 0)
    {
        UInt32 flag = 1;
        status = AudioUnitSetProperty(auComponentInstance, 
                                      kAudioOutputUnitProperty_EnableIO, 
                                      kAudioUnitScope_Input, 
                                      INPUT_BUS_ID,
                                      &flag, 
                                      sizeof(flag));
        drEnsureNoAudioUnitError(status);
    }
    
    /*set buffer size. */
    Float32 preferredBufferSize = s_instance->settings.desiredBufferSizeInFrames / (float)s_instance->settings.desiredSampleRate;
    status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, 
                                     sizeof(preferredBufferSize), 
                                     &preferredBufferSize);
    drEnsureNoAudioSessionError(status);
    
    Float64 sr = sampleRate;
    status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareSampleRate,
                                     sizeof(Float64),
                                     &sr);
    drEnsureNoAudioSessionError(status);
    
    /*enable playback*/    
    UInt32 flag = 1;
    status = AudioUnitSetProperty(auComponentInstance, 
                                  kAudioOutputUnitProperty_EnableIO, 
                                  kAudioUnitScope_Output, 
                                  OUTPUT_BUS_ID,
                                  &flag, 
                                  sizeof(flag));
    drEnsureNoAudioUnitError(status);
    
    /*set up output audio format description*/
    drSetASBD(&outputFormat, numOutChannels, sampleRate);
    
    /*apply format to output*/
    status = AudioUnitSetProperty(auComponentInstance, 
                                  kAudioUnitProperty_StreamFormat, 
                                  kAudioUnitScope_Input, 
                                  OUTPUT_BUS_ID, 
                                  &outputFormat, 
                                  sizeof(outputFormat));
    drEnsureNoAudioUnitError(status);
    
    /*apply format to input if enabled*/
    if (numInChannels > 0)
    {
        drSetASBD(&inputFormat, numInChannels, sampleRate);
        
        status = AudioUnitSetProperty(auComponentInstance, 
                                      kAudioUnitProperty_StreamFormat, 
                                      kAudioUnitScope_Output, 
                                      INPUT_BUS_ID, 
                                      &inputFormat, 
                                      sizeof(outputFormat));
        drEnsureNoAudioUnitError(status);
        
        int maxSliceSize = 0;
        int s = sizeof(maxSliceSize);
		status = AudioUnitGetProperty(auComponentInstance, 
                                      kAudioUnitProperty_MaximumFramesPerSlice, 
                                      kAudioUnitScope_Global, 
                                      0, 
                                      &maxSliceSize, 
                                      &s);
        drEnsureNoAudioUnitError(status);
        
        inputBufferList.mNumberBuffers = 1;
        inputBufferList.mBuffers[0].mNumberChannels = numInChannels;
        inputBufferByteSize = 2 * numInChannels * maxSliceSize;
        inputBufferList.mBuffers[0].mDataByteSize = inputBufferByteSize;
        inputBufferList.mBuffers[0].mData = malloc(inputBufferList.mBuffers[0].mDataByteSize);
    }
    
    assert(status == noErr);
    
    AURenderCallbackStruct renderCallbackStruct;
    /*hook up the input callback*/
    if (numInChannels > 0)
    {
        renderCallbackStruct.inputProc = drCoreAudioInputCallback;
        renderCallbackStruct.inputProcRefCon = s_instance;
        
        status = AudioUnitSetProperty(auComponentInstance, 
                                      kAudioOutputUnitProperty_SetInputCallback, 
                                      kAudioUnitScope_Global, 
                                      OUTPUT_BUS_ID, 
                                      &renderCallbackStruct, 
                                      sizeof(renderCallbackStruct));
        drEnsureNoAudioUnitError(status);
    }
    
    
    /*hook up the output callback*/
    renderCallbackStruct.inputProc = drCoreAudioOutputCallback;
    renderCallbackStruct.inputProcRefCon = s_instance;
    
    status = AudioUnitSetProperty(auComponentInstance, 
                                  kAudioUnitProperty_SetRenderCallback, 
                                  kAudioUnitScope_Global, 
                                  OUTPUT_BUS_ID,
                                  &renderCallbackStruct, 
                                  sizeof(renderCallbackStruct));
    
    drEnsureNoAudioUnitError(status);
    
    /*init audio unit*/
    status = AudioUnitInitialize(auComponentInstance);
    //printf("status %d\n", status);
    drEnsureNoAudioUnitError(status);
    
    /*start audio unit*/
    status = AudioOutputUnitStart(auComponentInstance);
    //printf("status %d\n", status);
    drEnsureNoAudioUnitError(status);
    
    //TODO: use these to see what buffer size and sample rate we actually got.
    Float32 buffDur;
    int ss = sizeof(Float32);
    AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, 
                            &ss, 
                            &buffDur);
    
    Float64 actualSampleRate;
    ss = sizeof(Float64);
    AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareSampleRate, 
                            &ss, 
                            &actualSampleRate);
    s_instance->sampleRate = actualSampleRate;
}

void drAudioRouteChangeCallback(void *inUserData,
                              AudioSessionPropertyID inPropertyID,
                              UInt32 inPropertyValueSize,
                              const void *inPropertyValue)
{
    //printf("* audio route changed,\n");
    drInstance* instance = (drInstance*)inUserData;
    
    //get the old audio route name and the reason for the change
    CFDictionaryRef dict = inPropertyValue;
    CFStringRef oldRoute = 
        CFDictionaryGetValue(dict, CFSTR(kAudioSession_AudioRouteChangeKey_OldRoute));
    CFNumberRef reason = 
    CFDictionaryGetValue(dict, CFSTR(kAudioSession_AudioRouteChangeKey_Reason));
    int reasonNumber = -1;
    CFNumberGetValue(reason, CFNumberGetType(reason), &reasonNumber);
    
    //reason specific code
    switch (reasonNumber)
    {
        case kAudioSessionRouteChangeReason_Unknown: //0
        {
            //printf("kAudioSessionRouteChangeReason_Unknown\n");
            break;
        }   
        case kAudioSessionRouteChangeReason_NewDeviceAvailable: //1
        {
            //printf("kAudioSessionRouteChangeReason_NewDeviceAvailable\n");
            break;
        }
        case kAudioSessionRouteChangeReason_OldDeviceUnavailable: //2
        {
            //printf("kAudioSessionRouteChangeReason_OldDeviceUnavailable\n");
            break;
        }
        case kAudioSessionRouteChangeReason_CategoryChange: //3
        {
            //printf("kAudioSessionRouteChangeReason_CategoryChange\n");
            break;
        }   
        case kAudioSessionRouteChangeReason_Override: //4
        {
            //printf("kAudioSessionRouteChangeReason_Override\n");
            break;
        }
            // this enum has no constant with a value of 5
        case kAudioSessionRouteChangeReason_WakeFromSleep: //6
        {
            //printf("kAudioSessionRouteChangeReason_WakeFromSleep\n");
            break;
        }
        case kAudioSessionRouteChangeReason_NoSuitableRouteForCategory:
        {
            //printf("kAudioSessionRouteChangeReason_NoSuitableRouteForCategory\n");
            break;
        }
    }
    
    /* 
     From the Apple "Handling Audio Hardware Route Changes" docs:
     
     "One of the audio hardware route change reasons in iOS is 
     kAudioSessionRouteChangeReason_CategoryChange. In other words, 
     a change in audio session category is considered by the system—in 
     this context—to be a route change, and will invoke a route change 
     property listener callback. As a consequence, such a callback—if 
     it is intended to respond only to headset plugging and unplugging—should 
     explicitly ignore this type of route change."
     
     If kAudioSessionRouteChangeReason_CategoryChange is not ignored, we could get 
     an infinite loop because the audio session category is set below, which will in
     turn trigger kAudioSessionRouteChangeReason_CategoryChange and so on.
     */
    if (reasonNumber != kAudioSessionRouteChangeReason_CategoryChange)
    {
        /*
         * Deinit the remote io and set it up again depending on if input is available. 
         */
        UInt32 isAudioInputAvailable; 
        UInt32 size = sizeof(isAudioInputAvailable);
        OSStatus result = AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, 
                                                  &size, 
                                                  &isAudioInputAvailable);
        drEnsureNoAudioSessionError(result);
        
        drStopAndDeinitRemoteIO();
        
        int numInChannels = isAudioInputAvailable != 0 ? s_instance->settings.desiredNumInputChannels : 0;
        UInt32 sessionCategory = numInChannels == 0 ? kAudioSessionCategory_MediaPlayback : 
                                                      kAudioSessionCategory_PlayAndRecord;
        result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
        drEnsureNoAudioSessionError(result);
        
        if (numInChannels > 0)
        {
            int val = 1;
            result = AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryDefaultToSpeaker, 
                                             sizeof(val), 
                                             &val);
            drEnsureNoAudioSessionError(result);
        }

        result = AudioSessionSetActive(true);
        drEnsureNoAudioSessionError(result); //-12986 seems to mean that kAudioSessionCategory_PlayAndRecord was set and no input is available 
        
        drInitAndStartRemoteIO();
    }
}

void drServerDiedCallback(void *inUserData,
                        AudioSessionPropertyID inPropertyID,
                        UInt32 inPropertyValueSize,
                        const void *inPropertyValue)
{
    //printf("server died\n");
}

void drInitAudioSession()
{
    OSStatus status = 0;
    
    /*
     * Initialize and activte audio session
     */
    status = AudioSessionInitialize(NULL, NULL, &drAudioSessionInterruptionCallback, s_instance);
    if (status == kAudioSessionAlreadyInitialized)
    {
        //already initialized
    }
    else
    {
        drEnsureNoAudioSessionError(status);
        assert(status == noErr);
    }
    
    /*
     UInt32 isOtherAudioPlaying = 0;
     UInt32 propertySize = sizeof(isOtherAudioPlaying);
     status = AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &propertySize, &isOtherAudioPlaying);
     assert(status == noErr);
     //printf("other audio playing = %d\n", isOtherAudioPlaying);
     */
    
    //check if audio input is available at all
    
    UInt32 inputAvailable; 
    int propertySize = sizeof(inputAvailable);
    status = AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, &propertySize, &inputAvailable);
    assert(status == noErr);
    
    if (inputAvailable == 0)
    {
        //This device does not support audio input at this point 
        //(this may change at any time, for example when connecting
        //a headset to an iPod touch).
        s_instance->isInputDisabled = 1;
    }
    
    UInt32 sessionCategory = inputAvailable == 0 ? kAudioSessionCategory_MediaPlayback : 
                                                   kAudioSessionCategory_PlayAndRecord;
    status = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
    drEnsureNoAudioSessionError(status);
    
    int val = 1;
    status = AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryDefaultToSpeaker, 
                                     sizeof(val), 
                                     &val);
    
    status = AudioSessionAddPropertyListener(kAudioSessionProperty_AudioInputAvailable, 
                                             &drInputAvailableChangeCallback, 
                                             s_instance);
    drEnsureNoAudioSessionError(status);
    
    status = AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, 
                                             drAudioRouteChangeCallback, 
                                             s_instance);
    drEnsureNoAudioSessionError(status);
    
    status = AudioSessionAddPropertyListener(kAudioSessionProperty_ServerDied, 
                                             drServerDiedCallback, 
                                             s_instance);
    drEnsureNoAudioSessionError(status);
}

void drSuspend(void)
{
    drStopAndDeinitRemoteIO();
    OSStatus result = AudioSessionSetActive(0);
    drEnsureNoAudioSessionError(result);
}

int drResume(void)
{
    OSStatus result = AudioSessionSetActive(1);
    if (result == noErr)
    {
        drInitAndStartRemoteIO();
        return 1;
    }
    
    return 0;
}

void drSetASBD(AudioStreamBasicDescription* asbd, int numChannels, float sampleRate)
{
    memset(asbd, 0, sizeof(AudioStreamBasicDescription));
    assert(numChannels == 1 || numChannels == 2);
    asbd->mBitsPerChannel = 16;
    asbd->mBytesPerFrame = 2 * numChannels;
    asbd->mBytesPerPacket = asbd->mBytesPerFrame;
    asbd->mChannelsPerFrame = numChannels;
    asbd->mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    asbd->mFormatID = kAudioFormatLinearPCM;
    asbd->mFramesPerPacket = 1;
    asbd->mSampleRate = sampleRate;
}

void drEnsureNoAudioUnitError(OSStatus result)
{
    switch (result)
    {
        case kAudioUnitErr_InvalidProperty:
            assert(0 && "kAudioUnitErr_InvalidProperty");
            break;
        case kAudioUnitErr_InvalidParameter:
            assert(0 && "kAudioUnitErr_InvalidParameter");
            break;
        case kAudioUnitErr_InvalidElement:
            assert(0 && "kAudioUnitErr_InvalidElement");
            break;
        case kAudioUnitErr_NoConnection:
            assert(0 && "kAudioUnitErr_NoConnection");
            break;
        case kAudioUnitErr_FailedInitialization:
            assert(0 && "kAudioUnitErr_FailedInitialization");
            break;
        case kAudioUnitErr_TooManyFramesToProcess:
            assert(0 && "kAudioUnitErr_TooManyFramesToProcess");
            break;
        case kAudioUnitErr_InvalidFile:
            assert(0 && "kAudioUnitErr_InvalidFile");
            break;
        case kAudioUnitErr_FormatNotSupported:
            assert(0 && "kAudioUnitErr_FormatNotSupported");
            break;
        case kAudioUnitErr_Uninitialized:
            assert(0 && "kAudioUnitErr_Uninitialized");
            break;
        case kAudioUnitErr_InvalidScope:
            assert(0 && "kAudioUnitErr_InvalidScope");
            break;
        case kAudioUnitErr_PropertyNotWritable:
            assert(0 && "kAudioUnitErr_PropertyNotWritable");
            break;
        case kAudioUnitErr_CannotDoInCurrentContext:
            assert(0 && "kAudioUnitErr_CannotDoInCurrentContext");
            break;
        case kAudioUnitErr_InvalidPropertyValue:
            assert(0 && "kAudioUnitErr_InvalidPropertyValue");
            break;
        case kAudioUnitErr_PropertyNotInUse:
            assert(0 && "kAudioUnitErr_PropertyNotInUse");
            break;
        case kAudioUnitErr_Initialized:
            assert(0 && "kAudioUnitErr_Initialized");
            break;
        case kAudioUnitErr_InvalidOfflineRender:
            assert(0 && "kAudioUnitErr_InvalidOfflineRender");
            break;
        case kAudioUnitErr_Unauthorized:
            assert(0 && "kAudioUnitErr_Unauthorized");
            break;
        default:
            assert(result == noErr);
            break;
    }
}

void drEnsureNoAudioSessionError(OSStatus result)
{
    switch (result) 
    {
        case kAudioSessionNotActiveError:
            assert(0 && "kAudioSessionNotActiveError");
            break;
        case kAudioSessionNotInitialized:
            assert(0 && "kAudioSessionNotInitialized");
            break;
        case kAudioSessionAlreadyInitialized:
            assert(0 && "kAudioSessionAlreadyInitialized");
            break;
        case kAudioSessionInitializationError:
            assert(0 && "kAudioSessionInitializationError");
            break;
        case kAudioSessionUnsupportedPropertyError:
            assert(0 && "kAudioSessionUnsupportedPropertyError");
            break;
        case kAudioSessionBadPropertySizeError:
            assert(0 && "kAudioSessionBadPropertySizeError");
            break;
        case kAudioServicesNoHardwareError:
            assert(0 && "kAudioServicesNoHardwareError");
            break;
        case kAudioSessionNoCategorySet:
            assert(0 && "kAudioSessionNoCategorySet");
            break;
        case kAudioSessionIncompatibleCategory:
            assert(0 && "kAudioSessionIncompatibleCategory");
            break;
        case kAudioSessionUnspecifiedError:
            assert(0 && "kAudioSessionUnspecifiedError");
            break;
        default:
            assert(result == noErr);
            break;
    }
}
    
void drDebugPrintAudioSessionInfo()
{
    int category = -1;
    int numOutChannels = -1;
    int numInChannels = -1;
    
    int propertySize = sizeof(category);
    
    OSStatus status = AudioSessionGetProperty(kAudioSessionProperty_AudioCategory, 
                                              &propertySize, 
                                              &category);
    assert(status == noErr);
    
    status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareInputNumberChannels, 
                                     &propertySize, 
                                     &numInChannels);
    assert(status == noErr);
    
    status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareOutputNumberChannels, 
                                     &propertySize, 
                                     &numOutChannels);
    assert(status == noErr);
    
    printf("    Audio session info:\n");
    printf("        category %d\n", category);
    printf("        n in ch  %d\n", numInChannels);    
    printf("        n out ch %d\n", numOutChannels);
}

void drDebugPrintRemoteIOInfo()
{
    AudioStreamBasicDescription outFmt;
    int sz = sizeof(AudioStreamBasicDescription);
    AudioUnitGetProperty(auComponentInstance, 
                         kAudioUnitProperty_StreamFormat, 
                         kAudioUnitScope_Input, 
                         0,
                         &outFmt, 
                         &sz);
    
    AudioStreamBasicDescription inFmt;
    sz = sizeof(AudioStreamBasicDescription);
    AudioUnitGetProperty(auComponentInstance, 
                         kAudioUnitProperty_StreamFormat, 
                         kAudioUnitScope_Output, 
                         1,
                         &inFmt, 
                         &sz);
    
    printf("    Remote IO info:\n");
    printf("        Input bits/channel %d\n", inFmt.mBitsPerChannel);
    printf("        Input bytes/frame %d\n", inFmt.mBytesPerFrame);
    printf("        Input bytes/packet %d\n", inFmt.mBytesPerPacket);
    printf("        Input channels/frame %d\n", inFmt.mChannelsPerFrame);
    printf("        Input format flags %d\n", inFmt.mFormatFlags);
    printf("        Input format ID %d\n", inFmt.mFormatID);
    printf("        Input frames per packet %d\n", inFmt.mFramesPerPacket);
    printf("        Input sample rate %f\n", inFmt.mSampleRate);
    printf("\n");
    printf("        Output bits/channel %d\n", outFmt.mBitsPerChannel);
    printf("        Output bytes/frame %d\n", outFmt.mBytesPerFrame);
    printf("        Output bytes/packet %d\n", outFmt.mBytesPerPacket);
    printf("        Output channels/frame %d\n", outFmt.mChannelsPerFrame);
    printf("        Output format flags %d\n", outFmt.mFormatFlags);
    printf("        Output format ID %d\n", outFmt.mFormatID);
    printf("        Output frames per packet %d\n", outFmt.mFramesPerPacket);
    printf("        Output sample rate %f\n", outFmt.mSampleRate);
    
}

