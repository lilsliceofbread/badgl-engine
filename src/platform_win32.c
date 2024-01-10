#include "platform.h"

#ifdef _WIN32

#include "glad/gl.h"
#include <windows.h>
#include "wglext.h"
#include <string.h>

static LARGE_INTEGER os_freq = 1000000000;
static double platform_time_offset = 0.0;

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
//static PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

bool platform_gl_extension_supported(const char* extension)
{
    PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = NULL;

    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

    const char* extension_list = wglGetExtensionsStringEXT();

    // if extension not found in list
    if(strstr(extension_list, extension) == NULL) return false;

    return true;
}

bool platform_init_vsync(void)
{
    if(!platform_gl_extension_supported("WGL_EXT_swap_control")) return false;

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
    //wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");

    return true;
}

void platform_toggle_vsync(bool on)
{
    wglSwapIntervalEXT((int)on);
}

void platform_reset_time(void)
{
    QueryPerformanceFrequency(&os_freq);
    platform_time_offset = platform_get_time();
}

double platform_get_time(void)
{
    LARGE_INTEGER os_time;

    QueryPerformanceCounter(&os_time);

    double time = (double)os_time.QuadPart / (double)os_freq.QuadPart;
    return time - platform_time_offset;
}


#endif