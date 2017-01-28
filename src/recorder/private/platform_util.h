#ifndef WAVE_PLATFORM_UTIL_H
#define WAVE_PLATFORM_UTIL_H

/*! \file */

#include "encoder.h"
#include "wave_settings.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     *
     */
    void wave_create_encoder(WaveEncoder* encoder, WaveSettings* settings);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_PLATFORM_UTIL_H */
