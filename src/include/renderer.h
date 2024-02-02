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
    RD_INTERNAL_VSYNC_ENABLED = 1 << 4,
} RendererFlags;

typedef struct Renderer 
{
    Shader* shaders;
    u32 skybox_shader, quad_shader, light_shader;
    u32 shader_count;
    i32 width, height;
    GLFWwindow* win;

    i32 mouse_wait_update;
    bool mouse_should_update; // * if this is true, anything relying on mouse should not update

    double last_time, delta_time;
    u64 framecount;

    ImGuiContext* imgui_ctx; 
    ImGuiIO* imgui_io; 

    RendererFlags flags;
} Renderer;

/**
 * @brief initialise window, OpenGL and shaders
 * @param  flags: use any RendererFlag except those marked with RD_INTERNAL_ (which will be ignored)
 */
void rd_init(Renderer* rd, i32 width, i32 height, const char* win_title, RendererFlags flags);

/**
 * @brief  clear screen, update delta_time
 */
void rd_begin_frame(Renderer* self);

/**
 * @brief  swap framebuffers
 */
void rd_end_frame(Renderer* self);

/**
 * @note paths should be relative to executable
 * @param  vert_src: path to vertex shader
 * @param  frag_src: path to fragment shader
 * @returns index to shader in rd->shaders
 */
u32 rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src);

/**
 * @returns bool
 */
bool rd_win_should_close(Renderer* self);

/**
 * @brief  manually update the glViewport to match the window (sometimes glfw doesn't automatically do this so if you need to, use this)
 */
void rd_update_viewport(Renderer* self);

/**
 * @brief  manually set glViewport to a specific value
 * @param  x: x of top left point
 * @param  y: y of top left point
 */
void rd_set_viewport(i32 x, i32 y, i32 width, i32 height);

/**
 * @brief  manually swap framebuffers (rd_end_frame does this for you)
 */
void rd_swap_buffers(Renderer* self);

/**
 * @brief  turn on wireframe mode
 * @param  useWireframe: true for wireframe, false for normal
 */
void rd_set_wireframe(bool useWireframe);

/**
 * @param  cursor_x_out: out parameter of cursor x pos
 * @param  cursor_y_out: out parameter of cursor y pos 
 */
void rd_get_cursor_pos(Renderer* self, double* cursor_x_out, double* cursor_y_out);

/**
 * TODO: change to use own key defines
 * @param  key: use GLFW key defines as key
 * @returns  bool
 */
bool rd_key_pressed(Renderer* self, i32 key);

void rd_free(Renderer* self);

/**
 * engine/internal functions
 */
void rd_draw_triangles(u32 ind_count);

#endif