#ifndef WAVE_ATOMIC_H
#define WAVE_ATOMIC_H

/*! \file */ 

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     *
     */
    int wave_atomic_load(int* value);
    
    /**
     *
     */
    void wave_atomic_store(int newValue, int* destination);
    
    /**
     *
     */
    int wave_atomic_add(int* value, int amount);
    
#ifdef __cplusplus
} //extern "C"
#endif /* __cplusplus */

#endif //WAVE_ATOMIC_H
