#include "window.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

#include <cglm/cglm.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

void window_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WinContext* ctx = (WinContext*)glfwGetWindowUserPointer(window);
    ctx->width = width;
    ctx->height = height;

    //printf("window resized - w: %d h: %d\n", width, height);
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

    // FOR TRANSPARENT TEXTURES
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    
    // triangle stuff //
    
    GLuint shader_program;
    if(!shader_program_create(&shader_program, "shaders/shader.vert", "shaders/shader.frag"))
    {
        return NULL;
    }
    ctx->shader_program = shader_program;

    /*float vertices[] = {
        // vertices         // tex coords
        0.5f, 0.5f, 0.0f,   1.0f, 1.0f, // top right - clockwise
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };*/

    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
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

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex attribute buffer specified determined by buffer currently bound to GL_ARRAY_BUFFER
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    Texture2D texture;
    if(!texture2d_create(&texture, "res/beans2.png", false, true))
    {
        return NULL;
    }
    ctx->textures = texture;

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);
    glUseProgram(0);

    return window;
}

void window_loop(GLFWwindow* window, WinContext* ctx)
{
    GLuint shader_program = ctx->shader_program;
    GLuint vao = ctx->vao;
    GLuint vbo = ctx->vbo;
    GLuint ebo = ctx->ebo;
    GLuint texture1 = ctx->textures.id;

    GLint model_uniform = glGetUniformLocation(shader_program, "model");
    GLint view_uniform = glGetUniformLocation(shader_program, "view");
    GLint proj_uniform = glGetUniformLocation(shader_program, "proj");
    if(model_uniform == -1 || view_uniform == -1 || proj_uniform == -1)
    {
        fprintf(stderr, "ERR: could not find uniform");
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteProgram(shader_program);
        return;
    }

    vec3 cube_pos[] = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        {2.4f, -0.4f, -3.5f},
        {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},
        {1.5f, 2.0f, -2.5f},
        {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}
    };

    mat4 model, view, proj;


    glm_mat4_identity(proj);
    float aspect_ratio = (float)(ctx->width)/(float)(ctx->height); // if don't cast both to float, will produce integer :(
    glm_perspective(glm_rad(90.0f), aspect_ratio, 0.1f, 100.0f, proj);
    glUseProgram(shader_program);
    glUniformMatrix4fv(proj_uniform, 1, false, (float*)proj);

    glClearColor(0.8f, 0.3f, 0.5f, 0.0f);
    while(!glfwWindowShouldClose(window))
    {
        window_process_inputs(window);

        float time = glfwGetTime();
        float offset = 5 * (sin(time) - 1.0f);

        glm_mat4_identity(view);
        glm_translate(view, (vec3){0.0f, 0.0f, offset});
        glm_rotate(view, time, (vec3){0.0f, 1.0f, 0.0f});

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        
        glUseProgram(shader_program);

        glUniformMatrix4fv(view_uniform, 1, false, (float*)view);

        glBindVertexArray(vao);
        for(int i = 0; i < 10; i++)
        {
            glm_mat4_identity(model);
            glm_translate(model, cube_pos[i]);

            vec3 axis;
            glm_vec3_add(cube_pos[i], (vec3){0.0f, 1.0f, 0.0f}, axis); // for initial cube with pos 0 0 0 don't have weirdness
            glm_vec3_normalize(axis);
            glm_rotate(model, time, axis);

            glUniformMatrix4fv(model_uniform, 1, false, (float*)model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader_program);
}