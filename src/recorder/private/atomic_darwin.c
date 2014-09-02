#include <libkern/OSAtomic.h>

#include "atomic.h"


int drAtomicLoad(int* value)
{
    return OSAtomicAdd32Barrier(0, value);
}

void drAtomicStore(int newValue, int* destination)
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

int drAtomicAdd(int* value, int amount)
{
    return OSAtomicAdd32Barrier(amount, value);
}
