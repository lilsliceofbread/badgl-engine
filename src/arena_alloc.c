#include "arena_alloc.h"

#include <stddef.h>
#include <stdlib.h>
#include "util.h"

Arena arena_create(size_t size)
{
    Arena self;

    //
    self.raw_memory = malloc(size + 1);
    ASSERT(self.raw_memory != NULL, "ARENA: malloc(%lu) failed", size);

    self.cursor = self.raw_memory;
    self.size = size;

    return self; // user will need to check if null
}

void *arena_alloc(Arena* self, size_t size)
{
    if(self->raw_memory == NULL || self->cursor == NULL) return NULL;
    uint8_t* tmp;

    void* new_alloc = self->cursor;
    tmp = (uint8_t*)self->cursor;
    tmp += size;
    self->cursor = (void*)tmp; // arithmetic with void* is bad, do this

    // if cursor is any more than 1 past the end of memory block, we have used unallocated memory
    // e.g. A = allocated, U = unallocated, ^ = ptr
    // good: AAAAAA^ bad: AAAAAU^ 
    size_t cursor_distance = (size_t)self->cursor - (size_t)self->raw_memory; 
    if(cursor_distance > self->size)
    {
        tmp -= size;
        self->cursor = (void*)tmp;
        return NULL;
    }

    return new_alloc;
}

void arena_free(Arena* self)
{
    free(self->raw_memory);
    self->raw_memory = self->cursor = NULL; // prevent further use of the arena
    self->size = (size_t)0;
}