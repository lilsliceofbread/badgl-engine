#include "arena.h"

#include <stdio.h>
#include <string.h>
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
    BGL_ASSERT(self->memory != NULL, "allocation of size %lu KB failed", size / 1024);

    BGL_LOG_INFO("created arena of size %lu KB", self->virtual_max / 1024);
}

u8* arena_alloc_unaligned(Arena* self, u32 size)
{
    BGL_ASSERT(self->memory != NULL, "trying to alloc using freed arena");

    u8* ptr = NULL;

    if(self->cursor + size > self->physical_max)
    {
        u32 physical_alloc_size = ALIGNED_SIZE(size, BGL_ARENA_BLOCK_SIZE);

        BGL_ASSERT(self->physical_max + physical_alloc_size <= self->virtual_max,
                   "arena of size %lu KB could not allocate to position %lu KB", self->virtual_max / 1024, (self->physical_max + physical_alloc_size) / 1024);

        platform_physical_alloc(self->memory + self->physical_max, physical_alloc_size);
        self->physical_max += physical_alloc_size;
    }
    
    ptr = self->memory + self->cursor;
    self->cursor += size;
    return ptr;
}

u8* arena_alloc(Arena* self, u32 size)
{
    size = ALIGNED_SIZE(size, sizeof(u8*));
    return arena_alloc_unaligned(self, size);
}

void arena_free(Arena* self)
{
    BGL_LOG_INFO("freed arena, final physical max %lu KB, final cursor pos %lu B", self->physical_max / 1024, self->cursor);

    platform_virtual_dealloc(self->memory, self->virtual_max);
    self->memory = NULL;
    self->virtual_max = 0;
    self->physical_max = 0;
    self->cursor = 0;
}

char* arena_read_file(Arena* self, const char* path, u32* file_size_out)
{
    FILE* file;
    if(file_size_out != NULL) *file_size_out = 0;
    file = fopen(path, "rb"); // windows will add carriage returns to \n in ftell count unless using binary read
    if(file == NULL || fseek(file, 0, SEEK_END))
    {
        return NULL;
    }

    i64 file_size = (i64)ftell(file); // ftell after seeking end gives file size
    if(file_size == -1)
    {
        return NULL;
    }
    char* file_data = (char*)arena_alloc(self, (u32)file_size + 1); // +1 for the null terminator
    
    fseek(file, 0, SEEK_SET);
    if(file_data == NULL || fread(file_data, sizeof(char), (u32)file_size, file) != (u32)file_size)
    {
        BGL_FREE(file_data); // free on NULL does nothing
        return NULL;
    }
    file_data[file_size] = '\0';
    if(file_size_out != NULL) *file_size_out = (u32)file_size;

    fclose(file);
    return file_data;
}

char* arena_read_file_unterminated_unaligned(Arena* self, const char* path, u32* file_size_out)
{
    FILE* file;
    if(file_size_out != NULL) *file_size_out = 0;
    file = fopen(path, "rb"); // windows will add carriage returns to \n in ftell count unless using binary read
    if(file == NULL || fseek(file, 0, SEEK_END))
    {
        return NULL;
    }

    i64 file_size = (i64)ftell(file); // ftell after seeking end gives file size
    if(file_size == -1)
    {
        return NULL;
    }
    char* file_data = (char*)arena_alloc_unaligned(self, (u32)file_size);
    
    fseek(file, 0, SEEK_SET);
    if(file_data == NULL || fread(file_data, sizeof(char), (u32)file_size, file) != (u32)file_size)
    {
        BGL_FREE(file_data); // free on NULL does nothing
        return NULL;
    }
    if(file_size_out != NULL) *file_size_out = (u32)file_size;

    fclose(file);
    return file_data;
}
