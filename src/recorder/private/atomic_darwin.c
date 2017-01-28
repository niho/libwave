#include <libkern/OSAtomic.h>

#include "atomic.h"


int wave_atomic_load(int* value)
{
    return OSAtomicAdd32Barrier(0, value);
}

void wave_atomic_store(int newValue, int* destination)
{
    while (true)
    {
        int oldValue = *destination;
        if (OSAtomicCompareAndSwap32Barrier(oldValue, newValue, destination))
        {
            return;
        }
    }
}

int wave_atomic_add(int* value, int amount)
{
    return OSAtomicAdd32Barrier(amount, value);
}
