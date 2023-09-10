#include "window.h"
#include "shader.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

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

    // REMOVE IN RELEASE
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

    // REMOVE IN RELEASE
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_callback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    glViewport(0, 0, ctx->width, ctx->height);
    glfwSetWindowUserPointer(window, ctx);
    glfwSetFramebufferSizeCallback(window, window_framebuffer_size_callback);
    
    // triangle stuff //
    
    GLuint shader_program;
    if(!create_shader_program(&shader_program, "shaders/shader.vert", "shaders/shader.frag"))
    {
        return NULL;
    }
    ctx->shader_program = shader_program;

    float vertices[] = {
        0.5f, 0.5f, 0.0f, // top right - clockwise
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    ctx->vao = vao;
    glGenBuffers(1, &vbo);
    ctx->vbo = vbo;
    glGenBuffers(1, &ebo);
    ctx->ebo = ebo;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    
    return window;
}

void window_loop(GLFWwindow* window, WinContext* ctx)
{
    GLuint shader_program = ctx->shader_program;
    GLuint vao = ctx->vao;
    GLuint vbo = ctx->vbo;
    GLuint ebo = ctx->ebo;

    GLint vert_offset = glGetUniformLocation(shader_program, "vert_offset");
    if(vert_offset == -1)
    {
        fprintf(stderr, "ERR: could not find uniform");
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteProgram(shader_program);
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window))
    {
        window_process_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shader_program);

        float time = glfwGetTime();
        float offset = sin(time) / 2.0f;
        glUniform1f(vert_offset, offset);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader_program);
}