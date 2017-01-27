#include "wave_error_codes.h"

const char* drErrorToString(drError errorCode)
{
    switch (errorCode)
    {
        case DR_NO_ERROR:
        {
            return "no error";
        }
        case DR_ALREADY_INITIALIZED:
        {
            return "recorder instance is already initialized";
        }
        case DR_NOT_INITIALIZED:
        {
            return "recorder instance is not initialized";
        }
        case DR_FAILED_TO_INITIALIZE_HOST:
        {
            return "failed to initialize the audio host";
        }
        case DR_FAILED_TO_DEINITIALIZE_HOST:
        {
            return "failed to deinitialize the audio host";
        }
        case DR_FAILED_TO_INITIALIZE_ENCODER:
        {
            return "failed to initialize audio encoder";
        }
        case DR_FAILED_TO_ENCODE_AUDIO_DATA:
        {
            return "failed to encode audio data";
        }
        case DR_FAILED_TO_OPEN_ENCODER_TARGET_FILE:
        {
            return "failed to open encoder target file";
        }
        case DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA:
        {
            return "failed to write encoded data";
        }
        case DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE:
        {
            return "failed to close encoded target file";
        }
        default:
        {
            break;
        }
    }
    
    return "unknown error code";
}
