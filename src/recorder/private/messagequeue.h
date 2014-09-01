#ifndef STF_MESSAGE_QUEUE_H
#define STF_MESSAGE_QUEUE_H

/*! \file */ 

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
    
    /**
     * A message queue with messages of a custom type.
     */
    typedef struct stfMessageQueue
    {
        int capacity;
        int numMessages;
        int messageSizeInBytes;
        void* messages;
    } stfMessageQueue;
    
    /**
     * Creates a new message queue with a given capacity and a given
     * message size in bytes.
     * @param capacity The maximum number of messages in the queue.
     * @param messageByteSize The size in bytes of a single message.
     * @return The created queue.
     */
    stfMessageQueue* stfMessageQueue_new(int capacity, int messageByteSize);
    
    /**
     * Deletes a given queue.
     * @param queue The queue to delete.
     */
    void stfMessageQueue_delete(stfMessageQueue* queue);
    
    /**
     * Moves messages from one queue to the end of another. If there is not
     * enough room for all messages in the destination queue, as many as possible
     * are moved and the source queue will contain the rest.
     * @param src The queue to move the messages from.
     * @param dest The queue to move the messages to.
     * @return The number of messages that were not moved. This is non-zero
     * if there is not enough room in the destination queue.
     */
    int stfMessageQueue_moveMessages(stfMessageQueue* src, stfMessageQueue* dest);
    
    /**
     * Removes all messages from a given queue.
     * @queue The queue to clear.
     */
    void stfMessageQueue_clear(stfMessageQueue* queue);
    
    /**
     * Adds a message to a given queue by copying the message contents.
     * @param queue The queue to add the message to.
     * @param message A pointer to the message to add. This must point
     * to a piece of data of size \c messageByteSize, as specified during
     * creation of the \c queue.
     * @return Zero on success, non-zero if \c queue is full.
     */
    int stfMessageQueue_addMessage(stfMessageQueue* queue, void* message);
    
    /**
     * Get the number of messages in a given queue.
     * @param queue The queue.
     * @return The number of messages currently in the queue.
     */
    int stfMessageQueue_getNumMessages(stfMessageQueue* queue);
    
    /**
     * Get a message at a given index from a given queue. If the index
     * is equal or greater than the number of messages, NULL is returned.
     * @param queue The queue.
     * @param idx The message slot index.
     * @return A pointer to the message at index \c idx.
     */ 
    void* stfMessageQueue_getMessage(stfMessageQueue* queue, int idx);
    
    
#ifdef __cplusplus
} //extern "C"
#endif /* __cplusplus */

#endif //STF_MESSAGE_QUEUE_H
