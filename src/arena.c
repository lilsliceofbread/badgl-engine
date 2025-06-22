#include "arena.h"

#include <stddef.h>
#include "defines.h"
#include "platform.h"

void arena_create(Arena* self)
{
    arena_create_sized(self, BGL_ARENA_VIRTUAL_MAX);
}

void arena_create_sized(Arena* self, u32 size)
{
    self->virtual_max = ALIGNED_SIZE(size, BGL_ARENA_BLOCK_SIZE); // to align to page boundaries
    self->physical_max = 0;
    self->cursor = 0;

    self->memory = (u8*)platform_virtual_alloc(self->virtual_max);
    BGL_ASSERT(self->memory != NULL, "allocation of size %lu failed", size);

    BGL_LOG_INFO("created arena of size %lu", self->virtual_max);
}

u8* arena_alloc(Arena* self, u32 size)
{
    BGL_ASSERT(self->memory != NULL, "trying to alloc using freed arena");

    u8* ptr = NULL;
    size = ALIGNED_SIZE(size, sizeof(u8*));

    if(self->cursor + size > self->physical_max)
    {
        u32 physical_alloc_size = ALIGNED_SIZE(size, BGL_ARENA_BLOCK_SIZE);

        BGL_ASSERT(self->physical_max + physical_alloc_size <= self->virtual_max,
                   "arena of size %lu could not allocate size %lu", self->virtual_max, self->physical_max + physical_alloc_size);

        platform_physical_alloc(self->memory + self->physical_max, physical_alloc_size);
        self->physical_max += physical_alloc_size;
    }
    
    ptr = self->memory + self->cursor;
    self->cursor += size;
    return ptr;
}

void arena_free(Arena* self)
{
    BGL_LOG_INFO("freed arena, final physical max %lu, final cursor pos %lu", self->physical_max, self->cursor);

    platform_virtual_dealloc(self->memory, self->virtual_max);
    self->memory = NULL;
    self->virtual_max = 0;
    self->physical_max = 0;
    self->cursor = 0;
}
