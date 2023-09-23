#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// forward declarations
//typedef struct Camera Camera;
#include "camera.h" // FIX LATER

#define MAX_TEXTURES 10

typedef struct Renderer 
{
    int width, height;
    int shader_program;
    int vao;
    int vbo;
    int ebo;
    int textures[MAX_TEXTURES];
    int tex_count;
    Camera cam;
} Renderer;

void renderer_init(GLFWwindow* window, Renderer* renderer);

void renderer_add_texture(Renderer* renderer, int texture, int texture_unit, const char* sampler_name);

int renderer_find_uniform(Renderer* renderer, const char* name);

void renderer_uniform_mat4(Renderer* renderer, const char* name, mat4 mat);

void renderer_free(Renderer* renderer);

#endif