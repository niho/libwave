#ifndef DR_MEM_H
#define DR_MEM_H

/*! \file 
 
    Memory management macros to make it easier to debug
    memory usage.
 
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  
#define DR_MALLOC(size, tag) (malloc(size))
    
#define DR_FREE(ptr) (free(ptr))
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DR_MEM_H */