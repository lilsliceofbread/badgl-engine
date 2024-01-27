#ifndef BGL_RENDERER_H
#define BGL_RENDERER_H

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "defines.h"
#include "shader.h"

#define RD_SHADER_ALLOC_SIZE 4

typedef enum RendererFlags
{
    // * if enabled, will generate the respective shader for each
    RD_USE_SKYBOX   = 1 << 0,
    RD_USE_UI       = 1 << 1,
    RD_USE_LIGHTING = 1 << 2,

    // ! user should not modify these flags, internal use only
    RD_INTERNAL_CURSOR_DISABLED = 1 << 3,
    RD_INTERNAL_VSYNC_ENABLED = 1 << 4
} RendererFlags;

typedef struct Renderer 
{
    Shader* shaders; // can have multiple shader programs
    u32 skybox_shader, quad_shader, light_shader; // required shader indices
    u32 shader_count;
    i32 width, height;
    GLFWwindow* win;

    i32 mouse_wait_update; // frames until mouse can update, for preventing camera flicking when cursor jumps
    bool mouse_should_update; // * if this is true, anything relying on mouse should not update

    double last_time, delta_time;
    u32 framecount;

    ImGuiContext* imgui_ctx; 
    ImGuiIO* imgui_io; 

    RendererFlags flags;
} Renderer;

/**
 * @brief initialise window, OpenGL and shaders
 * @note   
 * @param  rd: 
 * @param  width: width of window
 * @param  height: height of window
 * @param  win_title: title of window
 * @param  flags: use any RendererFlag except those marked with RD_INTERNAL_
 * @retval None
 */
void rd_init(Renderer* rd, i32 width, i32 height, const char* win_title, RendererFlags flags);

void rd_begin_frame(Renderer* self);

void rd_end_frame(Renderer* self);

u32 rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src); // returns index to shader in array

Shader* rd_get_shader(Renderer* self, u32 index);

bool rd_win_should_close(Renderer* self);

void rd_draw_triangles(u32 ind_count);

void rd_update_viewport(Renderer* self); // annoying fix, sometimes resize callback is delayed

void rd_set_viewport(i32 x, i32 y, i32 width, i32 height); // x and y of top left point

void rd_swap_buffers(Renderer* self); // allow user to manually swap buffers

void rd_set_wireframe(bool useWireframe); // true for wireframe, false for filled polygons

void rd_get_cursor_pos(Renderer* self, float* cursor_x_out, float* cursor_y_out);

bool rd_key_pressed(Renderer* self, i32 key);

void rd_free(Renderer* self);

#endif