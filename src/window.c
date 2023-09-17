#include "window.h"
#include "shader.h"
#include "texture.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"

#include <cglm/cglm.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define SENSITIVITY 0.1f

void window_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WinContext* ctx = (WinContext*)glfwGetWindowUserPointer(window);
    ctx->width = width;
    ctx->height = height;

    //printf("window resized - w: %d h: %d\n", width, height);
}

void window_mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y)
{
    WinContext* ctx = (WinContext*)glfwGetWindowUserPointer(window);
    Camera* cam = &(ctx->renderer.cam);
    if(cam->last_cursor_x == -1.0f) // if application just started
    {
        cam->last_cursor_x = cursor_x;
        cam->last_cursor_y = cursor_y;
        return;
    }

    cam->yaw += SENSITIVITY * (cursor_x - cam->last_cursor_x); // x offset * sens = yaw
    cam->pitch += SENSITIVITY * (cam->last_cursor_y - cursor_y); // y offset needs to be from bottom to top not top to bottom so -()
    cam->last_cursor_x = cursor_x;
    cam->last_cursor_y = cursor_y;

    // constrain pitch so you can't flip your "head"
    if(cam->pitch > 89.0f)
    {
        cam->pitch = 89.0f;
    } 
    else if(cam->pitch < -89.0f)
    {
        cam->pitch = -89.0f;
    }

    vec3 dir = {
        cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch)), 
        sin(glm_rad(cam->pitch)),
        sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch)), 
    };

    glm_vec3_normalize_to(dir, cam->dir);
}

GLFWwindow* window_init(WinContext* ctx) 
{
    if(!glfwInit())
    {
        fprintf(stderr, "ERR: failed to init GLFW");
        return NULL;
    }    

    // REMOVE IN RELEASE
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(ctx->width, ctx->height, "learnopengl", NULL, NULL);
    if(window == NULL)
    {
        fprintf(stderr, "ERR: failed to open window");
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ctx->renderer.cam = camera_init(window, window_mouse_callback);

    // FOR TRANSPARENT TEXTURES
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    
    // triangle stuff // SEPARATE FUNC renderer
    
    GLuint shader_program;
    if(!shader_program_create(&shader_program, "shaders/shader.vert", "shaders/shader.frag"))
    {
        return NULL;
    }
    ctx->renderer.shader_program = shader_program;

    float vertices[] = {
        // vertices         // tex coords
        0.5f, 0.5f, 0.0f,   1.0f, 1.0f, // top right - clockwise
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    ctx->renderer.vao = vao;
    glGenBuffers(1, &vbo);
    ctx->renderer.vbo = vbo;
    glGenBuffers(1, &ebo);
    ctx->renderer.ebo = ebo;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex attribute buffer specified determined by buffer currently bound to GL_ARRAY_BUFFER
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    GLuint texture;
    if(!texture2d_create(&texture, "res/beans2.png", false, true))
    {
        return NULL;
    }
    ctx->renderer.textures[0] = texture;

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);
    glUseProgram(0);

    return window;
}
