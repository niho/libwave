#ifndef DR_MEM_H
#define DR_MEM_H

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
    #define DR_MALLOC(size, tag) (drMalloc(size, tag))
    
    /**
     * Frees a given pointer.
     */
    #define DR_FREE(ptr) (drFree(ptr))
    
    void* drMalloc(size_t size, const char* tag);
    
    void drFree(void* ptr);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_MEM_H */