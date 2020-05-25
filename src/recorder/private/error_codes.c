#include "wave_error_codes.h"

const char* wave_error_str(WaveError errorCode)
{
    switch (errorCode)
    {
        case WAVE_NO_ERROR:
        {
            return "no error";
        }
        case WAVE_ALREADY_INITIALIZED:
        {
            return "recorder instance is already initialized";
        }
        case WAVE_NOT_INITIALIZED:
        {
            return "recorder instance is not initialized";
        }
        case WAVE_FAILED_TO_INITIALIZE_HOST:
        {
            return "failed to initialize the audio host";
        }
        case WAVE_FAILED_TO_DEINITIALIZE_HOST:
        {
            return "failed to deinitialize the audio host";
        }
        case WAVE_FAILED_TO_INITIALIZE_ENCODER:
        {
            return "failed to initialize audio encoder";
        }
        case WAVE_FAILED_TO_ENCODE_AUDIO_DATA:
        {
            return "failed to encode audio data";
        }
        case WAVE_FAILED_TO_WRITE_ENCODED_AUDIO_DATA:
        {
            return "failed to write encoded data";
        }
        default:
        {
            break;
        }
    }
    
    return "unknown error code";
}
