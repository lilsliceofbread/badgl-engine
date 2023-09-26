#ifndef RENDERER_H
#define RENDERER_H

#undef bool
#define bool _Bool

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "shader.h"
#include "camera.h"

typedef struct Renderer 
{
    Window win;
    Shader shader; // shader program
    Camera cam;
} Renderer;

void rd_init(Renderer* rd, int width, int height, const char* win_title);

void rd_send_vp_matrix(Renderer* self);

void rd_set_wireframe(bool useWireframe); // true for wireframe, false for filled polygons

void rd_clear_frame();

void rd_free(Renderer* self);

#endif