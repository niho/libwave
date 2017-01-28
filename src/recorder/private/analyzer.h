#ifndef WAVE_ANALYZER_H
#define WAVE_ANALYZER_H

/*! \file */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
    /**
     *
     */
    typedef void (*WaveAnalyzerAudioCallback)(void* analyzer, const float* inBuffer, int numChannels, int numFrames);
    
    /**
     *
     */
    typedef void (*WaveAnalyzerDeinitCallback)(void* analyzer);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_ANALYZER_H */
