#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// forward declarations
//typedef struct Camera Camera;
#include "camera.h" // FIX LATER

#define MAX_TEXTURES 10

typedef struct Renderer 
{
    GLuint shader_program;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint textures[10];
    int tex_count;
    Camera cam;
} Renderer;

int renderer_add_texture(Renderer* renderer);

void renderer_free(Renderer* renderer);

int renderer_init(Renderer* renderer);

#endif