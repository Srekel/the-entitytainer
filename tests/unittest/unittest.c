#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

static UnitTestData g_testdata;

void
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

int
main( int argc, char** argv ) {
    (void)( argc );
    (void)( argv );

    memset( &g_testdata, 0, sizeof( g_testdata ) );
    UnitTestData* testdata = &g_testdata;

    unittest_run_entity32( testdata );
    unittest_run_default( testdata );

    // A bit of a hack.
    system( "pause" );

    return 0;
}
