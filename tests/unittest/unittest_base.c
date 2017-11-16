#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "unittest.h"

#define ENTITYTAINER_assert unittest_entitytainer_assert
#define ASSERT unittest_entitytainer_assert
#define ENTITYTAINER_IMPLEMENTATION

#pragma warning( disable : 4464 ) // Include with ".."
#include "../../the_entitytainer.h"

#pragma warning( disable : 4710 ) // printf not inlined - I don't care. :)

static void
do_single_parent_tests( TheEntitytainer* entitytainer ) {
    entitytainer_add_entity( entitytainer, 3 );
    ASSERT( entitytainer_get_parent( entitytainer, 3 ) == 0 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 0 );

    int                    num_children;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( *children == 0 );
    ASSERT( num_children == 0 );

    entitytainer_add_child( entitytainer, 3, 4 );
    ASSERT( entitytainer_get_parent( entitytainer, 4 ) == 3 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 1 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 4 ) == 0 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( *children == 4 );
    ASSERT( num_children == 1 );

    entitytainer_add_child( entitytainer, 3, 6 );
    ASSERT( entitytainer_get_parent( entitytainer, 6 ) == 3 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 2 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 4 ) == 0 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 6 ) == 1 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( children[0] == 4 );
    ASSERT( children[1] == 6 );
    ASSERT( num_children == 2 );

    entitytainer_remove_entity( entitytainer, 4 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 1 );
    ASSERT( entitytainer_get_parent( entitytainer, 4 ) == 0 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 6 ) == 0 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( children[0] == 6 );
    ASSERT( num_children == 1 );

    for ( TheEntitytainerEntity i_child = 0; i_child < 4; ++i_child ) {
        entitytainer_add_child( entitytainer, 3, i_child + 10 );
    }

    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( children[0] == 6 );
    ASSERT( children[1] == 10 );
    ASSERT( children[4] == 13 );
    ASSERT( num_children == 5 );

    entitytainer_remove_child( entitytainer, 3, 6 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 4 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( children[0] == 10 );
    ASSERT( children[3] == 13 );
    ASSERT( num_children == 4 );

    entitytainer_remove_child( entitytainer, 3, 10 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 3 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 11 ) == 0 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 13 ) == 2 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( children[0] == 11 );
    ASSERT( children[2] == 13 );
    ASSERT( num_children == 3 );
}

static void
do_multi_parent_tests( TheEntitytainer* entitytainer ) {
    entitytainer_add_entity( entitytainer, 10 );
    entitytainer_add_entity( entitytainer, 20 );

    for ( TheEntitytainerEntity i_child = 0; i_child < 4; ++i_child ) {
        entitytainer_add_child( entitytainer, 10, 11 + i_child );
        entitytainer_add_child( entitytainer, 20, 21 + i_child );
    }

    ASSERT( entitytainer_get_parent( entitytainer, 11 ) == 10 );
    ASSERT( entitytainer_get_parent( entitytainer, 21 ) == 20 );
    ASSERT( entitytainer_num_children( entitytainer, 10 ) == 4 );
    ASSERT( entitytainer_num_children( entitytainer, 20 ) == 4 );

    entitytainer_remove_child( entitytainer, 20, 21 );
    ASSERT( entitytainer_get_parent( entitytainer, 11 ) == 10 );
    ASSERT( entitytainer_get_parent( entitytainer, 24 ) == 20 );
    ASSERT( entitytainer_num_children( entitytainer, 10 ) == 4 );
    ASSERT( entitytainer_num_children( entitytainer, 20 ) == 3 );

    entitytainer_add_entity( entitytainer, 30 );
    for ( TheEntitytainerEntity i_child = 0; i_child < 4; ++i_child ) {
        entitytainer_add_child( entitytainer, 30, 31 + i_child );
    }

    ASSERT( entitytainer_get_parent( entitytainer, 31 ) == 30 );
    ASSERT( entitytainer_num_children( entitytainer, 30 ) == 4 );

    entitytainer_remove_entity( entitytainer, 10 );
    entitytainer_add_entity( entitytainer, 10 );
    ASSERT( entitytainer_num_children( entitytainer, 10 ) == 0 );

    entitytainer_add_entity( entitytainer, 40 );
    for ( TheEntitytainerEntity i_child = 0; i_child < 15; ++i_child ) {
        entitytainer_add_child( entitytainer, 40, 41 + i_child );
    }

    for ( TheEntitytainerEntity i_child = 0; i_child < 15; ++i_child ) {
        ASSERT( entitytainer_get_parent( entitytainer, 41 + i_child ) == 40 );
        ASSERT( entitytainer_get_child_index( entitytainer, 40, 41 + i_child ) == i_child );
    }

    for ( TheEntitytainerEntity i_child = 0; i_child < 8; ++i_child ) {
        entitytainer_remove_entity( entitytainer, 41 + i_child );
    }

    int                    num_children;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 40, &children, &num_children );
    ASSERT( children[0] == 49 );
    ASSERT( num_children == 7 );

    entitytainer_add_entity( entitytainer, 41 );
    ASSERT( entitytainer_get_parent( entitytainer, 41 ) == 0 );
    ASSERT( entitytainer_num_children( entitytainer, 41 ) == 0 );
}

static void
unittest_run_base( UnitTestData* testdata ) {
    testdata->num_tests = 0;

    int   max_num_entries     = 1024;
    int   bucket_sizes[]      = { 4, 8, 16 };
    int   bucket_list_sizes[] = { 4, 2, 2 };
    int   needed_memory_size  = entitytainer_needed_size( max_num_entries, bucket_sizes, bucket_list_sizes, 3 );
    void* memory              = malloc( needed_memory_size );
    TheEntitytainer* entitytainer =
      entitytainer_create( memory, needed_memory_size, max_num_entries, bucket_sizes, bucket_list_sizes, 3 );

    printf( "\n" );
    printf( "Setup errors found: %u/%u\n", testdata->error_index, testdata->num_tests );

    if ( testdata->error_index > 0 ) {
        printf( "Errors found during setup, exiting.\n" );
        goto LABEL_done;
    }

    testdata->num_tests = 0;

    do_single_parent_tests( entitytainer );
    do_multi_parent_tests( entitytainer );

    printf( "Run errors found:   %u/%u\n", testdata->error_index, testdata->num_tests );

    printf( "\n" );
    if ( testdata->error_index == 0 ) {
        printf( "No errors found, YAY!\n" );
    }
    else {
        printf( "U are teh sux.\n" );
    }

LABEL_done:;
    free( entitytainer );
}
