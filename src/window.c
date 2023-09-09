#include "window.h"
#include "shader.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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

// make into shader.c with no winctx at some point
int window_create_shader_program(WinContext* ctx)
{
    int success;
    GLuint vert_shader, frag_shader, shader_program;
    char info_log[512];
    info_log[0] = '\0'; // if shader creation failed but no info log

    vert_shader = create_shader("shaders/shader.vert", GL_VERTEX_SHADER, info_log, sizeof(info_log), &success);
    if(!success)
    {
        fprintf(stderr, "ERR: failed to create vertex shader\n%s", info_log);
        return false;
    }

    frag_shader = create_shader("shaders/shader.frag", GL_FRAGMENT_SHADER, info_log, sizeof(info_log), &success);
    if(!success)
    {
        fprintf(stderr, "ERR: failed to create fragment shader\n%s", info_log);
        return false;
    }

    shader_program = glCreateProgram();
    ctx->shader_program = shader_program;
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "ERR: could not create shader program\n%s", info_log);
        return false;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return true;
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

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    glViewport(0, 0, ctx->width, ctx->height);
    glfwSetWindowUserPointer(window, ctx);
    glfwSetFramebufferSizeCallback(window, window_framebuffer_size_callback);
    
    // triangle stuff //
    
    if(!window_create_shader_program(ctx))
    {
        return NULL;
    }

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

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window))
    {
        window_process_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
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