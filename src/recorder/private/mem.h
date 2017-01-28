#ifndef WAVE_MEM_H
#define WAVE_MEM_H

#include <stdlib.h>

/*! \file 
 
    Memory management macros to make it easier to debug
    memory usage.
 
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * Tries to allocate a given number of bytes, associating the allocation
     * with an arbitrary user tag.
     */
    #define WAVE_MALLOC(size, tag) (wave_malloc(size, tag))
    
    /**
     * Frees a given pointer.
     */
    #define WAVE_FREE(ptr) (wave_free(ptr))
    
    void* wave_malloc(size_t size, const char* tag);
    
    void wave_free(void* ptr);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WAVE_MEM_H */
