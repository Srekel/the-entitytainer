
/*
the_entitytainer.h - v0.01 - public domain - Anders Elfgren @srekel, 2017

# THE ENTITYTAINER

Multimap implementation in C, aimed at game development.

Main purpose is to keep track of hierarchies of entities. This can be useful for attachments (e.g. holding a weapon in
the hand) and inventory (having a piece of cheese in a bag in the backpack on the back of a character) for example.

The name is a pun of entities and containers. If it wasn't obvious.

See github for latest version: https://github.com/Srekel/the-entitytainer

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

#ifdef ENTITYTAINER_IMPLEMENTATION


#ifndef ENTITYTAINER_assert
#include <assert.h>
#define ENTITYTAINER_assert assert;
#endif

#ifndef ENTITYTAINER_memcpy
#include <string.h>
#define ENTITYTAINER_memcpy memcpy
#endif

// #ifndef ENTITYTAINER_memset
// #include <string.h>
// #define ENTITYTAINER_memset memset
// #endif

typedef short TheEntitytainerEntry;
#define TheEntitytainerBucketMask 0x3f
// #define TheEntitytainerBuckMask 0xFFFF & ~0x3;

typedef void* ( *TheEntitytainerAllocatorFunction )( int size );

typedef struct {
    char* data;
    int   bucket_size;
    int   total_buckets;
    int   first_free_bucket;
    int   used_buckets;
    int   max_used_buckets;
} TheEntitytainerBucketList;

typedef struct {
    TheEntitytainerAllocatorFunction allocator_func;
    TheEntitytainerBucketList*           bucket_lists;
    int                           num_bucket_lists;
    TheEntitytainerEntry*            entry_lookup;
    TheEntitytainerEntry*            entry_reverse_lookup;
    int                           entry_lookup_size;
} TheEntitytainer;

void entitytainer_remove_child(TheEntitytainer* entitytainer, TheEntitytainerEntry entry, TheEntitytainerEntry child);

TheEntitytainer*
entitytainer_create( TheEntitytainerAllocatorFunction allocator_func,
                     int                           num_entries,
                     int*                          bucket_sizes,
                     int*                          bucket_list_sizes,
                     int                           num_bucket_lists ) {

    int size_needed = sizeof( TheEntitytainer );
    size_needed += num_entries * sizeof( TheEntitytainerEntry );
    size_needed += num_entries * sizeof( TheEntitytainerEntry );
    size_needed += num_bucket_lists * sizeof( TheEntitytainerBucketList );

    for ( int i = 0; i < num_bucket_lists; ++i ) {
        size_needed += bucket_list_sizes[i] * bucket_sizes[i] * sizeof( TheEntitytainerEntry );
    }

    char* buffer = allocator_func( size_needed );

    TheEntitytainer* entitytainer   = (TheEntitytainer*)buffer;
    entitytainer->allocator_func = allocator_func;
    buffer += sizeof( TheEntitytainer );
    entitytainer->entry_lookup = (TheEntitytainerEntry *)buffer;
    buffer += sizeof( TheEntitytainerEntry ) * num_entries;
    entitytainer->entry_reverse_lookup = (TheEntitytainerEntry *)buffer;
    buffer += sizeof( TheEntitytainerEntry ) * num_entries;

    char* bucket_data = buffer + sizeof( TheEntitytainerBucketList ) * num_bucket_lists;
    for ( int i = 0; i < num_bucket_lists; ++i ) {
        ENTITYTAINER_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
        TheEntitytainerBucketList* list = (TheEntitytainerBucketList*)buffer;
        list->data                   = bucket_data;
        list->bucket_size            = bucket_sizes[i];
        list->total_buckets          = bucket_list_sizes[i];
        list->first_free_bucket      = 0;
        list->used_buckets           = 0;
        list->max_used_buckets       = 0;
        buffer += sizeof( TheEntitytainerBucketList );
        bucket_data += list->bucket_size * list->total_buckets;
    }

    //ENTITYTAINER_assert( bucket_data == buffer + buffer_size );
    return entitytainer;
}

TheEntitytainer*
entitytainer_realloc( TheEntitytainer* entitytainer_old, float growth ) {
    int num_entries = entitytainer_old->entry_lookup_size; // * growth;
    int size_needed = sizeof( TheEntitytainer );
    size_needed += (int)( num_entries * sizeof( TheEntitytainerEntry ) );
    size_needed += (int)( num_entries * sizeof( TheEntitytainerEntry ) );
    size_needed += entitytainer_old->num_bucket_lists * sizeof( TheEntitytainerBucketList );

    for ( int i = 0; i < entitytainer_old->num_bucket_lists; ++i ) {
        TheEntitytainerBucketList* bucket_list = &entitytainer_old->bucket_lists[i];
        int old_bucket_size = bucket_list->total_buckets * bucket_list->bucket_size * sizeof( TheEntitytainerEntry );
        size_needed += (int)( old_bucket_size * growth );
    }

    char* buffer = entitytainer_old->allocator_func( size_needed );

    TheEntitytainer* entitytainer      = (TheEntitytainer*)buffer;
    *entitytainer                   = *entitytainer_old;
    entitytainer->entry_lookup_size = num_entries;
    buffer += sizeof( TheEntitytainer );
    entitytainer->entry_lookup = (TheEntitytainerEntry*)buffer;
    buffer += sizeof( TheEntitytainerEntry ) * num_entries;
    entitytainer->entry_reverse_lookup = (TheEntitytainerEntry*)buffer;
    buffer += sizeof( TheEntitytainerEntry ) * num_entries;

    char* bucket_data = buffer + sizeof( TheEntitytainerBucketList ) * entitytainer_old->num_bucket_lists;
    for ( int i = 0; i < entitytainer_old->num_bucket_lists; ++i ) {
        // ENTITYTAINER_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
        TheEntitytainerBucketList* list = (TheEntitytainerBucketList*)buffer;
        list->data                   = bucket_data;
        list->bucket_size            = entitytainer_old->bucket_lists[i].bucket_size;
        list->total_buckets          = entitytainer_old->bucket_lists[i].total_buckets;
        list->first_free_bucket      = entitytainer_old->bucket_lists[i].first_free_bucket;
        list->used_buckets           = entitytainer_old->bucket_lists[i].used_buckets;
        list->max_used_buckets       = entitytainer_old->bucket_lists[i].max_used_buckets;

        int old_buffer_size = entitytainer_old->bucket_lists[i].total_buckets * sizeof( TheEntitytainerEntry );
        ENTITYTAINER_memcpy( list->data, entitytainer_old->bucket_lists[i].data, old_buffer_size );
        buffer += sizeof( TheEntitytainerBucketList );
        bucket_data += list->bucket_size * list->total_buckets;
    }

    // ENTITYTAINER_assert( bucket_data == buffer + buffer_size );
    return entitytainer;
}

bool
entitytainer_needs_realloc( TheEntitytainer* entitytainer, float percent_free, int num_free_buckets ) {
    for ( int i = 0; i < entitytainer->num_bucket_lists; ++i ) {
        // ENTITYTAINER_assert( bucket_data - buffer > bucket_sizes[i] * bucket_list_sizes[i] ); // >= ?
        TheEntitytainerBucketList* list = &entitytainer->bucket_lists[i];
        if ( percent_free >= 0 ) {
            num_free_buckets = (int)(list->total_buckets * percent_free);
        }

        if ( list->total_buckets - list->used_buckets <= num_free_buckets ) {
            return true;
        }
    }

    return false;
}

void
entitytainer_add_entry( TheEntitytainer* entitytainer, TheEntitytainerEntry entry ) {
    ENTITYTAINER_assert( entitytainer->entry_lookup[entry] == 0 );
    TheEntitytainerBucketList* bucket_list  = &entitytainer->bucket_lists[0];
    int                     bucket_index = bucket_list->first_free_bucket;
    if ( bucket_index == 0 ) {
        bucket_index = bucket_list->used_buckets++;
    }
    else {
        bucket_list->first_free_bucket = *(int*)( bucket_list->data + bucket_index );
    }

    TheEntitytainerEntry* lookup = &entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( *lookup == 0 );
    *lookup              = (TheEntitytainerEntry) bucket_index; // bucket list index is 0
    bucket_list->data[0] = 0;
}

void
entitytainer_remove_entry( TheEntitytainer* entitytainer, TheEntitytainerEntry entry ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( lookup != 0 );
    int                     bucket_list_index = lookup >> ( sizeof( TheEntitytainerEntry ) * 8 - 2 );
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                     bucket_index      = lookup & 0x3f;
    int                     bucket_offset     = bucket_index * bucket_list->bucket_size;
    int*                    bucket            = (int*)( bucket_list->data + bucket_offset );
    *bucket                                   = bucket_list->first_free_bucket;
    bucket_list->first_free_bucket            = bucket_index;
    // ENTITYTAINER_memset( bucket_list->data + bucket_offset, 0, sizeof( TheEntitytainerEntry ) * bucket_list->bucket_size
    // );
    entitytainer->entry_lookup[entry] = 0;

    if ( entitytainer->entry_reverse_lookup[entry] != 0 ) {
		entitytainer_remove_child(entitytainer, entitytainer->entry_reverse_lookup[entry], entry);
    }
}

void
entitytainer_add_child( TheEntitytainer* entitytainer, TheEntitytainerEntry entry, TheEntitytainerEntry child ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( lookup != 0 );
    int                     bucket_list_index = lookup >> ( sizeof( TheEntitytainerEntry ) * 8 - 2 );
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                     bucket_index      = lookup & 0x3f;
    int                     bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntry*      bucket            = (TheEntitytainerEntry*)( bucket_list->data + bucket_offset );
    if ( bucket[0] + 1 == bucket_list->bucket_size ) {
        ASSERT( bucket_list_index != 3 );
        TheEntitytainerBucketList* bucket_list_new   = bucket_list + 1;
        int                    bucket_index_new  = bucket_list->first_free_bucket;
        int                    bucket_offset_new = bucket_index_new * bucket_list_new->bucket_size;
        TheEntitytainerEntry*     bucket_new        = (TheEntitytainerEntry*)( bucket_list_new->data + bucket_offset_new );
        ENTITYTAINER_memcpy( bucket_new, bucket, bucket_list->bucket_size * sizeof( TheEntitytainerEntry ) );

		TheEntitytainerEntry* free_entry = (TheEntitytainerEntry*)
			bucket_list->data + bucket_list->bucket_size * bucket_list->first_free_bucket;
		free_entry = 0; //TODO What goes here?;
		bucket_list->first_free_bucket = bucket_index;
        bucket_list_new->first_free_bucket =
          *(TheEntitytainerEntry*)( bucket_list_new->data +
                                 bucket_list_new->first_free_bucket * bucket_list->bucket_size ); // ?
    }

    bucket[0]++;
    bucket[bucket[0]] = child;
}

void
entitytainer_remove_child( TheEntitytainer* entitytainer, TheEntitytainerEntry entry, TheEntitytainerEntry child ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( lookup != 0 );
    int                     bucket_list_index = lookup >> ( sizeof( TheEntitytainerEntry ) * 8 - 2 );
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                     bucket_index      = lookup & 0x3f;
    int                     bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntry*      bucket            = (TheEntitytainerEntry*)( bucket_list->data + bucket_offset );
    TheEntitytainerBucketList* bucket_list_prev =
      bucket_list_index > 0 ? ( entitytainer->bucket_lists + bucket_list_index - 1 ) : NULL;
    if ( bucket_list_prev != NULL && bucket[0] + 1 == bucket_list_prev->bucket_size ) {
        bucket_list  = bucket_list_prev;
        bucket_index = bucket_list->first_free_bucket;
        bucket_list->first_free_bucket =
          *(TheEntitytainerEntry*)( bucket_list->data + bucket_list->first_free_bucket * bucket_list->bucket_size ); // ?
        bucket_offset = bucket_index * bucket_list->bucket_size;
        bucket        = (TheEntitytainerEntry*)( bucket_list->data + bucket_offset );
    }

	entitytainer->entry_reverse_lookup[child] = 0;
}

void entitytainer_remove_child_by_index( TheEntitytainer* entitytainer, TheEntitytainerEntry entry, int index );

void
entitytainer_get_children( TheEntitytainer*       entitytainer,
                           TheEntitytainerEntry   entry,
                           TheEntitytainerEntry** children,
                           int*                num_children ) {

    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( lookup != 0 );
    int                     bucket_list_index = lookup >> ( sizeof( TheEntitytainerEntry ) * 8 - 2 );
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                     bucket_index      = lookup & 0x3f;
    int                     bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntry*      bucket            = (TheEntitytainerEntry*)( bucket_list->data + bucket_offset );
    *num_children                             = (int)bucket[0];
    *children                                 = bucket + 1;
}

int
entitytainer_num_children( TheEntitytainer* entitytainer, TheEntitytainerEntry entry ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( lookup != 0 );
    int                     bucket_list_index = lookup >> ( sizeof( TheEntitytainerEntry ) * 8 - 2 );
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                     bucket_index      = lookup & 0x3f;
    int                     bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntry*      bucket            = (TheEntitytainerEntry*)( bucket_list->data + bucket_offset );
    return (int)bucket[0];
}

int
entitytainer_get_child_index( TheEntitytainer* entitytainer, TheEntitytainerEntry entry, TheEntitytainerEntry child ) {
    TheEntitytainerEntry lookup = entitytainer->entry_lookup[entry];
    ENTITYTAINER_assert( lookup != 0 );
    int                     bucket_list_index = lookup >> ( sizeof( TheEntitytainerEntry ) * 8 - 2 );
    TheEntitytainerBucketList* bucket_list       = entitytainer->bucket_lists + bucket_list_index;
    int                     bucket_index      = lookup & 0x3f;
    int                     bucket_offset     = bucket_index * bucket_list->bucket_size;
    TheEntitytainerEntry*      bucket            = (TheEntitytainerEntry*)( bucket_list->data + bucket_offset );
    int                     num_children      = (int)bucket[0];
    for ( int i = 0; i < num_children; ++i ) {
        if ( bucket[1 + i] == child ) {
            return i;
        }
    }

	return -1;
}

TheEntitytainerEntry
entitytainer_get_parent( TheEntitytainer* entitytainer, TheEntitytainerEntry entry ) {
    TheEntitytainerEntry parent = entitytainer->entry_reverse_lookup[entry];
    return parent;
}

#endif // ENTITYTAINER_IMPLEMENTATION
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
