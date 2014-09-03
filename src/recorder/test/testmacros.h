#ifndef DR_TEST_MACROS_H
#define DR_TEST_MACROS_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


int numFailedTests;
int numSuccessfulTests;

#define init_tests() numFailedTests = 0; numSuccessfulTests = 0

#define finish_tests() printf("\n\nFinished running %d tests (%d successful, %d failed) \n---------------------------------------------------------\n", numFailedTests + numSuccessfulTests, numSuccessfulTests, numFailedTests)

#define fail_unless(condition, message) if (!(condition)) { numFailedTests++; printf("    - FAILED: %s\n", message);} else { numSuccessfulTests++; printf("    - passed: %s\n", message);}

#define start_test(testName) printf("\nStarting test '%s'\n---------------------------------------------------------\n", testName);

#ifdef __cplusplus
}
#endif

    
#endif //DR

