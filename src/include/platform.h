#ifndef BGL_PLATFORM_H
#define BGL_PLATFORM_H

#include "types.h"

#define BGL_MAX_EXECUTABLE_DIR_LENGTH 512
#ifdef _WIN32
#define PLATFORM_FILE_SEPARATOR '\\'
#else
#define PLATFORM_FILE_SEPARATOR '/'
#endif

void platform_reset_time(void);

double platform_get_time(void);



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
