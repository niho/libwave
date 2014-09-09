#include <stdio.h>

#include "testmacros.h"
#include "test_oggopus.h"
#include "test_atomic.h"
#include "test_lock_free_fifo.h"

int main(int argc, const char * argv[])
{
    init_tests();
    
    testAtomic();
    testLockFreeFIFO();
    testOggOpus();
    
    finish_tests();
    
    return 0;
}

