#ifndef WAVE_STREAM_H
#define WAVE_STREAM_H

/*! \file */

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef void (*WaveStreamCallback)(const void* buffer, size_t numBytes, void* userData);

typedef struct WaveStream {
    WaveStreamCallback write;
    void *userData;
} WaveStream;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_STREAM_H */
