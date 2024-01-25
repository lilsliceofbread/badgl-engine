#ifndef BGL_ARENA_ALLOC_H
#define BGL_ARENA_ALLOC_H

#include <stddef.h>
#include "defines.h"

typedef struct Arena {
    u8* raw_memory;
    u8* cursor; // position where unused memory starts
    size_t size;
} Arena;

Arena arena_create(size_t size);

void* arena_alloc(Arena* self, size_t size);

void arena_free(Arena* self);

#endif