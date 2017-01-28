#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "mem.h"

#ifdef DEBUG

#define WAVE_MAX_NUM_DEBUG_ALLOCATION_RECORDS 1024

typedef struct WaveAllocationRecord
{
    void* ptr;
    size_t size;
    const char* tag;
} WaveAllocationRecord;

static WaveAllocationRecord allocationRecords[WAVE_MAX_NUM_DEBUG_ALLOCATION_RECORDS];

static int clearRecords = 1;

static size_t numLiveBytes = 0;

#endif

void* wave_malloc(size_t size, const char* tag)
{
#ifdef DEBUG
    if (clearRecords)
    {
        memset(allocationRecords, 0, WAVE_MAX_NUM_DEBUG_ALLOCATION_RECORDS * sizeof(WaveAllocationRecord));
        clearRecords = 0;
    }
    
    WaveAllocationRecord* record = NULL;
    
    for (int i = 0; i < WAVE_MAX_NUM_DEBUG_ALLOCATION_RECORDS; i++)
    {
        if (allocationRecords[i].ptr == 0)
        {
            record = &allocationRecords[i];
            break;
        }
    }
    
    if (!record)
    {
        assert(0 && "no free allocation records!");
        return NULL;
    }
    
    record->ptr = malloc(size);
    record->size = size;
    record->tag = tag;
    numLiveBytes += size;
    
    printf("allocated pointer %ld bytes at %p (%s), live bytes %ld\n",
           record->size, record->ptr, tag, numLiveBytes);
    
    return record->ptr;
#else
    return malloc(size);
#endif //DEBUG
}

void wave_free(void* ptr)
{
#ifdef DEBUG
    if (clearRecords)
    {
        memset(allocationRecords, 0, WAVE_MAX_NUM_DEBUG_ALLOCATION_RECORDS * sizeof(WaveAllocationRecord));
        clearRecords = 0;
    }
    
    WaveAllocationRecord* record = NULL;
    for (int i = 0; i < WAVE_MAX_NUM_DEBUG_ALLOCATION_RECORDS; i++)
    {
        if (allocationRecords[i].ptr == ptr)
        {
            record = &allocationRecords[i];
            break;
        }
    }
    
    if (!record)
    {
        assert(0 && "attempting to WAVE_FREE a pointer that was not allocated using WAVE_MALLOC");
        return;
    }
    
    numLiveBytes -= record->size;
    free(record->ptr);
    
    printf("freed pointer %p, live bytes %ld\n", record->ptr, numLiveBytes);
    
    memset(record, 0, sizeof(WaveAllocationRecord));
    
    
#else
    free(ptr);
#endif //DEBUG
}
