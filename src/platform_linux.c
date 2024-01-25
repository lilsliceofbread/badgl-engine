#include "platform.h"

#ifdef __linux__

#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static struct
{
    int platform_clock;
    double platform_time_offset;
} linux_ctx;

static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
// the GLX_EXT_swap_control extension does not have
// a GetSwapIntervalEXT func, use glXQueryDrawable instead

bool platform_file_exists(const char* filename)
{
    return access(filename, F_OK) == 0;
}

bool platform_gl_extension_supported(const char* extension)
{
    //Display* display = XOpenDisplay(":0");
    Display* display = glXGetCurrentDisplay();
    const char* extension_list = glXQueryExtensionsString(display, 0);

    // if extension not found in list
    if(strstr(extension_list, extension) == NULL) return false;

    return true;
}

bool platform_init_vsync(void)
{
    if(!platform_gl_extension_supported("GLX_EXT_swap_control")) return false;

    glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");

    return true;
}

void platform_toggle_vsync(bool on)
{
    Display* display = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();

    glXSwapIntervalEXT(display, drawable, (int)on);
}

void platform_reset_time(void)
{
    linux_ctx.platform_clock = CLOCK_REALTIME;
    #ifdef _POSIX_MONOTONIC_CLOCK
        struct timespec tmp;
        if(clock_gettime(CLOCK_MONOTONIC, &tmp) == 0)
        {
            linux_ctx.platform_clock = CLOCK_MONOTONIC;
        }
    #endif

    linux_ctx.platform_time_offset = platform_get_time();
}

double platform_get_time(void)
{
    struct timespec os_time;

    clock_gettime(linux_ctx.platform_clock, &os_time);    

    double time = (double)os_time.tv_sec + (double)(0.000000001 * (double)os_time.tv_nsec);
    return time - linux_ctx.platform_time_offset;
}

#endif