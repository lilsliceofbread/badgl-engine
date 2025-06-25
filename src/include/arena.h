#ifndef BGL_ARENA_H
#define BGL_ARENA_H

/* arena virtual alloc idea from https://github.com/PixelRifts/c-codebase/blob/master/source/base/mem.c */

#include <stddef.h>
#include "defines.h"

#define KILOBYTES(kb) (u32)((kb) * 1024)
#define MEGABYTES(mb) (u32)((mb) * 1024 * 1024)

/* default arena virtual size and physical allocation size. must be aligned to page boundaries e.g. 4KB */
#define BGL_ARENA_VIRTUAL_MAX MEGABYTES(512)
#define BGL_ARENA_BLOCK_SIZE KILOBYTES(8)

typedef struct Arena {
    u8* memory;
    u64 virtual_max; // maximum value of reserved virtual address space
    u64 physical_max; // maximum value of reserved space backed by physical memory
    u64 cursor; // position where unused memory starts
} Arena;

/**
 * @brief create arena of default size
 */
void arena_create(Arena* arena);

/**
 * @brief create arena of size size
 */
void arena_create_sized(Arena* arena, u64 size);

/**
 * @brief allocate memory within arena
 * @returns ptr to memory
 */
u8* arena_alloc(Arena* self, u64 size);

/**
 * @brief free entire arena
 */
void arena_free(Arena* self);

/**
 * @brief free all of arena allocated past ptr
 */
void arena_collapse(Arena* self, u8* ptr);

/**
 * @brief read file and allocate char buffer in arena
 * @returns ptr to char buffer
 */
char* arena_read_file(Arena* self, const char* path, u64* file_size_out);



/* probably don't use these, only used for shader parser */
u8* arena_alloc_unaligned(Arena* self, u64 size);

char* arena_read_file_unterminated_unaligned(Arena* self, const char* path, u64* file_size_out);

#endif
