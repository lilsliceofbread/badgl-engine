#ifndef BADGL_PLATFORM_H
#define BADGL_PLATFORM_H

#include <stdbool.h>

bool platform_gl_extension_supported(const char* extension);

bool platform_init_vsync(void);

void platform_toggle_vsync(bool on);

// returns the time since epoch in seconds
double platform_get_time(void);

#endif