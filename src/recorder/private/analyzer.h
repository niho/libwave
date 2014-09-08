#ifndef DR_ANALYZER_H
#define DR_ANALYZER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    /**
     *
     */
    typedef void (*drAnalyzerAudioCallback)(void* analyzer, const float* inBuffer, int numChannels, int numFrames);
    
    /**
     *
     */
    typedef void (*drAnalyzerDeinitCallback)(void* analyzer);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_ANALYZER_H */