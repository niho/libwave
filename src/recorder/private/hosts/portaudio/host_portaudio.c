#include <assert.h>
#include <portaudio.h>
#include <string.h>
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
    WaveInstance *instance = (WaveInstance*)userData;
    
    
    wave_instance_audio_output_callback(instance, outputBuffer, instance->numOutputChannels, (int)framesPerBuffer);
    
    wave_instance_audio_input_callback(instance, inputBuffer, instance->numInputChannels, (int)framesPerBuffer);
    
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
WaveError wave_instance_host_specific_init(WaveInstance* instance)
{
    WaveError result = WAVE_NO_ERROR;
    
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        result = WAVE_FAILED_TO_INITIALIZE_HOST;
    }
    
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream(&stream,
                               instance->settings.desiredNumInputChannels,
                               instance->settings.desiredNumOutputChannels,
                               paFloat32,   /* 32 bit floating point output. */
                               instance->settings.desiredSampleRate,
                               instance->settings.desiredBufferSizeInFrames, /* frames per buffer, i.e. the number
                                            of sample frames that PortAudio will
                                            request from the callback. Many apps
                                            may want to use
                                            paFramesPerBufferUnspecified, which
                                            tells PortAudio to pick the best,
                                            possibly changing, buffer size.*/
                               (PaStreamCallback*)&paCallback,
                               instance);
    if (err != paNoError)
    {
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        result = WAVE_FAILED_TO_INITIALIZE_HOST;
    }
    
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        result = WAVE_FAILED_TO_INITIALIZE_HOST;
    }

    if (result == WAVE_NO_ERROR)
    {
        const PaStreamInfo* si = Pa_GetStreamInfo(stream);
        instance->numInputChannels = instance->settings.desiredNumInputChannels;
        instance->numOutputChannels = instance->settings.desiredNumOutputChannels;
        instance->sampleRate = si->sampleRate;
    }

    return result;
}

/**
 * Shuts down PortAudio.
 * @param engine
 */
WaveError wave_instance_host_specific_deinit(WaveInstance* instance)
{
    WaveError result = WAVE_NO_ERROR;
    
    PaError err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        result = WAVE_FAILED_TO_DEINITIALIZE_HOST;
    }
    
    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        result = WAVE_FAILED_TO_DEINITIALIZE_HOST;
    }
    
    err = Pa_Terminate();
    if (err != paNoError)
    {
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        result = WAVE_FAILED_TO_DEINITIALIZE_HOST;
    }
    
    return result;
}
