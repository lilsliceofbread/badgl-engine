#ifndef BGL_PLATFORM_H
#define BGL_PLATFORM_H

// TODO: move glfw out of header? requires creating keymap
#include <glad/glad.h> // prevent errors
#include <GLFW/glfw3.h>

#include "types.h"

#define BGL_MAX_EXECUTABLE_DIR_LENGTH 512
#ifdef _WIN32
#define PLATFORM_FILE_SEPARATOR '\\'
#else
#define PLATFORM_FILE_SEPARATOR '/'
#endif

typedef struct BGLWindow BGLWindow;
typedef void(*BGLWindowResizeFunc)(BGLWindow* window);

typedef struct BGLWindow {
    GLFWwindow* win;
    i32 width, height;
    BGLWindowResizeFunc resize_func;

    i32 mouse_wait;
    bool mouse_enabled; // * if this is true, anything relying on mouse should not update
} BGLWindow;

/**
 * @param  key: use GLFW key defines as key TODO: use own key defines?
 */
bool platform_window_key_pressed(BGLWindow* window, i32 key);

/**
 * @param  x_out: out parameter of cursor x pos
 * @param  y_out: out parameter of cursor y pos 
 * @returns bool denoting if the mouse is captured, if not, don't use for player motion
 */
bool platform_window_get_cursor(BGLWindow* window, f64* x_out, f64* y_out);

/**
 * @brief toggle capture of mouse cursor
 */
void platform_window_toggle_cursor(BGLWindow* window);

bool platform_window_should_close(BGLWindow* window);

/**
 * @brief  swap framebuffers (rd_end_frame does this for you)
 */
void platform_window_swap_buffers(BGLWindow* window);

void platform_toggle_vsync(bool on);

void platform_reset_time(void);

double platform_get_time(void);


/**
 * internal functions
 */
void platform_window_init(BGLWindow* window, i32 width, i32 height, const char* win_title, i32 gl_version_major, i32 gl_version_minor);

void platform_window_poll_events(BGLWindow* window);

void platform_window_set_resize_callback(BGLWindow* window, BGLWindowResizeFunc func);

void platform_window_update_size(BGLWindow* window);

void platform_window_free(BGLWindow* window);

/* os-specific */

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

#endif
