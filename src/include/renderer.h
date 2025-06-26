#ifndef BGL_RENDERER_H
#define BGL_RENDERER_H

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include "defines.h"
#include "shader.h"

typedef enum RendererFlags
{
    /* if these flags are set, will not generate the respective shader for each */
    BGL_RD_SKYBOX_OFF   = 1 << 0,
    BGL_RD_UI_OFF       = 1 << 1,
    BGL_RD_LIGHTING_OFF = 1 << 2,

    /* user should not modify these flags, internal use only */
    _BGL_RD_VSYNC_ENABLED = 1 << 3,
} RendererFlags;

typedef struct Renderer 
{
    BGLWindow window;
    Shader* shaders;
    u32 skybox_shader, quad_shader, light_shader; // light shader is the shader for the model associated with light
    u32 shader_count;

    f64 last_time, delta_time;
    u64 framecount;

    ImGuiContext* imgui_ctx; 
    ImGuiIO* imgui_io; 

    char version[3]; // x.y

    RendererFlags flags;
} Renderer;

/**
 * @brief initialise window, OpenGL and shaders
 * @param  flags: use any RendererFlag except those marked with RD_INTERNAL_ (which will be ignored)
 * @param  version: opengl version in format "x.y" e.g. "3.3"
 */
void rd_init(Renderer* rd, i32 width, i32 height, const char* win_title, RendererFlags flags, const char* version);

/**
 * @brief  clear screen, update delta_time
 */
void rd_begin_frame(Renderer* self);

/**
 * @brief  swap framebuffers
 */
void rd_end_frame(Renderer* self);

/**
 * @brief  add shader program to renderer. pass in your GLSL shader code in any order, as long as there is all the required shaders
 * @note   the shaders can either be all contained in one file separated by "#type vertex|fragment|geometry| or each in a separate file
 * @note   #includes should be specified relative to the specific file
 * @param  scratch:  arena for doing temp work in. arena is reset back to its initial position before returning
 * @param  shader_filepaths: paths to each shader relative to executable, must contain vertex and fragment shader, geometry shader optional
 * @param  shader_out: returns shader_index in rd->shaders array
 * @returns bool denoting if shader was created or failed
 */
bool rd_add_shader(Renderer* self, Arena* scratch, const char** shader_filepaths, u32 shader_count, u32* shader_out);

/**
 * @brief  manually update the glViewport to match the window (sometimes it doesn't happen automatically so if you need to, use this)
 */
void rd_update_viewport(Renderer* self);

/**
 * @brief  manually set glViewport to a specific value
 * @param  x: x of top left point
 * @param  y: y of top left point
 */
void rd_set_viewport(i32 x, i32 y, i32 width, i32 height);

/**
 * @brief  turn on wireframe mode
 * @param  useWireframe: true for wireframe, false for normal
 */
void rd_set_wireframe(bool useWireframe);

void rd_free(Renderer* self);

/**
 * engine/internal functions
 */
void rd_draw_triangles(u32 ind_count);

#endif
