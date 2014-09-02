#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "messagequeue.h"

drMessageQueue* drMessageQueue_new(int capacity, int messageByteSize)
{
    assert(capacity > 0);
    assert(messageByteSize > 0);
    
    drMessageQueue* queue = DR_MALLOC(sizeof(drMessageQueue), "message queue");
    memset(queue, 0, sizeof(drMessageQueue));
    
    queue->messageSizeInBytes = messageByteSize;
    queue->capacity = capacity;
    queue->numMessages = 0;
    queue->messages = DR_MALLOC(capacity * messageByteSize, "message queue messages");
    memset(queue->messages, 0, capacity * messageByteSize);
    
    return queue;
}

void drMessageQueue_delete(drMessageQueue* queue)
{
    free(queue->messages);
    free(queue);
}

int drMessageQueue_moveMessages(drMessageQueue* src, drMessageQueue* dest)
{
    assert(src);
    assert(dest);
    assert(src->messageSizeInBytes == dest->messageSizeInBytes);
    
    //check how many messages there is room for in the destination queue
    int numMessagesToMove = src->numMessages;
    int numFreeDestinationSlots = dest->capacity - dest->numMessages;
    if (numMessagesToMove > numFreeDestinationSlots)
    {
        numMessagesToMove = numFreeDestinationSlots;
    }
    
    //move messages to destination queue. TODO this could be one memcpy
    int destStartIndex = dest->numMessages;
    for (int i = 0; i < numMessagesToMove; i++)
    {
        assert(destStartIndex + i < dest->capacity);
        memcpy(&dest->messages[src->messageSizeInBytes * (destStartIndex + i)], 
               &src->messages[src->messageSizeInBytes * i], 
               src->messageSizeInBytes);
    }
    
    //move any remaining source messages to the front of the queue
    const int numMessagesNotMoved = src->numMessages - numMessagesToMove;
    for (int i = 0; i < numMessagesNotMoved; i++)
    {
        memcpy(&src->messages[src->messageSizeInBytes * i], 
               &src->messages[src->messageSizeInBytes * (i + numMessagesToMove)], 
               src->messageSizeInBytes);
    }
    
    //update message counts
    src->numMessages = numMessagesNotMoved;
    dest->numMessages += numMessagesToMove;
    
    //return the number of messages *not* moved
    return numMessagesNotMoved;
}

void drMessageQueue_clear(drMessageQueue* queue)
{
    memset(queue->messages, 0, queue->numMessages * queue->messageSizeInBytes);
    queue->numMessages = 0;
}

int drMessageQueue_addMessage(drMessageQueue* queue, void* message)
{
    if (queue->numMessages == queue->capacity)
    {
        return 1;
    }
    
    //copy the messages into the appropriate slot
    memcpy(&queue->messages[queue->messageSizeInBytes * queue->numMessages], 
           message, 
           queue->messageSizeInBytes);

    queue->numMessages++;
    
    return 0;
}

int drMessageQueue_getNumMessages(drMessageQueue* queue)
{
    return queue->numMessages;
}

void* drMessageQueue_getMessage(drMessageQueue* queue, int idx)
{
    assert(idx >= 0);
    
    if (idx >= queue->numMessages)
    {
        return NULL;
    }
    
    return &queue->messages[idx * queue->messageSizeInBytes];
}
