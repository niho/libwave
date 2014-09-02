#ifndef STF_LOCK_FREE_FIFO_H
#define STF_LOCK_FREE_FIFO_H

/*! \file */ 

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Single reader, single writer lock free FIFO.
     */
    typedef struct drLockFreeFIFO
    {
        int capacity;
        int elementSize;
        void* elements;
        /** Only manipulated through atomic operations. Only changed by the consumer thread.*/
        int head;
        /** Only accessed through atomic operations. Only changed by the producer thread.*/
        int tail;
    } drLockFreeFIFO;
    
    /**
     *
     */
    void drLockFreeFIFO_init(drLockFreeFIFO* fifo, int capacity, int elementSize);
    
    /**
     *
     */
    void drLockFreeFIFO_deinit(drLockFreeFIFO* fifo);
    
    /**
     *
     */
    int drLockFreeFIFO_isEmpty(drLockFreeFIFO* fifo);
    
    /**
     *
     */
    int drLockFreeFIFO_isFull(drLockFreeFIFO* fifo);
    
    /**
     *
     */
    int drLockFreeFIFO_getNumElements(drLockFreeFIFO* fifo);
    
    /**
     * Called from the producer thread only.
     */
    int drLockFreeFIFO_push(drLockFreeFIFO* fifo, const void* element);
    
    /**
     * Called from the consumer thread only.
     */
    int drLockFreeFIFO_pop(drLockFreeFIFO* fifo, void* element);
    
#ifdef __cplusplus
} //extern "C"
#endif /* __cplusplus */

#endif //STF_LOCK_FREE_FIFO_H
