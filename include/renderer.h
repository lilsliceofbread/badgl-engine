#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "shader.h"
#include "camera.h"

typedef struct Renderer 
{
    int width, height;
    Window win;
    Shader shader; // shader program
    Camera cam;
} Renderer;

void rd_init(Renderer* rd, int width, int height);

void rd_send_vp_matrix(Renderer* self);

void rd_clear_frame();

void rd_free(Renderer* self);

#endif