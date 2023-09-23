#include "renderer.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "window.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

#define MAX_UNIFORMS 100

typedef struct uniform_pair {
    const char* name;
    int location;
} uniform_pair;

static uniform_pair stored_uniforms[MAX_UNIFORMS];
static int uniform_count = 0;

void renderer_init(GLFWwindow* window, Renderer* renderer)
{
#ifndef NO_DEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_callback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
#endif

    glViewport(0, 0, renderer->width, renderer->height);

    renderer->cam = camera_init();

    glEnable(GL_BLEND); // enable transparent textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // which blending function for transparency
    glEnable(GL_FRAMEBUFFER_SRGB); // use standard rgb for framebuffer
    //glDisable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    
    int shader_program;
    shader_program_create(&shader_program, "shaders/shader.vert", "shaders/shader.frag");
    renderer->shader_program = shader_program;
    
    // TRIANGLE STUFF

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

    int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    renderer->vao = vao;
    glGenBuffers(1, &vbo);
    renderer->vbo = vbo;
    glGenBuffers(1, &ebo);
    renderer->ebo = ebo;

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

    int texture;
    texture2d_create(&texture, "res/brick.jpeg", false, true);

    renderer_add_texture(renderer, texture, 0, "texture0");

    float aspect_ratio = (float)(renderer->width) / (float)(renderer->height);
    camera_calc_proj(&(renderer->cam), 90.0f, aspect_ratio, 0.1f, 100.0f);
}

void renderer_add_texture(Renderer* renderer, int texture, int texture_unit, const char* sampler_name)
{
    ASSERT(!(renderer->tex_count >= MAX_TEXTURES), "ERR: renderer max textures reached");
    renderer->textures[renderer->tex_count] = texture; 
    renderer->tex_count++;

    // associate texture unit with sampler in shader
    renderer_use_program(renderer);
    glUniform1i(glGetUniformLocation(renderer->shader_program, sampler_name), texture_unit);
        
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void renderer_use_program(Renderer* renderer)
{
    glUseProgram(renderer->shader_program);
}

int renderer_find_uniform(Renderer* renderer, const char* name)
{
    return glGetUniformLocation(renderer->shader_program, name);  
}

void renderer_uniform_mat4(Renderer* renderer, const char* name, mat4 mat)
{
    // only requires one find uniform per uniform
    uniform_pair curr;
    for(int i = 0; i < uniform_count; i++)
    {
        curr = stored_uniforms[i];
        if(strcmp(name, curr.name) == 0)
        {
            glUniformMatrix4fv(curr.location, 1, false, (float*)mat);
            return;
        }
    }
    // if haven't used uniform before
    int location = renderer_find_uniform(renderer, name);
    glUniformMatrix4fv(location, 1, false, (float*)mat);
    if(uniform_count < MAX_UNIFORMS) // don't add past array end
    {
        stored_uniforms[uniform_count].name = name;
        stored_uniforms[uniform_count].location = location;
        uniform_count++;
    }
}

void renderer_free(Renderer* renderer)
{
    glDeleteVertexArrays(1, &(renderer->vao));
    glDeleteBuffers(1, &(renderer->vbo));
    glDeleteBuffers(1, &(renderer->ebo));
    glDeleteProgram(renderer->shader_program);
}