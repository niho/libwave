#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include <assert.h>

#include "opus.h"
#include "mem.h"

#include "instance.h"
#include "wave_recorder.h"
#include "raw_encoder.h"
#include "platform_util.h"


WaveError wave_instance_init(WaveInstance* instance,
                             WaveNotificationCallback notificationCallback,
                             WaveErrorCallback errorCallback,
                             WaveAudioWrittenCallback audioWrittenCallback,
                             void* callbackUserData,
                             WaveSettings* settings)
{
    memset(instance, 0, sizeof(WaveInstance));
    
    //use custom settings if provided.
    if (settings)
    {
        memcpy(&instance->settings, settings, sizeof(WaveSettings));
    }
    else
    {
        wave_settings_init(&instance->settings);
    }
    
    //printf("opus_get_version_string %s\n", opus_get_version_string());
    
    
    //remember which thread created the instance to verify that functions get called
    //from the right threads
    instance->mainThread = thrd_current();
    
    //hook up notification callback
    instance->audioWrittenCallback = audioWrittenCallback;
    instance->notificationCallback = notificationCallback;
    instance->errorCallback = errorCallback;
    instance->callbackUserData = callbackUserData;
    
    wave_create_encoder(&instance->recordingSession.encoder,
                    &instance->settings);
    
    wave_lock_free_fifo_init(&instance->inputAudioDataQueue,
                             instance->settings.recordFIFOCapacity,
                             sizeof(WaveRecordedChunk));
    
    //create error, notification and control event queues
    wave_lock_free_fifo_init(&instance->notificationFIFO,
                             instance->settings.notificationFIFOCapacity,
                             sizeof(WaveNotification));
    
    wave_lock_free_fifo_init(&instance->controlEventFIFO,
                             instance->settings.controlEventFIFOCapacity,
                             sizeof(WaveControlEvent));
    
    wave_lock_free_fifo_init(&instance->errorFIFO,
                             instance->settings.errorFIFOCapacity,
                             sizeof(WaveError));
    
    wave_lock_free_fifo_init(&instance->realTimeDataFifo,
                             instance->settings.realtimeDataFIFOCapacity,
                             sizeof(WaveRealtimeInfo));
    
    
    //create audio analyzers
    assert(instance->settings.desiredNumInputChannels <= MAX_NUM_INPUT_CHANNELS);
    for (int i = 0; i < instance->settings.desiredNumInputChannels; i++)
    {
        wave_level_meter_init(&instance->inputLevelMeters[i],
                          i,
                          instance->settings.desiredSampleRate,
                          instance->settings.levelMeterAttackTime,
                          instance->settings.levelMeterReleaseTime,
                          instance->settings.rmsWindowSizeInSeconds);
        wave_instance_add_input_analyzer(instance,
                                    &instance->inputLevelMeters[i],
                                    wave_level_meter_process_buffer,
                                    wave_level_meter_deinit);
    }
    
    //create level advisor
    wave_level_advisor_init(&instance->levelAdvisor,
                        instance,
                        0,
                        instance->settings.desiredSampleRate);
    wave_instance_add_input_analyzer(instance,
                                &instance->levelAdvisor,
                                wave_level_advisor_process_buffer,
                                wave_level_advisor_deinit);
    
    WaveError initResult = wave_instance_host_specific_init(instance);
    return initResult;
}

WaveError wave_instance_deinit(WaveInstance* instance)
{
    assert(wave_instance_is_on_main_thread(instance));
    //stop the audio system
    WaveError deinitResult = wave_instance_host_specific_deinit(instance);
    
    WaveNotification n;
    n.type = WAVE_DID_SHUT_DOWN;
    wave_instance_invoke_notification_callback(instance, &n);
    
    //clean up analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (instance->inputAnalyzerSlots[i].analyzerData &&
            instance->inputAnalyzerSlots[i].deinitCallback)
        {
            instance->inputAnalyzerSlots[i].deinitCallback(instance->inputAnalyzerSlots[i].analyzerData);
        }
    }
    
    WAVE_FREE(instance->recordingSession.encoder.encoderData);
    
    wave_lock_free_fifo_deinit(&instance->inputAudioDataQueue);
    
    //release event queues
    wave_lock_free_fifo_deinit(&instance->notificationFIFO);
    wave_lock_free_fifo_deinit(&instance->controlEventFIFO);
    wave_lock_free_fifo_deinit(&instance->errorFIFO);
    wave_lock_free_fifo_deinit(&instance->realTimeDataFifo);

    //clear the instance
    memset(instance, 0, sizeof(WaveInstance));
    
    return deinitResult;
}

void wave_instance_update(WaveInstance* instance, float timeStep)
{
    assert(wave_instance_is_on_main_thread(instance));
    
    //update dev info before consuming any events
    instance->devInfo.controlEventFIFOLevel = wave_lock_free_fifo_get_num_elements(&instance->controlEventFIFO) /
    ((float)instance->controlEventFIFO.capacity);
    
    instance->devInfo.notificationFIFOLevel = wave_lock_free_fifo_get_num_elements(&instance->notificationFIFO) /
    ((float)instance->notificationFIFO.capacity);
    
    instance->devInfo.recordFIFOLevel = wave_lock_free_fifo_get_num_elements(&instance->inputAudioDataQueue) /
    ((float)instance->inputAudioDataQueue.capacity);
    
    //get measured levels
    WaveRealtimeInfo l;
    while (wave_lock_free_fifo_pop(&instance->realTimeDataFifo, &l))
    {
        //TODO: number of input channels?
        memcpy(&instance->realtimeInfo, &l, sizeof(WaveRealtimeInfo));
    }
    
    //invoke the error callback for any incoming errors on the main thread
    WaveError e;
    while (wave_lock_free_fifo_pop(&instance->errorFIFO, &e))
    {
        wave_instance_on_main_thread_error(instance, e);
    }
    
    //invoke the event callback for any incoming events on the main thread
    WaveNotification n;
    while (wave_lock_free_fifo_pop(&instance->notificationFIFO, &n))
    {
        wave_instance_on_main_thread_notification(instance, &n);
    }
    
    //pump audio data FIFO after the notifiaction FIFO, to make sure
    //a recording started event arrives before the first audio data.
    WaveRecordedChunk c;
    int errorOccured = 0;
    while (wave_lock_free_fifo_pop(&instance->inputAudioDataQueue, &c))
    {
        //printf("%d frames of recorded %d channel audio arrived on the main thread\n", c.numFrames, c.numChannels);
        if (!errorOccured)
        {
            
            //struct timeval  tv1, tv2;
            //gettimeofday(&tv1, NULL);
            /* stuff to do! */
            
            WaveError writeResult = WAVE_NO_ERROR;
            if (c.numFrames > 0)
            {
                int numBytesWritten = 0;
                writeResult = instance->recordingSession.encoder.writeCallback(instance->recordingSession.encoder.encoderData,
                                                                               c.numChannels,
                                                                               c.numFrames,
                                                                               c.samples,
                                                                               &numBytesWritten);
                
                if (numBytesWritten > 0)
                {
                    instance->audioWrittenCallback(instance->recordingSession.targetFilePath,
                                                   numBytesWritten,
                                                   instance->callbackUserData);
                }
            }
            
            //gettimeofday(&tv2, NULL);
            
            /*printf ("Encoder write callback took = %f seconds\n",
             (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
             (double) (tv2.tv_sec - tv1.tv_sec));*/
            
            if (writeResult != WAVE_NO_ERROR)
            {
                errorOccured = 1;
                wave_instance_on_main_thread_error(instance, writeResult);
                wave_instance_enqueue_control_event_of_type(instance, WAVE_STOP_RECORDING);
            }
            else
            {
                //printf("recorded %d frames on the main thread\n", instance->recordingSession.numRecordedFrames);
                if (c.lastChunk)
                {
                    wave_instance_stop_recording(instance);
                }
            }
        }
    }
}

void wave_instance_audio_input_callback(WaveInstance* in, const float* inBuffer, int numChannels, int numFrames)
{
    //pass audio input to analyzers
    for (int i = 0; i < MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (in->inputAnalyzerSlots[i].analyzerData != NULL)
        {
            in->inputAnalyzerSlots[i].audioCallback(in->inputAnalyzerSlots[i].analyzerData, inBuffer, numChannels, numFrames);
        }
    }
    
    //record, if requested
    if (in->stateAudioThread == WAVE_STATE_RECORDING ||
        in->stateAudioThread == WAVE_STATE_RECORDING_PAUSED)
    {
        WaveRecordedChunk c;
        const int lastBuffer = in->stopRecordingRequested;
        
        const int numSamples = numChannels * numFrames;
        int sampleIdx = 0;
        const int chunkSize = MAX_RECORDED_CHUNK_SIZE;
        const int numChunks = ceil(numSamples / (float)chunkSize);
        int chunkIdx = 0;
        while (sampleIdx < numSamples)
        {
            int samplesLeft = numSamples - sampleIdx;
            if (samplesLeft > chunkSize)
            {
                samplesLeft = chunkSize;
            }
            
            //TODO: this is a double memcpy....
            const int paused = in->stateAudioThread == WAVE_STATE_RECORDING_PAUSED;
            c.numFrames = paused ? 0 : samplesLeft;
            c.numChannels = numChannels;
            c.lastChunk = 0;
            if (lastBuffer && chunkIdx == numChunks - 1)
            {
                c.lastChunk = 1;
            }
            memcpy(c.samples, &inBuffer[sampleIdx], samplesLeft * sizeof(float));
            //printf("sending %d frames of recorded %d channel audio to the main thread\n", samplesLeft, numChannels);
            int pushSuccess = wave_lock_free_fifo_push(&in->inputAudioDataQueue, &c);
            if (pushSuccess == 0)
            {
                in->devInfo.recordFIFOUnderrun = 1;
            }
            else if (in->stateAudioThread == WAVE_STATE_RECORDING)
            {
                in->recordingSession.numRecordedFrames += samplesLeft;
            }
            
            sampleIdx += samplesLeft;
            chunkIdx++;
        }
        
        
        if (in->stopRecordingRequested)
        {
            in->stopRecordingRequested = 0;
            in->stateAudioThread = WAVE_STATE_IDLE;
            wave_instance_enqueue_notification_of_type(in, WAVE_RECORDING_STOPPED);
        }
    }
    
    //update measured levels
    for (int i = 0; i < MAX_NUM_INPUT_CHANNELS; i++)
    {
        WaveRealtimeInfo l;
        
        WaveLevelMeter* m = &in->inputLevelMeters[i];
        
        l.peakLevel = m->peak;
        l.peakLevelEnvelope = m->peakEnvelope;
        l.rmsLevel = m->rmsLevel;
        l.numRecordedSeconds = in->recordingSession.numRecordedFrames / (float)in->settings.desiredSampleRate;
        
        //TODO: edge detect or something?
        l.hasClipped = m->clip;
        
        //TODO: more channels
        wave_lock_free_fifo_push(&in->realTimeDataFifo, &l);
    }
    
    //copy measured levels to the main thread. TODO: pass these using a FIFO
    //memcpy(in->inputLevelsShared, in->inputLevelsAudio, sizeof(WaveRealtimeInfo) * MAX_NUM_INPUT_CHANNELS);
    
    WaveControlEvent e;
    while (wave_lock_free_fifo_pop(&in->controlEventFIFO, &e))
    {
        wave_instance_on_audio_thread_control_event(in, &e);
    }
}

void wave_instance_audio_output_callback(WaveInstance* in, float* inBuffer, int numChannels, int numFrames)
{
    if (in->firstSampleHasPlayed == 0)
    {
        WaveNotification e;
        e.type = WAVE_DID_INITIALIZE;
        wave_instance_enqueue_notification(in, &e);
        in->firstSampleHasPlayed = 1;
    }
    
    for (int i = 0; i < numFrames; i++)
    {
        inBuffer[numChannels * i + 1] = 0.0f;//0.2f * (-1 + 0.0002f * (rand() % 10000));
    }
}

int wave_instance_is_on_main_thread(WaveInstance* instance)
{
    return thrd_equal(thrd_current(), instance->mainThread);
}


int wave_instance_add_input_analyzer(WaveInstance* instance,
                                void* analyzerData,
                                WaveAnalyzerAudioCallback audioCallback,
                                WaveAnalyzerDeinitCallback deinitCallback)
{
    //TODO: make sure this is not called when the audio system is running. or lock this somehow.
    for (int i = 0; MAX_NUM_ANALYZER_SLOTS; i++)
    {
        if (instance->inputAnalyzerSlots[i].analyzerData == NULL)
        {
            instance->inputAnalyzerSlots[i].analyzerData = analyzerData;
            instance->inputAnalyzerSlots[i].audioCallback = audioCallback;
            instance->inputAnalyzerSlots[i].deinitCallback = deinitCallback;
            
            return 0;
        }
    }
    
    //no free slots :(
    return 1;
}

void wave_instance_request_start_recording(WaveInstance* instance, const char* filePath)
{
    
    strncpy(instance->requestedAudioFilePath, filePath, WAVE_MAX_PATH_LEN);
    wave_instance_enqueue_control_event_of_type(instance, WAVE_START_RECORDING);

}

void wave_instance_initiate_recording(WaveInstance* instance)
{
    assert(wave_instance_is_on_main_thread(instance));
    strcpy(instance->recordingSession.targetFilePath, instance->requestedAudioFilePath);
    instance->requestedAudioFilePath[0] = '\0';
    assert(strlen(instance->recordingSession.targetFilePath) > 0);
    WaveError initResult = instance->recordingSession.encoder.initCallback(instance->recordingSession.encoder.encoderData,
                                                    instance->recordingSession.targetFilePath,
                                                    instance->sampleRate,
                                                    instance->settings.desiredNumInputChannels); //TODO
    
    if (initResult != WAVE_NO_ERROR)
    {
        wave_instance_invoke_error_callback(instance, initResult);
    }
    
    printf("wave_instance_initiate_recording\n");
}

void wave_instance_stop_recording(WaveInstance* instance)
{
    assert(wave_instance_is_on_main_thread(instance));
    printf("wave_instance_stop_recording\n");
    
    //clear any queued up audio chunks
    while (!wave_lock_free_fifo_is_empty(&instance->inputAudioDataQueue))
    {
        WaveRecordedChunk c;
        wave_lock_free_fifo_pop(&instance->inputAudioDataQueue, &c);
        
        //TODO: write this chunk!
        //instance->recordingSession.encoder.writeCallback(
    }
    
    instance->recordingSession.encoder.stopCallback(instance->recordingSession.encoder.encoderData);
    
    instance->recordingSession.numRecordedFrames = 0;
}


void wave_instance_invoke_error_callback(WaveInstance* instance, WaveError errorCode)
{
    assert(wave_instance_is_on_main_thread(instance));
    
    if (instance->errorCallback)
    {
        instance->errorCallback(errorCode, instance->callbackUserData);
    }
}

void wave_instance_invoke_notification_callback(WaveInstance* instance, const WaveNotification* notification)
{
    if (instance->notificationCallback)
    {
        instance->notificationCallback(notification, instance->callbackUserData);
    }
}

void wave_instance_enqueue_error(WaveInstance* instance, WaveError error)
{
    assert(!wave_instance_is_on_main_thread(instance));
    int pushSuccess = wave_lock_free_fifo_push(&instance->errorFIFO, &error);
    if (pushSuccess == 0)
    {
        instance->devInfo.errorFIFOUnderrun = 1;
    }
}

void wave_instance_enqueue_notification(WaveInstance* instance, const WaveNotification* notification)
{
    assert(!wave_instance_is_on_main_thread(instance));
    int pushSuccess = wave_lock_free_fifo_push(&instance->notificationFIFO, notification);
    if (pushSuccess == 0)
    {
        instance->devInfo.notificationFIFOUnderrun = 1;
    }
}

void wave_instance_enqueue_notification_of_type(WaveInstance* instance, WaveNotificationType type)
{
    WaveNotification n;
    memset(&n, 0, sizeof(WaveNotification));
    n.type = type;
    wave_instance_enqueue_notification(instance, &n);
}

void wave_instance_enqueue_control_event(WaveInstance* instance, const WaveControlEvent* event)
{
    assert(wave_instance_is_on_main_thread(instance));
    int pushSuccess = wave_lock_free_fifo_push(&instance->controlEventFIFO, (void*)event);
    if (pushSuccess == 0)
    {
        instance->devInfo.controlEventFIFOUnderrun = 1;
    }
}

void wave_instance_enqueue_control_event_of_type(WaveInstance* instance, WaveControlEventType type)
{
    WaveControlEvent e;
    memset(&e, 0, sizeof(WaveControlEvent));
    e.type = type;
    wave_instance_enqueue_control_event(instance, &e);
}

void wave_instance_on_audio_thread_control_event(WaveInstance* instance, const WaveControlEvent* event)
{
    assert(!wave_instance_is_on_main_thread(instance));
    
    switch (event->type)
    {
        case WAVE_START_RECORDING:
        {
            if (instance->stateAudioThread == WAVE_STATE_IDLE)
            {
                //recording requested and we're currently not recording.
                //start recording.
                instance->recordingSession.numRecordedFrames = 0;
                instance->stateAudioThread = WAVE_STATE_RECORDING;
                wave_instance_enqueue_notification_of_type(instance, WAVE_RECORDING_STARTED);
            }
            break;
        }
        case WAVE_PAUSE_RECORDING:
        {
            if (instance->stateAudioThread == WAVE_STATE_RECORDING)
            {
                //recording pause requested and we're currently recording.
                //pause recording.
                instance->stateAudioThread = WAVE_STATE_RECORDING_PAUSED;
                wave_instance_enqueue_notification_of_type(instance, WAVE_RECORDING_PAUSED);
            }
            break;
        }
        case WAVE_RESUME_RECORDING:
        {
            if (instance->stateAudioThread == WAVE_STATE_RECORDING_PAUSED)
            {
                //recording resume requested and we're currently paused.
                //resume recording.
                instance->stateAudioThread = WAVE_STATE_RECORDING;
                wave_instance_enqueue_notification_of_type(instance, WAVE_RECORDING_RESUMED);
            }
            break;
        }
        case WAVE_STOP_RECORDING:
        {
            if (instance->stateAudioThread == WAVE_STATE_RECORDING ||
                instance->stateAudioThread == WAVE_STATE_RECORDING_PAUSED)
            {
                //cancel recording requested and we're currently recording.
                //cancel.
                instance->stopRecordingRequested = 1;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void wave_instance_on_main_thread_error(WaveInstance* instance, WaveError error)
{
    assert(wave_instance_is_on_main_thread(instance));
    
    wave_instance_invoke_error_callback(instance, error);
    
}

void wave_instance_on_main_thread_notification(WaveInstance* instance, const WaveNotification* notification)
{
    assert(wave_instance_is_on_main_thread(instance));
    
    switch (notification->type)
    {
        case WAVE_DID_INITIALIZE:
        {
            break;
        }
        case WAVE_RECORDING_STARTED:
        {
            instance->stateMainThread = WAVE_STATE_RECORDING;
            //initiate recording
            wave_instance_initiate_recording(instance);
            break;
        }
        case WAVE_RECORDING_STOPPED:
        {
            //nothing here. the recording is finished when the last buffer
            //has been received.
            instance->stateMainThread = WAVE_STATE_IDLE;
            break;
        }
        case WAVE_RECORDING_PAUSED:
        {
            instance->stateMainThread = WAVE_STATE_RECORDING_PAUSED;
            break;
        }
        case WAVE_RECORDING_RESUMED:
        {
            instance->stateMainThread = WAVE_STATE_RECORDING;
            break;
        }
        
        default:
        {
            break;
        }
    }
    
    wave_instance_invoke_notification_callback(instance, notification);
}

static float lin2LogLevel(float lin)
{
    //TODO: tweak this
    return powf(lin, 0.3f);
}

void wave_instance_get_realtime_info(WaveInstance* instance, int channel, int logLevels, WaveRealtimeInfo* result)
{
    memset(result, 0, sizeof(WaveRealtimeInfo));
    
    if (channel >= MAX_NUM_INPUT_CHANNELS)
    {
        return;
    }
    
    WaveRealtimeInfo* lSrc = &instance->realtimeInfo;
    
    result->hasClipped = lSrc->hasClipped;
    result->peakLevel = logLevels ? lin2LogLevel(lSrc->peakLevel) : lSrc->peakLevel;
    result->peakLevelEnvelope = logLevels ? lin2LogLevel(lSrc->peakLevelEnvelope) : lSrc->peakLevelEnvelope;
    result->rmsLevel = logLevels ? lin2LogLevel(lSrc->rmsLevel) : lSrc->rmsLevel;
    result->numRecordedSeconds = lSrc->numRecordedSeconds;
}

void wave_instance_get_dev_info(WaveInstance* instance, WaveDevInfo* devInfo)
{
    //copy the info to the caller
    memcpy(devInfo, &instance->devInfo, sizeof(WaveDevInfo));
    
    //clear any flags
    instance->devInfo.recordFIFOUnderrun = 0;
    instance->devInfo.controlEventFIFOUnderrun = 0;
    instance->devInfo.notificationFIFOUnderrun = 0;
    instance->devInfo.errorFIFOUnderrun = 0;
}
