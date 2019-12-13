/* clang-format off */

/*
the_entitytainer.h - v0.01 - public domain - Anders Elfgren @srekel, 2019

# THE ENTITYTAINER

"Multimap" implementation in C, aimed at game development.

Main purpose is to keep track of hierarchies of entities. This can be useful for attachments (e.g. holding a weapon in
the hand) and inventory (having a piece of cheese in a bag in the backpack on the back of a character) for example.

The name is a pun of entities and containers. If it wasn't obvious.

See github for detailed documentation and the latest version: https://github.com/Srekel/the-entitytainer

## Usage

In *ONE* source file, put:

```C
#define ENTITYTAINER_IMPLEMENTATION

// Define any of these if you wish to override them.
// (There are more. Find them in the beginning of the code.)
#define ENTITYTAINER_assert
#define ENTITYTAINER_memcpy

#include "the_entitytainer.h"
```

Other source files should just include the_entitytainer.h

I recommend looking at the unittest.c file for an example of how to use it, but basically:


```C
    int   max_num_entries     = 1024;
    int   bucket_sizes[]      = { 4, 16, 256 };
    int   bucket_list_sizes[] = { 4, 2, 2 };
    int   needed_memory_size  = entitytainer_needed_size( max_num_entries, bucket_sizes, bucket_list_sizes, 3 );
    void* memory              = malloc( needed_memory_size );
    TheEntitytainer* entitytainer =
      entitytainer_create( memory, needed_memory_size, max_num_entries, bucket_sizes, bucket_list_sizes, 3 );

    entitytainer_add_entity( entitytainer, 3 );
    entitytainer_add_child( entitytainer, 3, 10 );

    int                    num_children;
    TheEntitytainerEntity* children;
    entitytainer_get_children( entitytainer, 3, &children, &num_children );
    ASSERT( num_children == 1 );
    ASSERT( children[0] == 10 );
```

## Notes

See the accompanying unit test projects for references on how to use it.
Or the documentation on the github page.

## References and related stuff

* https://github.com/nothings/stb
* https://github.com/incrediblejr/ijhandlealloc

## License

Public Domain / MIT.
See end of file for license information.

*/

#ifndef INCLUDE_THE_ENTITYTAINER_H
#define INCLUDE_THE_ENTITYTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENTITYTAINER_ENABLE_WARNINGS
#ifdef _MSC_VER
#pragma warning( push, 0 )
#pragma warning( disable : 4365 )
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
#endif // ENTITYTAINER_ENABLE_WARNINGS

#ifdef ENTITYTAINER_IMPLEMENTATION

#ifndef ENTITYTAINER_assert
#include <assert.h>
#define ENTITYTAINER_assert assert;
#endif

#ifndef ENTITYTAINER_memcpy
#include <string.h>
#define ENTITYTAINER_memcpy memcpy
#endif

#ifndef ENTITYTAINER_memset
#include <string.h>
#define ENTITYTAINER_memset memset
#endif

#ifndef ENTITYTAINER_Entity
typedef unsigned short TheEntitytainerEntity;
#define ENTITYTAINER_InvalidEntity ( (TheEntitytainerEntity)0u )
#endif

#ifndef ENTITYTAINER_Entry
typedef unsigned short TheEntitytainerEntry;
#define ENTITYTAINER_BucketMask 0x3fff
#define ENTITYTAINER_BucketListBitCount 2
#define ENTITYTAINER_BucketListOffset ( sizeof( TheEntitytainerEntry ) * 8 - ENTITYTAINER_BucketListBitCount )
#endif

#define ENTITYTAINER_NoFreeBucket ( (TheEntitytainerEntity)-1 )
#define ENTITYTAINER_ShrinkMargin 1

#if defined( ENTITYTAINER_STATIC )
#define ENTITYTAINER_API static
#else
#define ENTITYTAINER_API extern
#endif

struct TheEntitytainerConfig {
    void* memory;
    int   memory_size;
    int   num_entries;
    int*  bucket_sizes;
    int*  bucket_list_sizes;
    int   num_bucket_lists;
    bool  remove_with_holes;
    bool  keep_capacity_on_remove;
};

typedef struct {
    TheEntitytainerEntity* bucket_data;
    int                    bucket_size;
    int                    total_buckets;
    int                    first_free_bucket;
    int                    used_buckets;
} TheEntitytainerBucketList;

typedef struct {
    TheEntitytainerEntry*      entry_lookup;
    TheEntitytainerEntity*     entry_parent_lookup;
    TheEntitytainerBucketList* bucket_lists;
    int                        num_bucket_lists;
    int                        entry_lookup_size;
    bool                       remove_with_holes;
    bool                       keep_capacity_on_remove;
} TheEntitytainer;

ENTITYTAINER_API void entitytainer_remove_child_no_holes( TheEntitytainer*      entitytainer,
                                                          TheEntitytainerEntity parent,
                                                          TheEntitytainerEntity child );

ENTITYTAINER_API void entitytainer_remove_child_with_holes( TheEntitytainer*      entitytainer,
                                                            TheEntitytainerEntity parent,
                                                            TheEntitytainerEntity child );

ENTITYTAINER_API int
entitytainer_needed_size( int num_entries, int* bucket_sizes, int* bucket_list_sizes, int num_bucket_lists ) {
    int size_needed = sizeof( TheEntitytainer );
    size_needed += num_entries * sizeof( TheEntitytainerEntry );           // Lookup
    size_needed += num_entries * sizeof( TheEntitytainerEntity );          // Reverse lookup
    size_needed += num_bucket_lists * sizeof( TheEntitytainerBucketList ); // List structs

    // Bucket lists
    for ( int i = 0; i < num_bucket_lists; ++i ) {
        size_needed += bucket_list_sizes[i] * bucket_sizes[i] * sizeof( TheEntitytainerEntity );
    }

    return size_needed;
}

ENTITYTAINER_API TheEntitytainer*
                 entitytainer_create( struct TheEntitytainerConfig* config ) {

    char* buffer_start = (char*)config->memory;
    char* buffer       = buffer_start;
    ENTITYTAINER_memset( buffer, 0, config->memory_size );

    TheEntitytainer* entitytainer         = (TheEntitytainer*)buffer;
    entitytainer->num_bucket_lists        = config->num_bucket_lists;
    entitytainer->remove_with_holes       = config->remove_with_holes;
    entitytainer->keep_capacity_on_remove = config->keep_capacity_on_remove;

    buffer += sizeof( TheEntitytainer );
    entitytainer->entry_lookup = (TheEntitytainerEntry*)buffer;
    buffer += sizeof( TheEntitytainerEntry ) * config->num_entries;
    entitytainer->entry_parent_lookup = (TheEntitytainerEntity*)buffer;
    buffer += sizeof( TheEntitytainerEntity ) * config->num_entries;
    entitytainer->bucket_lists = (TheEntitytainerBucketList*)buffer;

    char*                  bucket_list_end   = buffer + sizeof( TheEntitytainerBucketList ) * config->num_bucket_lists;
    TheEntitytainerEntity* bucket_data_start = (TheEntitytainerEntity*)bucket_list_end;
    TheEntitytainerEntity* bucket_data       = bucket_data_start;
    for ( int i = 0; i < config->num_bucket_lists; ++i ) {
        // Just making sure that we don't go into the bucket data area
        ENTITYTAINER_assert( buffer + sizeof( TheEntitytainerBucketList ) <= bucket_list_end );

        // We need to do this because first_free_bucket is stored as an int.
        ENTITYTAINER_assert( config->bucket_sizes[i] * sizeof( TheEntitytainerEntity ) >= sizeof( int ) );

        TheEntitytainerBucketList* list = (TheEntitytainerBucketList*)buffer;
        list->bucket_data               = bucket_data;
        list->bucket_size               = config->bucket_sizes[i];
        list->total_buckets             = config->bucket_list_sizes[i];
        list->first_free_bucket         = ENTITYTAINER_NoFreeBucket;
        list->used_buckets              = 0;

        if ( i == 0 ) {
            // We need this in order to ensure that we can use 0 as the default "invalid" entry.
            list->used_buckets = 1;
        }

        buffer += sizeof( TheEntitytainerBucketList );
        bucket_data += list->bucket_size * list->total_buckets;
    }

    ENTITYTAINER_assert( *bucket_data_start == 0 );
    ENTITYTAINER_assert( (char*)bucket_data == buffer_start + config->memory_size );
    return entitytainer;
}

ENTITYTAINER_API TheEntitytainer*
                 entitytainer_realloc( TheEntitytainer* entitytainer_old, void* memory, int memory_size, float growth ) {
    ENTITYTAINER_assert( false ); // Not yet implemented
    (void)memory_size;

    int num_entries = entitytainer_old->entry_lookup_size; // * growth;
    int size_needed = sizeof( TheEntitytainer );
    size_needed += (int)( num_entries * sizeof( TheEntitytainerEntry ) );
    size_needed += (int)( num_entries * sizeof( TheEntitytainerEntity ) );
    size_needed += entitytainer_old->num_bucket_lists * sizeof( TheEntitytainerBucketList );

    for ( int i = 0; i < entitytainer_old->num_bucket_lists; ++i ) {
        TheEntitytainerBucketList* bucket_list = &entitytainer_old->bucket_lists[i];
        int old_bucket_size = bucket_list->total_buckets * bucket_list->bucket_size * sizeof( TheEntitytainerEntity );
        size_needed += (int)( old_bucket_size * growth );
    }

    char* buffer = (char*)memory;

    TheEntitytainer* entitytainer   = (TheEntitytainer*)buffer;
    *entitytainer                   = *entitytainer_old;
    entitytainer->entry_lookup_size = num_entries;
    buffer += sizeof( TheEntitytainer );
    entitytainer->entry_lookup = (TheEntitytainerEntry*)buffer;
    buffer += sizeof( TheEntitytainerEntry ) * num_entries;
    entitytainer->entry_parent_lookup = (TheEntitytainerEntity*)buffer;
    buffer += sizeof( TheEntitytainerEntity ) * num_entries;

    // char* bucket_data = buffer + sizeof( TheEntitytainerBucketList ) * entitytainer_old->num_bucket_lists;
    // for ( int i = 0; i < entitytainer_old->num_bucket_lists; ++i ) {
    //     // ENTITYTAINER_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
    //     TheEntitytainerBucketList* list = (TheEntitytainerBucketList*)buffer;
    //     list->bucket_data                   = bucket_data;
    //     list->bucket_size               = entitytainer_old->bucket_lists[i].bucket_size;
    //     list->total_buckets             = entitytainer_old->bucket_lists[i].total_buckets;
    //     list->first_free_bucket         = entitytainer_old->bucket_lists[i].first_free_bucket;
    //     list->used_buckets              = entitytainer_old->bucket_lists[i].used_buckets;

    //     int old_buffer_size = entitytainer_old->bucket_lists[i].total_buckets * sizeof( TheEntitytainerEntry );
    //     ENTITYTAINER_memcpy( list->bucket_data, entitytainer_old->bucket_lists[i].bucket_data, old_buffer_size );
    //     buffer += sizeof( TheEntitytainerBucketList );
    //     bucket_data += list->bucket_size * list->total_buckets;
    // }

    // ENTITYTAINER_assert( bucket_data == buffer + buffer_size );
    return entitytainer;
}

ENTITYTAINER_API bool
entitytainer_needs_realloc( TheEntitytainer* entitytainer, float percent_free, int num_free_buckets ) {
    for ( int i = 0; i < entitytainer->num_bucket_lists; ++i ) {
        // ENTITYTAINER_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
        TheEntitytainerBucketList* list = &entitytainer->bucket_lists[i];
        if ( percent_free >= 0 ) {
            num_free_buckets = (int)( list->total_buckets * percent_free );
        }

        if ( list->total_buckets - list->used_buckets <= num_free_buckets ) {
            return true;
        }
    }

    return false;
}

ENTITYTAINER_API void
entitytainer_add_entity( TheEntitytainer* entitytainer, TheEntitytainerEntity entity ) {
    ENTITYTAINER_assert( entitytainer->entry_lookup[entity] == 0 );

    TheEntitytainerBucketList* bucket_list  = &entitytainer->bucket_lists[0];
    int                        bucket_index = bucket_list->used_buckets;
    if ( bucket_list->first_free_bucket != ENTITYTAINER_NoFreeBucket ) {
        // There's a freed bucket available
        bucket_index                   = bucket_list->first_free_bucket;
        int bucket_offset              = bucket_index * bucket_list->bucket_size;
        bucket_list->first_free_bucket = bucket_list->bucket_data[bucket_offset];
    }

    // TODO: Move to larger bucket list if this one is full
    ASSERT( bucket_list->used_buckets < bucket_list->total_buckets );
    ++bucket_list->used_buckets;

    TheEntitytainerEntry* lookup = &entitytainer->entry_lookup[entity];
    ENTITYTAINER_assert( *lookup == 0 );
    *lookup = (TheEntitytainerEntry)bucket_index; // bucket list index is 0

    int                    bucket_offset = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity* bucket        = bucket_list->bucket_data + bucket_offset;
    ENTITYTAINER_memset( bucket, 0, sizeof( TheEntitytainerEntity ) * bucket_list->bucket_size );
}

ENTITYTAINER_API void
entitytainer_remove_entity( TheEntitytainer* entitytainer, TheEntitytainerEntity entity ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entity];

    if ( entitytainer->entry_parent_lookup[entity] != 0 ) {
        if ( entitytainer->remove_with_holes ) {
            entitytainer_remove_child_with_holes( entitytainer, entitytainer->entry_parent_lookup[entity], entity );
        }
        else {
            entitytainer_remove_child_no_holes( entitytainer, entitytainer->entry_parent_lookup[entity], entity );
        }

        lookup = entitytainer->entry_lookup[entity];
    }

    if ( lookup == 0 ) {
        // lookup is 0 for entities that don't have children (or haven't been added by _add_entity)
        return;
    }

    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = bucket_list->bucket_data + bucket_offset;
    *bucket                                      = 0xffff; // TODO remove
    *bucket                                      = (TheEntitytainerEntity)bucket_list->first_free_bucket;
    bucket_list->first_free_bucket               = bucket_index;

    entitytainer->entry_lookup[entity] = 0;
    --bucket_list->used_buckets;
}

ENTITYTAINER_API void
entitytainer_reserve( TheEntitytainer* entitytainer, TheEntitytainerEntity parent, int capacity ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = bucket_list->bucket_data + bucket_offset;
    if ( bucket_list->bucket_size > capacity ) {
        return;
    }

    TheEntitytainerBucketList* bucket_list_new = NULL;
    for ( int i_bl = 0; i_bl < entitytainer->num_bucket_lists; ++i_bl ) {
        bucket_list_new = &entitytainer->bucket_lists[i_bl];
        if ( bucket_list_new->bucket_size > capacity ) {
            break;
        }
    }

    ASSERT( bucket_list_new != NULL && bucket_list_new->bucket_size > capacity );

    int bucket_index_new = bucket_list_new->used_buckets;
    if ( bucket_list_new->first_free_bucket != ENTITYTAINER_NoFreeBucket ) {
        // There's a freed bucket available
        bucket_index_new                   = bucket_list_new->first_free_bucket;
        int bucket_offset_new              = bucket_index_new * bucket_list_new->bucket_size;
        bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
    }

    int                    bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
    TheEntitytainerEntity* bucket_new        = bucket_list_new->bucket_data + bucket_offset_new;
    ENTITYTAINER_memcpy( bucket_new, bucket, bucket_list->bucket_size * sizeof( TheEntitytainerEntity ) );

    *bucket                        = (TheEntitytainerEntity)bucket_list->first_free_bucket;
    bucket_list->first_free_bucket = bucket_index;
    bucket                         = bucket_new;

    bucket_list_new->used_buckets++;
    bucket_list->used_buckets--;

    // Update lookup
    int                  bucket_list_index_new = ( bucket_list_index + 1 ) << ENTITYTAINER_BucketListOffset;
    TheEntitytainerEntry lookup_new            = (TheEntitytainerEntry)bucket_list_index_new;
    lookup_new                                 = lookup_new | (TheEntitytainerEntry)bucket_index_new;
    entitytainer->entry_lookup[parent]         = lookup_new;
}

ENTITYTAINER_API void
entitytainer_add_child( TheEntitytainer* entitytainer, TheEntitytainerEntity parent, TheEntitytainerEntity child ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = bucket_list->bucket_data + bucket_offset;
    if ( bucket[0] + 1 == bucket_list->bucket_size ) {
        ASSERT( bucket_list_index != 3 );
        TheEntitytainerBucketList* bucket_list_new  = bucket_list + 1;
        int                        bucket_index_new = bucket_list_new->used_buckets;
        if ( bucket_list_new->first_free_bucket != ENTITYTAINER_NoFreeBucket ) {
            // There's a freed bucket available
            bucket_index_new                   = bucket_list_new->first_free_bucket;
            int bucket_offset_new              = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        int                    bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        TheEntitytainerEntity* bucket_new        = bucket_list_new->bucket_data + bucket_offset_new;
        ENTITYTAINER_memcpy( bucket_new, bucket, bucket_list->bucket_size * sizeof( TheEntitytainerEntity ) );

        *bucket                        = (TheEntitytainerEntity)bucket_list->first_free_bucket;
        bucket_list->first_free_bucket = bucket_index;
        bucket                         = bucket_new;

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        // Update lookup
        int                  bucket_list_index_new = ( bucket_list_index + 1 ) << ENTITYTAINER_BucketListOffset;
        TheEntitytainerEntry lookup_new            = (TheEntitytainerEntry)bucket_list_index_new;
        lookup_new                                 = lookup_new | (TheEntitytainerEntry)bucket_index_new;
        entitytainer->entry_lookup[parent]         = lookup_new;
    }

    // Update count and insert child into bucket
    TheEntitytainerEntity count = bucket[0] + (TheEntitytainerEntity)1;
    bucket[0]                   = count;
    if ( entitytainer->remove_with_holes ) {
        int i = 1;
        for ( ; i < count; ++i ) {
            if ( bucket[i] == ENTITYTAINER_InvalidEntity ) {
                bucket[i] = child;
                break;
            }
        }
        if ( i == count ) {
            // Didn't find a "holed" slot, add child to the end.
            bucket[i] = child;
        }
    }
    else {
        bucket[count] = child;
    }

    entitytainer->entry_parent_lookup[child] = parent;
}

ENTITYTAINER_API void
entitytainer_add_child_at_index( TheEntitytainer*      entitytainer,
                                 TheEntitytainerEntity parent,
                                 TheEntitytainerEntity child,
                                 int                   index ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = bucket_list->bucket_data + bucket_offset;
    while ( index + 1 >= bucket_list->bucket_size ) {
        ASSERT( bucket_list_index != 3 );
        TheEntitytainerBucketList* bucket_list_new  = bucket_list + 1;
        int                        bucket_index_new = bucket_list_new->used_buckets;
        if ( bucket_list_new->first_free_bucket != ENTITYTAINER_NoFreeBucket ) {
            // There's a freed bucket available
            bucket_index_new                   = bucket_list_new->first_free_bucket;
            int bucket_offset_new              = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        int                    bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        TheEntitytainerEntity* bucket_new        = bucket_list_new->bucket_data + bucket_offset_new;
        ENTITYTAINER_memcpy( bucket_new, bucket, bucket_list->bucket_size * sizeof( TheEntitytainerEntity ) );

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        bucket      = bucket_new;
        bucket_list = bucket_list_new;
        bucket_list_index += 1;

        // Update lookup
        int                  bucket_list_index_new = ( bucket_list_index ) << ENTITYTAINER_BucketListOffset;
        TheEntitytainerEntry lookup_new            = (TheEntitytainerEntry)bucket_list_index_new;
        lookup_new                                 = lookup_new | (TheEntitytainerEntry)bucket_index_new;
        entitytainer->entry_lookup[parent]         = lookup_new;
    }

    // Update count and insert child into bucket
    ASSERT( bucket[index + 1] == ENTITYTAINER_InvalidEntity );
    TheEntitytainerEntity count = bucket[0] + (TheEntitytainerEntity)1;
    bucket[0]                   = count;
    bucket[index + 1]           = child;

    entitytainer->entry_parent_lookup[child] = parent;
}

ENTITYTAINER_API void
entitytainer_remove_child_no_holes( TheEntitytainer*      entitytainer,
                                    TheEntitytainerEntity parent,
                                    TheEntitytainerEntity child ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = (TheEntitytainerEntity*)( bucket_list->bucket_data + bucket_offset );

    // Remove child from bucket, move children after forward one step.
    int                    num_children  = bucket[0];
    TheEntitytainerEntity* child_to_move = &bucket[1];
    int                    count         = 0;
    while ( *child_to_move != child && count < num_children ) {
        ++count;
        ++child_to_move;
    }

    ASSERT( count < num_children );

    for ( ; count < num_children - 1; ++count ) {
        *child_to_move = *( child_to_move + 1 );
        ++child_to_move;
    }

    // Lower child count, clear entry
    bucket[0]--;
    entitytainer->entry_parent_lookup[child] = 0;

    if ( entitytainer->keep_capacity_on_remove ) {
        return;
    }

    TheEntitytainerBucketList* bucket_list_prev =
      bucket_list_index > 0 ? ( entitytainer->bucket_lists + bucket_list_index - 1 ) : NULL;
    if ( bucket_list_prev != NULL && bucket[0] + 1 == bucket_list_prev->bucket_size ) {
        TheEntitytainerBucketList* bucket_list_new  = bucket_list_prev;
        int                        bucket_index_new = bucket_list_new->used_buckets;
        if ( bucket_list_new->first_free_bucket != ENTITYTAINER_NoFreeBucket ) {
            // There's a freed bucket available
            bucket_index_new                   = bucket_list_new->first_free_bucket;
            int bucket_offset_new              = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        int                    bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        TheEntitytainerEntity* bucket_new        = bucket_list_new->bucket_data + bucket_offset_new;
        ENTITYTAINER_memcpy( bucket_new, bucket, bucket_list_new->bucket_size * sizeof( TheEntitytainerEntity ) );

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        // Update lookup
        int                  bucket_list_index_new = ( bucket_list_index - 1 ) << ENTITYTAINER_BucketListOffset;
        TheEntitytainerEntry lookup_new            = (TheEntitytainerEntry)bucket_list_index_new;
        lookup_new                                 = lookup_new | (TheEntitytainerEntry)bucket_index_new;
        entitytainer->entry_lookup[parent]         = lookup_new;
    }
}

ENTITYTAINER_API void
entitytainer_remove_child_with_holes( TheEntitytainer*      entitytainer,
                                      TheEntitytainerEntity parent,
                                      TheEntitytainerEntity child ) {
    ENTITYTAINER_assert( entitytainer->remove_with_holes );
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = (TheEntitytainerEntity*)( bucket_list->bucket_data + bucket_offset );

    // Remove child from bucket, move children after forward one step.
    int capacity            = bucket_list->bucket_size;
    int last_child_index    = 0;
    int child_to_move_index = 0;
    for ( int i = 1; i < capacity; i++ ) {
        if ( bucket[i] == child ) {
            child_to_move_index = i;
        }
        else if ( bucket[i] != ENTITYTAINER_InvalidEntity ) {
            last_child_index = i;
        }
    }

    ASSERT( child_to_move_index != 0 );
    bucket[child_to_move_index] = ENTITYTAINER_InvalidEntity;

    // Lower child count, clear entry
    bucket[0]--;
    entitytainer->entry_parent_lookup[child] = 0;

    if ( entitytainer->keep_capacity_on_remove ) {
        return;
    }

    TheEntitytainerBucketList* bucket_list_prev =
      bucket_list_index > 0 ? ( entitytainer->bucket_lists + bucket_list_index - 1 ) : NULL;
    if ( bucket_list_prev != NULL && last_child_index + ENTITYTAINER_ShrinkMargin < bucket_list_prev->bucket_size ) {
        // We've shrunk enough to fit in the previous bucket, move.
        TheEntitytainerBucketList* bucket_list_new   = bucket_list_prev;
        int                        bucket_index_new  = bucket_list_new->used_buckets;
        int                        bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        if ( bucket_list_new->first_free_bucket != ENTITYTAINER_NoFreeBucket ) {
            // There's a freed bucket available
            bucket_index_new                   = bucket_list_new->first_free_bucket;
            bucket_offset_new                  = bucket_index_new * bucket_list_new->bucket_size;
            bucket_list_new->first_free_bucket = bucket_list_new->bucket_data[bucket_offset_new];
        }

        TheEntitytainerEntity* bucket_new = bucket_list_new->bucket_data + bucket_offset_new;
        ENTITYTAINER_memcpy( bucket_new, bucket, bucket_list_new->bucket_size * sizeof( TheEntitytainerEntity ) );

        bucket_list_new->used_buckets++;
        bucket_list->used_buckets--;

        // Update lookup
        int                  bucket_list_index_new = ( bucket_list_index - 1 ) << ENTITYTAINER_BucketListOffset;
        TheEntitytainerEntry lookup_new            = (TheEntitytainerEntry)bucket_list_index_new;
        lookup_new                                 = lookup_new | (TheEntitytainerEntry)bucket_index_new;
        entitytainer->entry_lookup[parent]         = lookup_new;
    }
}

ENTITYTAINER_API void
entitytainer_get_children( TheEntitytainer*        entitytainer,
                           TheEntitytainerEntity   parent,
                           TheEntitytainerEntity** children,
                           int*                    num_children,
                           int*                    capacity ) {

    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = (TheEntitytainerEntity*)( bucket_list->bucket_data + bucket_offset );
    *num_children                                = (int)bucket[0];
    *children                                    = bucket + 1;
    *capacity                                    = bucket_list->bucket_size - 1;
}

ENTITYTAINER_API int
entitytainer_num_children( TheEntitytainer* entitytainer, TheEntitytainerEntity parent ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = (TheEntitytainerEntity*)( bucket_list->bucket_data + bucket_offset );
    return (int)bucket[0];
}

ENTITYTAINER_API int
entitytainer_get_child_index( TheEntitytainer*      entitytainer,
                              TheEntitytainerEntity parent,
                              TheEntitytainerEntity child ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[parent];
    ENTITYTAINER_assert( lookup != 0 );
    int                        bucket_list_index = lookup >> ENTITYTAINER_BucketListOffset;
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                        bucket_index      = lookup & ENTITYTAINER_BucketMask;
    int                        bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntity*     bucket            = (TheEntitytainerEntity*)( bucket_list->bucket_data + bucket_offset );
    int                        num_children      = (int)bucket[0];
    for ( int i = 0; i < num_children; ++i ) {
        if ( bucket[1 + i] == child ) {
            return i;
        }
    }

    return -1;
}

ENTITYTAINER_API TheEntitytainerEntity
                 entitytainer_get_parent( TheEntitytainer* entitytainer, TheEntitytainerEntity child ) {
    TheEntitytainerEntity parent = entitytainer->entry_parent_lookup[child];
    return parent;
}

ENTITYTAINER_API bool
entitytainer_is_added( TheEntitytainer* entitytainer, TheEntitytainerEntity entity ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entity];
    return lookup != 0;
}

#endif // ENTITYTAINER_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#ifndef DEBUGINATOR_ENABLE_WARNINGS
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // DEBUGINATOR_ENABLE_WARNINGS

#endif // INCLUDE_THE_ENTITYTAINER_H

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Anders Elfgren
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

/* clang-format on */
