#ifndef BADGL_RENDERER_H
#define BADGL_RENDERER_H

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "shader.h"

typedef struct Renderer 
{
    Shader* shaders; // can have multiple shader programs
    Shader skybox_shader, quad_shader, light_shader; // required shaders; separate from other optional ones
    uint32_t shader_count;
    int width, height;
    GLFWwindow* win;

    int mouse_wait_update; // frames until mouse can update, for preventing camera flicking when cursor jumps (optional)
    bool cursor_disabled; // anything relying on mouse should not update when this is false

    float last_time, delta_time; // delta time of last frame, last_time to calculate delta_time (not used by user)
    uint32_t framecount;

    bool vsync_enabled; // if vsync extension exists on this machine

    ImGuiContext* imgui_ctx; 
    ImGuiIO* imgui_io; 
} Renderer;

void rd_init(Renderer* rd, int width, int height, const char* win_title);

void rd_configure_gl(Renderer* self);

void rd_update_viewport(Renderer* self); // annoying fix, sometimes resize callback is delayed

void rd_set_viewport(int x, int y, int width, int height); // x and y of top left point

void rd_swap_buffers(Renderer* self); // allow user to manually swap buffers

void rd_imgui_init(Renderer* self, const char* glsl_version);

Shader* rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src); // returns index to shader in array

void rd_set_wireframe(bool useWireframe); // true for wireframe, false for filled polygons

void rd_begin_frame(Renderer* self);

void rd_end_frame(Renderer* self);

float rd_get_time(void);

bool rd_win_should_close(Renderer* self);

void rd_get_cursor_pos(Renderer* self, float* cursor_x_out, float* cursor_y_out);

bool rd_key_pressed(Renderer* self, int key);

void rd_free(Renderer* self);

void APIENTRY rd_debug_callback(GLenum source, GLenum type, unsigned int id,
                                GLenum severity, GLsizei length,
                                const char *message, const void *user_param);

#endif