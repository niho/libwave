#ifndef DR_RECORDING_SESSION_H
#define DR_RECORDING_SESSION_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    typedef struct drRecordingSession
    {
        char targetFilePath;
        int numRecordedFrames;
        
    } drRecordingSession;
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_RECORDING_SESSION_H */