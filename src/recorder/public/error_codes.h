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
        DR_NOT_INITIALIZED
    } drError;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_ERROR_CODES_H */