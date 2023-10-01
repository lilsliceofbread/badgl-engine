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
    uint32_t shader_count;
    int width, height;
    GLFWwindow* win;

    int mouse_wait_update; // frames until mouse can update, for preventing camera flicking when cursor jumps (optional)
    bool cursor_disabled; // anything relying on mouse should not update when this is false

    float last_time, delta_time; // delta time of last frame, last_time to calculate delta_time (not used by user)
    uint32_t framecount;
} Renderer;

void rd_init(Renderer* rd, int width, int height, const char* win_title);

uint32_t rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src); // returns index to shader in array

void rd_set_wireframe(bool useWireframe); // true for wireframe, false for filled polygons

void rd_begin_frame(Renderer* self);

void rd_end_frame(Renderer* self);

float rd_get_time(void);

bool rd_win_should_close(Renderer* self);

void rd_get_cursor_pos(Renderer* self, float* cursor_x, float* cursor_y);

bool rd_get_key(Renderer* self, int key);

void rd_free(Renderer* self);

void APIENTRY rd_debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *user_param);

#endif