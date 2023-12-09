#ifndef BADGL_PLATFORM_H
#define BADGL_PLATFORM_H

#include <stdbool.h>

bool platform_gl_extension_supported(const char* extension);

bool platform_init_vsync();

void platform_toggle_vsync(bool on);

#endif