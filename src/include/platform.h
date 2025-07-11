#ifndef BGL_PLATFORM_H
#define BGL_PLATFORM_H

#include <stdio.h>
#include "types.h"

#ifndef BGL_NO_DEBUG
#if defined(__linux__)
    #define BGL_EXIT() __builtin_trap()
#elif defined(_WIN32)
    #define BGL_EXIT() __debugbreak()
#endif
#else
    #define BGL_EXIT() exit(-1)
#endif

#if defined(__linux__)
    #define PLATFORM_FILE_SEPARATOR '/'
    #define BGL_EXPORT
    typedef enum BGLColour
    {
        BGL_RED = 91,
        BGL_YELLOW = 93,
        BGL_MAGENTA = 95,
    } BGLColour;
#elif defined(_WIN32)
    #define PLATFORM_FILE_SEPARATOR '\\'
    #define BGL_EXPORT __declspec(dllexport)
    typedef enum BGLColour
    {
        BGL_RED = 0x8 | 0x4,
        BGL_YELLOW = 0x8 | 0x4 | 0x2,
        BGL_MAGENTA = 0x8 | 0x4 | 0x1,
    } BGLColour;
#endif

#define BGL_MAX_EXECUTABLE_DIR_LENGTH 512

void platform_reset_time(void);

double platform_get_time(void);

void platform_print_coloured(FILE* file, const char* str, BGLColour colour);

/**
 * internal functions
 */
void platform_init(void);

void* platform_virtual_alloc(u64 size); // allocate in virtual address space

void platform_physical_alloc(void* ptr, u64 size); // commit a section of virtual address space to physical memory

void platform_physical_free(void* ptr, u64 size); // decommit

void platform_virtual_free(void* ptr, u64 size); // unreserve virtual address space

void platform_prepend_executable_directory(char* buffer, u32 length, const char* path);

void platform_get_executable_path(char* buffer, u32 length);

bool platform_file_exists(const char* filename);

bool platform_gl_extension_supported(const char* extension);

bool platform_init_vsync(void);

void platform_toggle_vsync(bool on);

#endif
