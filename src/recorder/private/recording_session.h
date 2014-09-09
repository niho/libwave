#ifndef DR_RECORDING_SESSION_H
#define DR_RECORDING_SESSION_H

/*! \file */

#include "encoder.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    typedef struct drRecordingSession
    {
        drEncoder encoder;
        int numRecordedFrames;
        
    } drRecordingSession;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_RECORDING_SESSION_H */