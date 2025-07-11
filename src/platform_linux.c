#include "platform.h"

#ifdef __linux__

#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "defines.h"
#include "util.h"

static struct
{
    i32 clock;
    f64 time_offset;
    char directory[BGL_MAX_EXECUTABLE_DIR_LENGTH];
} linux_ctx;

static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;

void platform_init(void)
{
    char exe_path[BGL_MAX_EXECUTABLE_DIR_LENGTH];
    memset(exe_path, 0, BGL_MAX_EXECUTABLE_DIR_LENGTH);
    platform_get_executable_path(exe_path, BGL_MAX_EXECUTABLE_DIR_LENGTH);
    find_directory_from_path(linux_ctx.directory, BGL_MAX_EXECUTABLE_DIR_LENGTH, exe_path);
    
    platform_reset_time();
}

void platform_print_coloured(FILE* file, const char* str, BGLColour colour)
{
    fprintf(file, "\x1B[%dm%s\x1B[0m", (i32)colour, str);
}

void* platform_virtual_alloc(u64 size)
{
    return mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void platform_physical_alloc(void* ptr, u64 size)
{
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

void platform_physical_free(void* ptr, u64 size)
{
    mprotect(ptr, size, PROT_NONE);
}

void platform_virtual_free(void* ptr, u64 size)
{
    munmap(ptr, size);
}

void platform_prepend_executable_directory(char* buffer, u32 length, const char* path)
{
    if(strstr(path, linux_ctx.directory)) // if already full path
    {
        strncpy(buffer, path, length);
        return;
    }
    
    snprintf(buffer, length, "%s/%s", linux_ctx.directory, path);
}

void platform_get_executable_path(char* buffer, u32 length)
{
    bool success = readlink("/proc/self/exe", buffer, length) != -1;
    BGL_ASSERT(success, "unable to get executable directory. errno: %d", errno);
}

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
    linux_ctx.clock = CLOCK_REALTIME;
    #ifdef _POSIX_MONOTONIC_CLOCK
        struct timespec tmp;
        if(clock_gettime(CLOCK_MONOTONIC, &tmp) == 0)
        {
            linux_ctx.clock = CLOCK_MONOTONIC;
        }
    #endif

    linux_ctx.time_offset = 0.0;
    linux_ctx.time_offset = platform_get_time();
}

f64 platform_get_time(void)
{
    struct timespec os_time;

    clock_gettime(linux_ctx.clock, &os_time);    

    f64 time = (f64)os_time.tv_sec + (f64)(0.000000001 * (f64)os_time.tv_nsec);
    return time - linux_ctx.time_offset;
}

#endif
