#ifndef WINDOW_H
#define WINDOW_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// forward declarations
#undef bool
#define bool _Bool

// has to interface with glfw, kinda hacky but cbf
typedef void(*KeyCallbackFunc)(GLFWwindow* window, int key, int scancode, int action, int mods);
typedef void(*MouseCallbackFunc)(GLFWwindow* window, double cursor_x, double cursor_y);
typedef void(*ResizeCallbackFunc)(GLFWwindow* window, int width, int height);

typedef struct Window
{
    GLFWwindow* ptr;
    int width, height;
} Window;

Window window_init(int width, int height, const char* win_title);

void window_end_frame(Window* self);

bool window_should_close(Window* self);

void window_set_callback_ptr(Window* self, void* callback_ptr);

void window_key_callback(Window* self, KeyCallbackFunc callback);

void window_mouse_callback(Window* self, MouseCallbackFunc callback);

void window_resize_callback(Window* self, ResizeCallbackFunc callback);

bool window_get_key(Window* self, int key);

void window_terminate();

#endif