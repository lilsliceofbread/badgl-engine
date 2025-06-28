#ifndef BGL_WINDOW_H
#define BGL_WINDOW_H

// TODO: move glfw out of header? requires creating keymap
#include <glad/glad.h> // prevent errors
#include <GLFW/glfw3.h>

#include "types.h"

typedef struct BGLWindow BGLWindow;
typedef void(*BGLWindowResizeFunc)(BGLWindow* window);

typedef struct BGLWindow {
    GLFWwindow* win;
    i32 width, height;
    BGLWindowResizeFunc resize_func;

    i32 mouse_wait;
    bool mouse_enabled;
} BGLWindow;

/**
 * @param  key: use GLFW key defines as key TODO: use own key defines?
 */
bool window_key_pressed(BGLWindow* window, i32 key);

/**
 * @param  x_out: out parameter of cursor x pos
 * @param  y_out: out parameter of cursor y pos 
 */
bool window_get_cursor(BGLWindow* window, f64* x_out, f64* y_out);

/**
 * @brief toggle capture of mouse cursor
 */
void window_toggle_cursor(BGLWindow* window);

bool window_should_close(BGLWindow* window);

/**
 * @brief  swap framebuffers (rd_end_frame does this for you)
 */
void window_swap_buffers(BGLWindow* window);

/**
 * internal functions
 */
void window_init(BGLWindow* window, i32 width, i32 height, const char* win_title, i32 gl_version_major, i32 gl_version_minor);

void window_poll_events(BGLWindow* window);

void window_set_resize_callback(BGLWindow* window, BGLWindowResizeFunc func);

void window_update_size(BGLWindow* window);

void window_free(BGLWindow* window);

#endif
