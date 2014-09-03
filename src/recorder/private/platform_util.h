#ifndef DR_PLATFORM_UTIL_H
#define DR_PLATFORM_UTIL_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * Returns 0 on success, 1 otherwise.
     */
    int drGetWritableFilePath(char* buffer, int bufferSize);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_PLATFORM_UTIL_H */