#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"

#include <cglm/cglm.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

static bool cursor_disabled = true;

void _framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    renderer->width = width;
    renderer->height = height;

    //printf("window resized - w: %d h: %d\n", width, height);
}

void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);

    if(key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1); // true
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(cursor_disabled) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            renderer->cam.wait_update = true; // to stop sharp turn when cursor jumps
            cursor_disabled = false;
        }
        else
        {
            glfwFocusWindow(window);
            glfwSetCursorPos(window, renderer->width/2, renderer->height/2); // move cursor to middle of screen
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            renderer->cam.wait_update = true; // to stop sharp turn when cursor jumps
            cursor_disabled = true;
        }
    }
}

void _mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y)
{
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    Camera* cam = &(renderer->cam);
    camera_mouse_update(cam, cursor_x, cursor_y);
}

GLFWwindow* window_init(Renderer* renderer) 
{
    if(!glfwInit())
    {
        fprintf(stderr, "ERR: failed to init GLFW");
        return NULL;
    }    

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
    
    GLFWwindow* window = glfwCreateWindow(renderer->width, renderer->height, "learnopengl", NULL, NULL);

    ASSERT(window != NULL, "ERR: failed to open window");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "ERR: failed to init GLAD");

    glfwSetWindowUserPointer(window, renderer);
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);
    glfwSetCursorPosCallback(window, _mouse_callback);
    glfwSetKeyCallback(window, _key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    renderer_init(window, renderer);

    return window;
}
