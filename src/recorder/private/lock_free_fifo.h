#ifndef WAVE_LOCK_FREE_FIFO_H
#define WAVE_LOCK_FREE_FIFO_H

/*! \file */ 

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * Single reader, single writer lock free FIFO.
     */
    typedef struct WaveLockFreeFIFO
    {
        int capacity;
        int elementSize;
        void* elements;
        /** Only manipulated through atomic operations. Only changed by the consumer thread.*/
        int head;
        /** Only accessed through atomic operations. Only changed by the producer thread.*/
        int tail;
    } WaveLockFreeFIFO;
    
    /**
     *
     */
    void wave_lock_free_fifo_init(WaveLockFreeFIFO* fifo, int capacity, int elementSize);
    
    /**
     *
     */
    void wave_lock_free_fifo_deinit(WaveLockFreeFIFO* fifo);
    
    /**
     *
     */
    int wave_lock_free_fifo_is_empty(WaveLockFreeFIFO* fifo);
    
    /**
     *
     */
    int wave_lock_free_fifo_is_full(WaveLockFreeFIFO* fifo);
    
    /**
     *
     */
    int wave_lock_free_fifo_get_num_elements(WaveLockFreeFIFO* fifo);
    
    /**
     * Called from the producer thread only.
     */
    int wave_lock_free_fifo_push(WaveLockFreeFIFO* fifo, const void* element);
    
    /**
     * Called from the consumer thread only.
     */
    int wave_lock_free_fifo_pop(WaveLockFreeFIFO* fifo, void* element);
    
#ifdef __cplusplus
} //extern "C"
#endif /* __cplusplus */

#endif //WAVE_LOCK_FREE_FIFO_H
