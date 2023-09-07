#include "window.h"
#include "shader.h"
#include <stdlib.h>
#include <stdio.h>

void window_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WinContext* ctx = (WinContext*)glfwGetWindowUserPointer(window);
    ctx->width = width;
    ctx->height = height;
}

void window_process_inputs(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1); // true
    }
}

GLFWwindow* window_init(WinContext* ctx) 
{
    if(!glfwInit())
    {
        fprintf(stderr, "ERR: failed to init GLFW");
        return NULL;
    }    

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(ctx->width, ctx->height, "learnopengl", NULL, NULL);
    if(window == NULL)
    {
        fprintf(stderr, "ERR: failed to open window");
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        fprintf(stderr, "ERR: failed to init GLAD");
        return NULL;
    }

    glViewport(0, 0, ctx->width, ctx->height);
    glfwSetWindowUserPointer(window, ctx);
    glfwSetFramebufferSizeCallback(window, window_framebuffer_size_callback);
    
    char* frag_src;
    frag_src = get_shader_source("shaders/frag_shader.frag");
    if(frag_src == NULL) return NULL;
    printf("%s", frag_src);
    free(frag_src);

    return window;
}

void window_loop(GLFWwindow* window, WinContext* ctx)
{
    while(!glfwWindowShouldClose(window))
    {
        window_process_inputs(window);

        glClearColor(0.5f, 0.1f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
