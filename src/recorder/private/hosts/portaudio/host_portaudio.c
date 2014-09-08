
#include <include/portaudio.h>
#include "instance.h"

PaStream *stream;

/**
 * This method gets called by PortAudio when it's 
 * time to fill another output buffer.
 * @param inputBuffer
 * @param outputBuffer The buffer to write to.
 * @param framesPerBuffer The number of frames per buffer.
 * @param timeInfo 
 * @param statusFlags 
 * @param userData The user data passed to PortAudio during initialization. 
 *                 In this case a pointer to a kwlMixer struct.
 * @return Non-zero if an error occured, zero otherwise.
 */ 
static int paCallback(const void *inputBuffer, 
                      void *outputBuffer,
                      long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData)
{
    drInstance *instance = (drInstance*)userData;
    
    /*Get the number of output channels.*/
    const int numOutChannels = instance->settings.;
    
    /*Fill the output buffer. For efficiency reasons, the 
      mixer has an internal maximum buffer size determined 
      by KWL_TEMP_BUFFER_SIZE_IN_FRAMES. If the size of the 
      requested output buffer exceeds the internal buffer 
      size, multiple calls to kwlMixer_render 
      are made. This is typically not the case.*/
    int currFrame = 0;
    while (currFrame < framesPerBuffer)
    {
        /*Compute the number of frames to mix.*/
        int numFramesToMix = framesPerBuffer - currFrame;
        if (numFramesToMix > KWL_TEMP_BUFFER_SIZE_IN_FRAMES)
        {
            numFramesToMix = KWL_TEMP_BUFFER_SIZE_IN_FRAMES;
        }
        
        /*Perform mixing.*/
        
        drInstance_audioOutputCallback(<#drInstance *in#>, <#float *inBuffer#>, <#int numChannels#>, <#int numFrames#>, <#void *data#>)
        
        kwlMixer_render(mixer, 
                                          mixer->outBuffer, 
                                          numFramesToMix);
        
        /*Copy mixed samples to the output buffer*/
        int outIdx = currFrame * numOutChannels;
        
        memcpy(&((float*)outputBuffer)[outIdx],
               mixer->outBuffer,
               sizeof(float) * numOutChannels * numFramesToMix);
        
        kwlMixer_processInputBuffer(mixer, 
                                    &((float*)inputBuffer)[outIdx],
                                    numFramesToMix);
        
        /*Increment write position*/
        currFrame += numFramesToMix;
    }
    
    /*Return 0 to indicate that everything went well.*/
    return 0;
}

/** 
 * Initializes PortAudio.
 * @param engine
 * @param sampleRate
 * @param numChannels
 * @param bufferSize
 * @return An error code.
 */
drError kwlEngine_hostSpecificInitialize(drInstance* instance, int sampleRate, int numOutChannels, int numInChannels, int bufferSize)
{
    PaError err = Pa_Initialize();
    assert(err == paNoError && "error initializing portaudio");
    
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream(&stream,
                               numInChannels,
                               numOutChannels,
                               paFloat32,   /* 32 bit floating point output. */
                               sampleRate,
                               bufferSize, /* frames per buffer, i.e. the number
                                            of sample frames that PortAudio will
                                            request from the callback. Many apps
                                            may want to use
                                            paFramesPerBufferUnspecified, which
                                            tells PortAudio to pick the best,
                                            possibly changing, buffer size.*/
                               (PaStreamCallback*)&paCallback,
                               engine->mixer);
    //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    assert(err == paNoError);
    
    err = Pa_StartStream(stream);
    //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    assert(err == paNoError);

	const PaStreamInfo* si = Pa_GetStreamInfo(stream);

    return KWL_NO_ERROR;
}

/**
 * Shuts down PortAudio.
 * @param engine
 */
drError kwlEngine_hostSpecificDeinitialize(drInstance* instance)
{
    
    PaError err = Pa_StopStream(stream);
    //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    assert(err == paNoError);
    
    err = Pa_CloseStream(stream);
    //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    assert(err == paNoError);
    
    err = Pa_Terminate();
    //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    assert(err == paNoError);
    return KWL_NO_ERROR;
}
