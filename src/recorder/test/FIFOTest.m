
#import <stdio.h>
#import "FIFOTest.h"

#import "c/thread/lockfreefifo.h"
#import "c/thread/thread.h"

@implementation FIFOTest

static const int loopCount = 10000;

static void* entryPointProducer(void* data)
{
    stfLockFreeFIFO* f = (stfLockFreeFIFO*)data;

    for (int i = 0; i < loopCount; i++)
    {
        int success = stfLockFreeFIFO_push(f, &i);
        //printf("pushed %d (success %d)\n", i, success);
    }
    
    return NULL;
}

static void* entryPointConsumer(void* data)
{
    
    stfLockFreeFIFO* f = (stfLockFreeFIFO*)data;
    
    for (int i = 0; i < loopCount; i++)
    {
        int val = 0;
        int success = stfLockFreeFIFO_pop(f, &val);
        //printf("popped %d (success %d)\n", val, success);

    }
    
    return NULL;

}

- (void)testTwoThreads
{
    //TODO: actually check stuff
    const int es = sizeof(int);
    const int c = 100;
    
    stfLockFreeFIFO f;
    stfLockFreeFIFO_init(&f, c, es);
    
    stfThread* t1 = stfThread_new(entryPointConsumer, &f);
    stfThread* t2 = stfThread_new(entryPointProducer, &f);
    
    stfThread_join(t1);
    stfThread_join(t2);
}

- (void)testSize
{
    const int es = sizeof(int);
    const int c = 100;
    
    const int nCases = 5;
    int nPush[nCases] = {10, 20, 30, 5, 90};
    int nPop[nCases] = {5, 1, 60, 10, 90};
    
    for (int i = 0; i < nCases; i++)
    {
        stfLockFreeFIFO f;
        stfLockFreeFIFO_init(&f, c, es);
        
        for (int j = 0; j < nPush[i]; j++)
        {
            int success = stfLockFreeFIFO_push(&f, &j);
        }
        
        for (int j = 0; j < nPop[i]; j++)
        {
            int val = 0;
            int success = stfLockFreeFIFO_pop(&f, &val);
        }
        
        const int expectedSize = fmaxf(0.0f, nPush[i] - nPop[i]);
        const int size = stfLockFreeFIFO_getNumElements(&f);
        STAssertEquals(expectedSize, size, @"wrong size");
        
        stfLockFreeFIFO_deinit(&f);
    }
    
    
}

- (void)testPushUntilFull
{
    const int es = sizeof(int);
    const int c = 100;
    
    stfLockFreeFIFO f;
    stfLockFreeFIFO_init(&f, c, es);
    
    for (int i = 0; i < c; i++)
    {
        int success = stfLockFreeFIFO_push(&f, &i);
        if (!success)
        {
            STAssertEquals(success, 1, @"");
        }
    }
    
    int success = stfLockFreeFIFO_push(&f, &c);
    STAssertEquals(success, 0, @"");
    
    const int full = stfLockFreeFIFO_isFull(&f);
    STAssertEquals(full, 1, @"");
    
    const int empty = stfLockFreeFIFO_isEmpty(&f);
    STAssertEquals(empty, 0, @"");
}

- (void)testPushUntilFullAndPopUntilEmpty
{
    const int es = sizeof(int);
    const int c = 100;
    
    stfLockFreeFIFO f;
    stfLockFreeFIFO_init(&f, c, es);
    
    for (int i = 0; i < c; i++)
    {
        int success = stfLockFreeFIFO_push(&f, &i);
        if (!success)
        {
            STAssertEquals(success, 1, @"");
        }
    }
    
    int success = stfLockFreeFIFO_push(&f, &c);
    STAssertEquals(success, 0, @"");
    
    int full = stfLockFreeFIFO_isFull(&f);
    STAssertEquals(full, 1, @"");
    
    int empty = stfLockFreeFIFO_isEmpty(&f);
    STAssertEquals(empty, 0, @"");
    
    for (int i = 0; i < c; i++)
    {
        int val = 0;
        int success = stfLockFreeFIFO_pop(&f, &val);
        STAssertEquals(success, 1, @"pop failed");
        STAssertEquals(val, i, @"pop gave wrong value");
    }
    
    full = stfLockFreeFIFO_isFull(&f);
    STAssertEquals(full, 0, @"");
    
    empty = stfLockFreeFIFO_isEmpty(&f);
    STAssertEquals(empty, 1, @"");
}


@end
