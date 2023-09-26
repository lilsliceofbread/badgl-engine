#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"

#include <cglm/cglm.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

void default_resize_callback(GLFWwindow* window, int width, int height);

Window window_init(int width, int height, const char* win_title) 
{
    Window self;

    ASSERT(glfwInit(), "ERR: failed to init GLFW");

    #ifndef NO_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    GLFWwindow* ptr = glfwCreateWindow(width, height, win_title, NULL, NULL);

    self.ptr = ptr;

    ASSERT(ptr != NULL, "ERR: failed to open window");

    glfwMakeContextCurrent(ptr);
    glfwSwapInterval(1);

    ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "ERR: failed to init GLAD");

    glfwSetFramebufferSizeCallback(ptr, default_resize_callback);
    glfwSetInputMode(ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return self;
}

void window_set_callback_ptr(Window* self, void* callback_ptr)
{
    glfwSetWindowUserPointer(self->ptr, callback_ptr);
}

/* can't use in callback anyway :/
void* window_get_callback_ptr(Window* self)
{
    return glfwGetWindowUserPointer(self->ptr);
}
*/

void window_key_callback(Window* self, KeyCallbackFunc callback)
{
    glfwSetKeyCallback(self->ptr, callback);
}

void window_resize_callback(Window* self, ResizeCallbackFunc callback)
{
    glfwSetFramebufferSizeCallback(self->ptr, callback);
}

void window_mouse_callback(Window* self, MouseCallbackFunc callback)
{
    glfwSetCursorPosCallback(self->ptr, callback);
}

bool window_get_key(Window* self, int key)
{
    return GLFW_PRESS == glfwGetKey(self->ptr, key);
}

void window_terminate()
{
    glfwTerminate();
}

bool window_should_close(Window* self)
{
    return glfwWindowShouldClose(self->ptr);
}

void window_end_frame(Window* self)
{
    glfwSwapBuffers(self->ptr);
    glfwPollEvents();
}

// default resize callback
void default_resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    
    //printf("window resized - w: %d h: %d\n", width, height);
}