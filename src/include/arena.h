#ifndef BGL_ARENA_H
#define BGL_ARENA_H

/* idea from https://github.com/PixelRifts/c-codebase/blob/master/source/base/mem.c */

#include <stddef.h>
#include "defines.h"

#define KILOBYTES(kb) (u32)((kb) * 1024)
#define MEGABYTES(mb) (u32)((mb) * 1024 * 1024)

/* default arena virtual size and physical allocation size. must be aligned to page boundaries e.g. 4KB */
#define BGL_ARENA_VIRTUAL_MAX MEGABYTES(512)
#define BGL_ARENA_BLOCK_SIZE KILOBYTES(8)

typedef struct Arena {
    u8* memory;
    u32 virtual_max; // maximum value of reserved virtual address space
    u32 physical_max; // maximum value of reserved space backed by physical memory
    u32 cursor; // position where unused memory starts
} Arena;

void arena_create(Arena* arena);

void arena_create_sized(Arena* arena, u32 size);

u8* arena_alloc(Arena* self, u32 size);

void arena_free(Arena* self);

char* arena_read_file(Arena* self, const char* path, u32* file_size_out);



/* probably don't use these, only used for shader parser */
u8* arena_alloc_unaligned(Arena* self, u32 size);

char* arena_read_file_unterminated_unaligned(Arena* self, const char* path, u32* file_size_out);

#endif
