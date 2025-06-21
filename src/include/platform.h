#ifndef BGL_PLATFORM_H
#define BGL_PLATFORM_H

#include "types.h"

#define BGL_MAX_EXECUTABLE_DIR_LENGTH 512

void platform_init(void);

void platform_prepend_executable_directory(char* buffer, u32 length, const char* path);

void platform_get_executable_path(char* buffer, u32 length);

bool platform_file_exists(const char* filename);

bool platform_gl_extension_supported(const char* extension);

bool platform_init_vsync(void);

void platform_toggle_vsync(bool on);

void platform_reset_time(void);

double platform_get_time(void);

#endif