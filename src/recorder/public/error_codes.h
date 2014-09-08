#ifndef DR_ERROR_CODES_H
#define DR_ERROR_CODES_H

/*! \file  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Error codes.
     */
    typedef enum drError
    {
        DR_NO_ERROR = 0,
        DR_ALREADY_INITIALIZED,
        DR_NOT_INITIALIZED,
        
        DR_FAILED_TO_INITIALIZE_HOST,
        DR_FAILED_TO_DEINITIALIZE_HOST,
        
        DR_FAILED_TO_INITIALIZE_ENCODER,
        DR_FAILED_TO_ENCODE_AUDIO_DATA,
        DR_FAILED_TO_OPEN_ENCODER_TARGET_FILE,
        DR_FAILED_TO_WRITE_ENCODED_AUDIO_DATA,
        DR_FAILED_TO_CLOSE_ENCODER_TARGET_FILE
    } drError;
    
    /**
     * Helper function that returns a human readable description of an
     * error code.
     */
    const char* drErrorToString(drError errorCode);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_ERROR_CODES_H */