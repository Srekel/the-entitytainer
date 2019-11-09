
#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct UnitTestData {
    char     errors[256][256];
    unsigned error_index;
    unsigned num_tests;
} UnitTestData;

void unittest_entitytainer_assert( bool test );

void unittest_run_default(UnitTestData* testdata);
// void unittest_run_entity32(UnitTestData* testdata);
