#include "platform.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <glad/glad.h>
#include "wglext.h"

#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "util.h"

static struct
{
    LARGE_INTEGER freq;
    f64 time_offset;
    char directory[BGL_MAX_EXECUTABLE_DIR_LENGTH];
} win_ctx;

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

void platform_init(void)
{
    char exe_path[BGL_MAX_EXECUTABLE_DIR_LENGTH];
    memset(exe_path, 0, BGL_MAX_EXECUTABLE_DIR_LENGTH);
    platform_get_executable_path(exe_path, BGL_MAX_EXECUTABLE_DIR_LENGTH);
    find_directory_from_path(win_ctx.directory, BGL_MAX_EXECUTABLE_DIR_LENGTH, exe_path);
    
    platform_reset_time();
}

void* platform_virtual_alloc(u64 size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

void platform_physical_alloc(void* ptr, u64 size)
{
    VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

void platform_physical_dealloc(void* ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

void platform_virtual_dealloc(void* ptr, u64 size)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

void platform_prepend_executable_directory(char* buffer, u32 length, const char* path)
{
    if(strstr(path, win_ctx.directory)) // if already full path
    {
        strncpy(buffer, path, length);
        return;
    }
    
    snprintf(buffer, length, "%s/%s", win_ctx.directory, path);
}

void platform_get_executable_path(char* buffer, u32 length)
{
    bool success = GetModuleFileNameA(NULL, buffer, (DWORD)length) != 0;
    BGL_ASSERT(success, "unable to get executable directory. err: %lu", GetLastError());
}

bool platform_file_exists(const char* filename)
{
    return _access(filename, 0) != -1;
}

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

    return true;
}

void platform_toggle_vsync(bool on)
{
    wglSwapIntervalEXT((int)on);
}

void platform_reset_time(void)
{
    QueryPerformanceFrequency(&win_ctx.freq);
    bool success = win_ctx.freq.QuadPart != 0;
    BGL_ASSERT(success, "unable to get platform timer frequency. err: %lu", GetLastError());

    win_ctx.time_offset = 0.0;
    win_ctx.time_offset = platform_get_time();
}

f64 platform_get_time(void)
{
    LARGE_INTEGER os_time;

    QueryPerformanceCounter(&os_time);

    f64 time = (f64)os_time.QuadPart / (f64)win_ctx.freq.QuadPart;
    return time - win_ctx.time_offset;
}

#endif
