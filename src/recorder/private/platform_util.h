#ifndef DR_PLATFORM_UTIL_H
#define DR_PLATFORM_UTIL_H

/*! \file */

#include "encoder.h"
#include "settings.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * Returns 0 on success, 1 otherwise.
     */
    int drGetWritableFilePath(char* buffer, int bufferSize);
    
    /**
     *
     */
    void drCreateEncoder(drEncoder* encoder, drSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_PLATFORM_UTIL_H */