#ifndef BADGL_ARENA_ALLOC_H
#define BADGL_ARENA_ALLOC_H

#include <inttypes.h>
#include <stddef.h>

typedef struct Arena {
    void* raw_memory;
    void* cursor; // position where unused memory starts
    size_t size;
} Arena;

Arena arena_create(size_t size);

void* arena_alloc(Arena* self, size_t size);

void arena_free(Arena* self);

#endif