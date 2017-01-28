#include <stdlib.h>
#include <string.h>
#include "lock_free_fifo.h"
#include "atomic.h"
#include "mem.h"

/*Based on http://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular*/

void wave_lock_free_fifo_init(WaveLockFreeFIFO* fifo, int capacity, int elementSize)
{
    memset(fifo, 0, sizeof(WaveLockFreeFIFO));
    fifo->capacity = capacity + 1;
    fifo->elementSize = elementSize;
    fifo->elements = WAVE_MALLOC(fifo->capacity * elementSize, "FIFO elements");
}

void wave_lock_free_fifo_deinit(WaveLockFreeFIFO* fifo)
{
    if (fifo->elements)
    {
        WAVE_FREE(fifo->elements);
    }
    
    memset(fifo, 0, sizeof(WaveLockFreeFIFO));
}

static int increment(int idx, int capacity)
{
    return (idx + 1) % capacity;
}

int wave_lock_free_fifo_is_empty(WaveLockFreeFIFO* fifo)
{
    return wave_atomic_load(&fifo->head) == wave_atomic_load(&fifo->tail);
}

int wave_lock_free_fifo_is_full(WaveLockFreeFIFO* fifo)
{
    const int nextTail = increment(wave_atomic_load(&fifo->tail), fifo->capacity);
    return nextTail == wave_atomic_load(&fifo->head);
}

int wave_lock_free_fifo_get_num_elements(WaveLockFreeFIFO* fifo)
{
    const int currentTail = wave_atomic_load(&fifo->tail);
    const int currentHead = wave_atomic_load(&fifo->head);
    
    const int d = currentTail - currentHead;
    return d < 0 ? d + fifo->capacity : d;
}

int wave_lock_free_fifo_push(WaveLockFreeFIFO* fifo, const void* element)
{    
    int currentTail = wave_atomic_load(&fifo->tail);
    const int nextTail = increment(currentTail, fifo->capacity);
    if(nextTail != wave_atomic_load(&fifo->head))
    {
        memcpy(&(((unsigned char*)fifo->elements)[currentTail * fifo->elementSize]), element, fifo->elementSize);
        wave_atomic_store(nextTail, &fifo->tail);
        return 1;
    }
    return 0;
}

int wave_lock_free_fifo_pop(WaveLockFreeFIFO* fifo, void* element)
{
    const int currentHead = wave_atomic_load(&fifo->head);
    if(currentHead == wave_atomic_load(&fifo->tail))
    {
        return 0; // empty queue
    }
    
    memcpy(element, &fifo->elements[currentHead * fifo->elementSize], fifo->elementSize);
    wave_atomic_store(increment(currentHead, fifo->capacity), &fifo->head);
    return 1;
}
