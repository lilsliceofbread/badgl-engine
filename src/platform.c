#include "platform.h"
#include "defines.h"

/* non os-specific platform stuff i.e. glfw abstraction
 * allows for potentially removing glfw in future? */

/**
 * internal functions
 */
void platform_resize_callback(GLFWwindow* win, i32 width, i32 height);
void platform_key_callback(GLFWwindow* win, i32 key, i32 scancode, i32 action, i32 mods);

void platform_window_init(BGLWindow* window, i32 width, i32 height, const char* win_title, i32 gl_version_major, i32 gl_version_minor)
{
    window->width = width;
    window->height = height;
    window->mouse_wait = 1;
    window->mouse_enabled = true;
    window->resize_func = NULL;

    BGL_ASSERT(glfwInit(), "failed to init GLFW");

    #ifndef BGL_NO_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_version_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    GLFWwindow* win = glfwCreateWindow(width, height, win_title, NULL, NULL);
    window->win = win;
    BGL_ASSERT(win != NULL, "failed to open window. is your opengl version supported on your machine?");

    glfwMakeContextCurrent(win);
    glfwSetWindowUserPointer(window->win, window);
    glfwSetCursorPos(win, (f64)width / 2, (f64)height / 2);
    glfwSetKeyCallback(win, platform_key_callback);
    glfwSetFramebufferSizeCallback(win, platform_resize_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool platform_window_key_pressed(BGLWindow* window, i32 key)
{
    return GLFW_PRESS == glfwGetKey(window->win, key);
}

bool platform_window_get_cursor(BGLWindow* window, f64* x_out, f64* y_out)
{
    glfwGetCursorPos(window->win, x_out, y_out);

    return window->mouse_enabled && window->mouse_wait <= 0;
}

void platform_window_toggle_cursor(BGLWindow* window)
{
    if(!window->mouse_enabled) // toggle between cursor locked vs usable
    {
        glfwSetInputMode(window->win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window->win, (f64)window->width / 2, (f64)window->height / 2);

        window->mouse_wait = 2; // mouse updates should wait 1 frame to prevent flicking
        window->mouse_enabled = true;                                    
    }
    else
    {
        glfwFocusWindow(window->win);
        glfwSetInputMode(window->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        window->mouse_enabled = false;                                    
    }
}

bool platform_window_should_close(BGLWindow* window)
{
    return glfwWindowShouldClose(window->win);
}

void platform_window_swap_buffers(BGLWindow* window)
{
    glfwSwapBuffers(window->win);
}

void platform_window_poll_events(BGLWindow* window)
{
    glfwPollEvents();

    if(window->mouse_wait > 0)
    {
        window->mouse_wait--;
        return;
    }
}

void platform_window_set_resize_callback(BGLWindow* window, BGLWindowResizeFunc func)
{
    window->resize_func = func;
}

void platform_window_update_size(BGLWindow* window)
{
    glfwGetFramebufferSize(window->win, &window->width, &window->height);
}

void platform_window_free(BGLWindow* window)
{
    glfwDestroyWindow(window->win);
    glfwTerminate();
}



/**
 * internal functions
 */
void platform_resize_callback(GLFWwindow* win, i32 width, i32 height)
{
    BGLWindow* window = (BGLWindow*)glfwGetWindowUserPointer(win); 
    window->width = width;
    window->height = height;
    if(window->resize_func != NULL) window->resize_func(window); // awkward system to avoid calling glViewport in platform
}

void platform_key_callback(GLFWwindow* win, i32 key, BGL_UNUSED i32 scancode, i32 action, BGL_UNUSED i32 mods)
{
    BGLWindow* window = (BGLWindow*)glfwGetWindowUserPointer(win);

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        platform_window_toggle_cursor(window);
    }
}
