
#pragma warning( disable : 4710 ) // printf not inlined - I don't care. :)
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

static void
do_single_parent_tests( TheEntitytainer* entitytainer ) {
    entitytainer_add_entity( entitytainer, 3 );
    ASSERT( entitytainer_get_parent( entitytainer, 3 ) == 0 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 0 );

    int                    num_children;
    int                    capacity;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( *children == 0 );
    ASSERT( num_children == 0 );

    entitytainer_add_child( entitytainer, 3, 4 );
    ASSERT( entitytainer_get_parent( entitytainer, 4 ) == 3 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 1 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 4 ) == 0 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( *children == 4 );
    ASSERT( num_children == 1 );

    entitytainer_add_child( entitytainer, 3, 6 );
    ASSERT( entitytainer_get_parent( entitytainer, 6 ) == 3 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 2 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 4 ) == 0 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 6 ) == 1 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( children[0] == 4 );
    ASSERT( children[1] == 6 );
    ASSERT( num_children == 2 );

    entitytainer_remove_entity( entitytainer, 4 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 1 );
    ASSERT( entitytainer_get_parent( entitytainer, 4 ) == 0 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 6 ) == 0 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( children[0] == 6 );
    ASSERT( num_children == 1 );

    for ( TheEntitytainerEntity i_child = 0; i_child < 4; ++i_child ) {
        entitytainer_add_child( entitytainer, 3, i_child + 10 );
    }

    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( children[0] == 6 );
    ASSERT( children[1] == 10 );
    ASSERT( children[4] == 13 );
    ASSERT( num_children == 5 );

    entitytainer_remove_child_no_holes( entitytainer, 3, 6 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 4 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( children[0] == 10 );
    ASSERT( children[3] == 13 );
    ASSERT( num_children == 4 );

    entitytainer_remove_child_no_holes( entitytainer, 3, 10 );
    ASSERT( entitytainer_num_children( entitytainer, 3 ) == 3 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 11 ) == 0 );
    ASSERT( entitytainer_get_child_index( entitytainer, 3, 13 ) == 2 );

    entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    ASSERT( children[0] == 11 );
    ASSERT( children[2] == 13 );
    ASSERT( num_children == 3 );

    while ( num_children > 0 ) {
        entitytainer_remove_entity( entitytainer, children[0] );
        entitytainer_get_children( entitytainer, 3, &children, &num_children, &capacity );
    }

    entitytainer_remove_entity( entitytainer, 3 );
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

    entitytainer_remove_child_no_holes( entitytainer, 20, 21 );
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

    int                    num_children;
    int                    capacity;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    while ( num_children > 0 ) {
        entitytainer_remove_entity( entitytainer, children[0] );
        entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    }

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

    entitytainer_get_children( entitytainer, 40, &children, &num_children, &capacity );
    ASSERT( children[0] == 49 );
    ASSERT( num_children == 7 );

    entitytainer_add_entity( entitytainer, 41 );
    ASSERT( entitytainer_get_parent( entitytainer, 41 ) == 0 );
    ASSERT( entitytainer_num_children( entitytainer, 41 ) == 0 );

    entitytainer_get_children( entitytainer, 20, &children, &num_children, &capacity );
    while ( num_children > 0 ) {
        entitytainer_remove_entity( entitytainer, children[0] );
        entitytainer_get_children( entitytainer, 20, &children, &num_children, &capacity );
    }

    entitytainer_get_children( entitytainer, 40, &children, &num_children, &capacity );
    while ( num_children > 0 ) {
        entitytainer_remove_entity( entitytainer, children[0] );
        entitytainer_get_children( entitytainer, 40, &children, &num_children, &capacity );
    }

    entitytainer_remove_entity( entitytainer, 10 );
    entitytainer_remove_entity( entitytainer, 20 );
    entitytainer_remove_entity( entitytainer, 40 );
}

static void
do_single_parent_hole_tests( TheEntitytainer* entitytainer ) {
    entitytainer_add_entity( entitytainer, 10 );

    int                    num_children;
    int                    capacity;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 3 );

    entitytainer_add_child( entitytainer, 10, 20 );
    entitytainer_add_child( entitytainer, 10, 21 );
    entitytainer_add_child( entitytainer, 10, 22 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 3 );

    entitytainer_add_child( entitytainer, 10, 23 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 4 );
    ASSERT( capacity == 7 );

    entitytainer_remove_child_with_holes( entitytainer, 10, 20 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 7 );
    ASSERT( children[0] == ENTITYTAINER_InvalidEntity );

    entitytainer_add_child( entitytainer, 10, 30 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 4 );
    ASSERT( capacity == 7 );
    ASSERT( children[0] == 30 );

    entitytainer_remove_child_with_holes( entitytainer, 10, 30 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 7 );
    ASSERT( children[0] == ENTITYTAINER_InvalidEntity );

    entitytainer_remove_child_with_holes( entitytainer, 10, 21 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 22 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 23 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 3 );

    entitytainer_add_child( entitytainer, 10, 20 );
    entitytainer_add_child( entitytainer, 10, 21 );
    entitytainer_add_child( entitytainer, 10, 22 );
    entitytainer_add_child( entitytainer, 10, 23 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 23 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 7 );

    entitytainer_remove_child_with_holes( entitytainer, 10, 22 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 2 );
    ASSERT( capacity == 3 );
}

static void
do_reserve_tests( TheEntitytainer* entitytainer ) {
    entitytainer_add_entity( entitytainer, 10 );

    int                    num_children;
    int                    capacity;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 3 );

    entitytainer_add_child( entitytainer, 10, 20 );
    entitytainer_add_child( entitytainer, 10, 21 );
    entitytainer_add_child( entitytainer, 10, 22 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 3 );

    entitytainer_reserve( entitytainer, 10, 5 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 7 );

    entitytainer_remove_child_with_holes( entitytainer, 10, 22 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 21 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 20 );

    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 7 );

    entitytainer_add_child( entitytainer, 10, 20 );
    entitytainer_add_child( entitytainer, 10, 21 );
    entitytainer_add_child( entitytainer, 10, 22 );
    entitytainer_add_child( entitytainer, 10, 23 );
    entitytainer_add_child( entitytainer, 10, 24 );
    entitytainer_add_child( entitytainer, 10, 25 );
    entitytainer_add_child( entitytainer, 10, 26 );

    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 7 );
    ASSERT( capacity == 7 );

    entitytainer_remove_child_with_holes( entitytainer, 10, 25 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 23 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 24 );
    entitytainer_remove_child_with_holes( entitytainer, 10, 22 );

    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 7 );
}

static void
do_multi_entity_tests( TheEntitytainer* entitytainer ) {
    entitytainer_add_entity( entitytainer, 10 );

    int                    num_children;
    int                    capacity;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 3 );

    entitytainer_add_child( entitytainer, 10, 20 );
    entitytainer_add_child( entitytainer, 10, 21 );
    entitytainer_add_child( entitytainer, 10, 22 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 3 );

    entitytainer_add_entity( entitytainer, 30 );
    entitytainer_get_children( entitytainer, 30, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 3 );
    ASSERT( children[0] == 0 );
    ASSERT( children[1] == 0 );
    ASSERT( children[2] == 0 );

    entitytainer_add_child( entitytainer, 30, 31 );
    entitytainer_add_child( entitytainer, 30, 32 );
    entitytainer_add_child( entitytainer, 30, 33 );

    // This is the real test - 10's old bicked should be freed and then given to 1000
    entitytainer_add_child( entitytainer, 10, 23 );
    entitytainer_add_child( entitytainer, 10, 24 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 5 );
    ASSERT( capacity == 7 );

    entitytainer_add_entity( entitytainer, 40 );
    entitytainer_get_children( entitytainer, 40, &children, &num_children, &capacity );
    ASSERT( num_children == 0 );
    ASSERT( capacity == 3 );
    ASSERT( children[0] == 0 );
    ASSERT( children[1] == 0 );
    ASSERT( children[2] == 0 );

    entitytainer_get_children( entitytainer, 30, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 3 );
    ASSERT( children[0] == 31 );
    ASSERT( children[1] == 32 );
    ASSERT( children[2] == 33 );

    entitytainer_remove_child_no_holes( entitytainer, 10, 23 );
    entitytainer_remove_child_no_holes( entitytainer, 10, 24 );
    entitytainer_get_children( entitytainer, 10, &children, &num_children, &capacity );
    ASSERT( num_children == 3 );
    ASSERT( capacity == 3 );

    entitytainer_add_child( entitytainer, 30, 34 );
    entitytainer_get_children( entitytainer, 30, &children, &num_children, &capacity );
    ASSERT( num_children == 4 );
    ASSERT( capacity == 7 );
    ASSERT( children[0] == 31 );
    ASSERT( children[1] == 32 );
    ASSERT( children[2] == 33 );
    ASSERT( children[3] == 34 );
    ASSERT( children[4] == 0 );
    ASSERT( children[5] == 0 );
    ASSERT( children[6] == 0 );
}

static void
do_save_load_test( TheEntitytainer* entitytainer ) {
    int            buffer_size = entitytainer_save( entitytainer, NULL, 0 );
    unsigned char* buffer      = malloc( buffer_size );
    memset( buffer, 0, buffer_size );
    entitytainer_save( entitytainer, buffer, buffer_size );
    ASSERT( memcmp( entitytainer, buffer, buffer_size ) == 0 );

    unsigned char* loaded_buffer = malloc( buffer_size );
    memset( loaded_buffer, 0, buffer_size );
    memcpy( loaded_buffer, buffer, buffer_size );
    TheEntitytainer* loaded = entitytainer_load( loaded_buffer, buffer_size );

    memcpy( buffer, loaded, buffer_size );
    TheEntitytainer* loaded2 = entitytainer_load( buffer, buffer_size );
    // ASSERT( memcmp( entitytainer, &buffer, buffer_size ) == 0 );
    (void)loaded2;
}

static void
unittest_run_base( UnitTestData* testdata ) {
    testdata->num_tests = 0;

    int                          max_num_entries     = 64;
    int                          bucket_sizes[]      = { 4, 8, 16 };
    int                          bucket_list_sizes[] = { 4, 2, 2 };
    struct TheEntitytainerConfig config              = { 0 };
    config.num_entries                               = max_num_entries;
    config.bucket_sizes                              = bucket_sizes;
    config.bucket_list_sizes                         = bucket_list_sizes;
    config.num_bucket_lists                          = 3;
    config.remove_with_holes                         = false;
    int needed_memory_size                           = entitytainer_needed_size( &config );
    config.memory                                    = malloc( needed_memory_size );
    config.memory_size                               = needed_memory_size;

    TheEntitytainer* entitytainer = entitytainer_create( &config );

    printf( "\n" );
    printf( "Setup errors found: %u/%u\n", testdata->error_index, testdata->num_tests );

    if ( testdata->error_index > 0 ) {
        printf( "Errors found during setup, exiting.\n" );
        goto LABEL_done;
    }

    testdata->num_tests = 0;

    do_single_parent_tests( entitytainer );
    do_multi_parent_tests( entitytainer );

    memset( config.memory, 0, config.memory_size );
    config.remove_with_holes = true;
    entitytainer             = entitytainer_create( &config );
    do_single_parent_hole_tests( entitytainer );
    do_save_load_test( entitytainer );

    memset( config.memory, 0, config.memory_size );
    config.keep_capacity_on_remove = true;
    entitytainer                   = entitytainer_create( &config );
    do_reserve_tests( entitytainer );
    do_save_load_test( entitytainer );

    memset( config.memory, 0, config.memory_size );
    config.keep_capacity_on_remove = false;
    entitytainer                   = entitytainer_create( &config );
    do_multi_entity_tests( entitytainer );
    do_save_load_test( entitytainer );

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
