#ifndef BGL_PLATFORM_H
#define BGL_PLATFORM_H

#include "defines.h"

bool platform_file_exists(const char* filename);

bool platform_gl_extension_supported(const char* extension);

bool platform_init_vsync(void);

void platform_toggle_vsync(bool on);

void platform_reset_time(void);

// returns the time since epoch in seconds
double platform_get_time(void);

#endif