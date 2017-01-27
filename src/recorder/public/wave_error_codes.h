#ifndef WAVE_ERROR_CODES_H
#define WAVE_ERROR_CODES_H

/*! \file  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Error codes.
     */
    typedef enum WaveError
    {
        WAVE_NO_ERROR = 0,
        WAVE_ALREADY_INITIALIZED,
        WAVE_NOT_INITIALIZED,
        
        WAVE_FAILED_TO_INITIALIZE_HOST,
        WAVE_FAILED_TO_DEINITIALIZE_HOST,
        
        WAVE_FAILED_TO_INITIALIZE_ENCODER,
        WAVE_FAILED_TO_ENCODE_AUDIO_DATA,
        WAVE_FAILED_TO_OPEN_ENCODER_TARGET_FILE,
        WAVE_FAILED_TO_WRITE_ENCODED_AUDIO_DATA,
        WAVE_FAILED_TO_CLOSE_ENCODER_TARGET_FILE
    } WaveError;
    
    /**
     * Helper function that returns a human readable description of an
     * error code.
     */
    const char* wave_error_str(WaveError errorCode);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_ERROR_CODES_H */
