#ifndef DR_ATOMIC_H
#define DR_ATOMIC_H

/*! \file */ 

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     *
     */
    int drAtomicLoad(int* value);
    
    /**
     *
     */
    void drAtomicStore(int newValue, int* destination);
    
    /**
     *
     */
    int drAtomicAdd(int* value, int amount);
    
#ifdef __cplusplus
} //extern "C"
#endif /* __cplusplus */

#endif //DR_ATOMIC_H
