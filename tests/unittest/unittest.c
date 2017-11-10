#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct UnitTestData {
    char     errors[256][256];
    unsigned error_index;
    unsigned num_tests;

    bool simplebool_target;
    bool generatedbool_target;

    char stringtest[256];
} UnitTestData;

static UnitTestData g_testdata;

static void
unittest_entitytainer_assert( bool test ) {
    ++g_testdata.num_tests;
    if ( g_testdata.error_index == 256 ) {
        assert( false );
    }
    if ( !test ) {
        memcpy( g_testdata.errors[g_testdata.error_index++], "LOL", 4 );
    }
    // DebugBreak();
}

#define ENTITYTAINER_assert unittest_entitytainer_assert
#define ASSERT unittest_entitytainer_assert
#define ENTITYTAINER_IMPLEMENTATION

#include "../../the_entitytainer.h"

void* allocate(int size) {
	return malloc(size);
}

static void
unittest_run() {

    memset( &g_testdata, 0, sizeof( g_testdata ) );
    UnitTestData* testdata = &g_testdata;

	int bucket_sizes[] = { 4,16,256 };
	int bucket_list_sizes[] = { 32, 8, 2 };
	TheEntitytainer* entitytainer = entitytainer_create(allocate, 65535, bucket_sizes, bucket_list_sizes, 3);

    printf( "\n" );
    printf( "Setup errors found: %u/%u\n", testdata->error_index, testdata->num_tests );

    if ( testdata->error_index > 0 ) {
        printf( "Errors found during setup, exiting.\n" );
        goto LABEL_done;
    }

    testdata->num_tests = 0;

    printf( "Run errors found:   %u/%u\n", testdata->error_index, testdata->num_tests );

    printf( "\n" );
    if ( testdata->error_index == 0 ) {
        printf( "No errors found, YAY!\n" );
    }
    else {
        printf( "U are teh sux.\n" );
    }

LABEL_done:;
	free(entitytainer);
}

int
main( int argc, char** argv ) {
    (void)( argc, argv );
    
	unittest_run();

	// A bit of a hack. 
	system("pause");

    return 0;
}
