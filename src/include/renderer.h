#ifndef BADGL_RENDERER_H
#define BADGL_RENDERER_H

#undef bool
#define bool _Bool

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"

typedef struct Renderer 
{
    Shader* shaders; // can have multiple shader programs
    int shader_count;
    int width, height;
    GLFWwindow* win;

    bool* cam_wait_update; // for preventing camera flicking when cursor jumps (optional)
    bool cursor_disabled; // anything relying on mouse should not update when this is false
} Renderer;

void rd_init(Renderer* rd, int width, int height, const char* win_title);

int rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src); // returns index to shader in array

Shader* rd_get_shader(Renderer* self, int index);

void rd_set_wireframe(bool useWireframe); // true for wireframe, false for filled polygons

void rd_begin_frame();

void rd_end_frame(Renderer* self);

bool rd_win_should_close(Renderer* self);

void rd_set_cam_bool(Renderer* self, bool* cam_bool); // bit hacky, but works

void rd_get_cursor_pos(Renderer* self, float* cursor_x, float* cursor_y);

bool rd_get_key(Renderer* self, int key);

void rd_free(Renderer* self);

#endif