#include <stdlib.h>
#include <string.h>
#include "lock_free_fifo.h"
#include "atomic.h"
#include "mem.h"

/*Based on http://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular*/

void drLockFreeFIFO_init(drLockFreeFIFO* fifo, int capacity, int elementSize)
{
    memset(fifo, 0, sizeof(drLockFreeFIFO));
    fifo->capacity = capacity
    ;
    fifo->elementSize = elementSize;
    fifo->elements = DR_MALLOC(capacity * elementSize, "FIFO elements");
}

void drLockFreeFIFO_deinit(drLockFreeFIFO* fifo)
{
    if (fifo->elements)
    {
        DR_FREE(fifo->elements);
    }
    
    memset(fifo, 0, sizeof(drLockFreeFIFO));   
}

static int increment(int idx, int capacity)
{
    return (idx + 1) % capacity;
}

int drLockFreeFIFO_isEmpty(drLockFreeFIFO* fifo)
{
    return drAtomicLoad(&fifo->head) == drAtomicLoad(&fifo->tail);
}

int drLockFreeFIFO_isFull(drLockFreeFIFO* fifo)
{
    const int nextTail = increment(drAtomicLoad(&fifo->tail), fifo->capacity);
    return nextTail == drAtomicLoad(&fifo->head);
}

int drLockFreeFIFO_getNumElements(drLockFreeFIFO* fifo)
{
    const int currentTail = drAtomicLoad(&fifo->tail);
    const int currentHead = drAtomicLoad(&fifo->head);
    
    const int d = currentTail - currentHead;
    return d < 0 ? d + fifo->capacity : d;
}

int drLockFreeFIFO_push(drLockFreeFIFO* fifo, const void* element)
{    
    int currentTail = drAtomicLoad(&fifo->tail);
    const int nextTail = increment(currentTail, fifo->capacity);
    if(nextTail != drAtomicLoad(&fifo->head))
    {
        memcpy(&fifo->elements[currentTail * fifo->elementSize], element, fifo->elementSize);
        drAtomicStore(nextTail, &fifo->tail);
        return 1;
    }
    return 0;
}

int drLockFreeFIFO_pop(drLockFreeFIFO* fifo, void* element)
{
    const int currentHead = drAtomicLoad(&fifo->head);
    if(currentHead == drAtomicLoad(&fifo->tail))
    {
        return 0; // empty queue
    }
    
    memcpy(element, &fifo->elements[currentHead * fifo->elementSize], fifo->elementSize);
    drAtomicStore(increment(currentHead, fifo->capacity), &fifo->head);
    return 1;
}