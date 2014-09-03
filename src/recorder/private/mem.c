#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "mem.h"

#ifdef DEBUG

#define DR_MAX_NUM_DEBUG_ALLOCATION_RECORDS 1024

typedef struct drAllocationRecord
{
    void* ptr;
    size_t size;
    const char* tag;
} drAllocationRecord;

static drAllocationRecord allocationRecords[DR_MAX_NUM_DEBUG_ALLOCATION_RECORDS];

static int clearRecords = 1;

static size_t numLiveBytes = 0;

#endif

void* drMalloc(size_t size, const char* tag)
{
#ifdef DEBUG
    if (clearRecords)
    {
        memset(allocationRecords, 0, DR_MAX_NUM_DEBUG_ALLOCATION_RECORDS * sizeof(drAllocationRecord));
        clearRecords = 0;
    }
    
    drAllocationRecord* record = NULL;
    
    for (int i = 0; i < DR_MAX_NUM_DEBUG_ALLOCATION_RECORDS; i++)
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
    
    printf("allocated pointer %p (%s), live bytes %ld\n", record->ptr, tag, numLiveBytes);
    
    return record->ptr;
#else
    return malloc(size);
#endif //DEBUG
}

void drFree(void* ptr)
{
#ifdef DEBUG
    if (clearRecords)
    {
        memset(allocationRecords, 0, DR_MAX_NUM_DEBUG_ALLOCATION_RECORDS * sizeof(drAllocationRecord));
        clearRecords = 0;
    }
    
    drAllocationRecord* record = NULL;
    for (int i = 0; i < DR_MAX_NUM_DEBUG_ALLOCATION_RECORDS; i++)
    {
        if (allocationRecords[i].ptr == 0)
        {
            record = &allocationRecords[i];
            break;
        }
    }
    
    if (!record)
    {
        assert(0 && "attempting to DR_FREE a pointer that was not allocated using DR_MALLOC");
        return;
    }
    
    numLiveBytes -= record->size;
    free(record->ptr);
    
    printf("freed pointer %p, live bytes %ld\n", record->ptr, numLiveBytes);
    
    memset(record, 0, sizeof(drAllocationRecord));
    
    
#else
    free(ptr);
#endif //DEBUG
}
