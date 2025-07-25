#ifndef BGL_RENDERER_H
#define BGL_RENDERER_H

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include "defines.h"
#include "window.h"
#include "shader.h"

typedef enum RendererFlags
{
    /* if these flags are set, will not generate the respective shader for each */
    BGL_RD_SKYBOX_OFF   = 1 << 0,
    BGL_RD_QUAD_OFF       = 1 << 1,
    BGL_RD_LIGHTING_OFF = 1 << 2,

    /* user should not modify these flags, internal use only */
    _BGL_RD_VSYNC_ENABLED = 1 << 3,
} RendererFlags;

/* engine debug editor constants */
#ifdef BGL_EDITOR
#define BGL_MAX_EDITOR_PANES 16
#define BGL_MAX_EDITOR_STR 32
#endif

typedef struct Renderer 
{
    BGLWindow window;
    Shader* shaders;
    u32 skybox_shader, quad_shader, light_shader; // light shader is the shader for the model associated with light
    u32 shader_count;
    u32 current_shader;

    f64 last_time, delta_time;
    u64 framecount;

    ImGuiContext* imgui_ctx; 
    ImGuiIO* imgui_io; 

    #ifdef BGL_EDITOR
    union // to handle the case where multiple panes are handled by 1 button
    {
        char str[BGL_MAX_EDITOR_STR];
        struct
        {
            u8 null_terminator;
            u32 index;
        };
    } editor_panes[BGL_MAX_EDITOR_PANES];
    bool* editor_flags[BGL_MAX_EDITOR_PANES];
    u32 pane_count;

    bool editor_open;
    #endif

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
 * @note   setting uniform buffer object bindings in the shader is allowed even for opengl < 4.2
 * @param  scratch:  arena for doing temp work in. arena is reset back to its initial position before returning
 * @param  shader_filepaths: paths to each shader relative to executable, must contain vertex and fragment shader, geometry shader optional
 * @param  shader_out: returns shader_index in rd->shaders array
 * @returns bool denoting if shader was created or failed
 */
bool rd_add_shader(Renderer* self, Arena* scratch, const char** shader_filepaths, u32 shader_count, u32* shader_out);

/**
 * @param  index: index of shader given by rd_add_shader
 */
void rd_use_shader(Renderer* self, u32 index);

/**
 * @brief hot reload shader
 * @note not fast - frame must wait some time
 */
void rd_reload_shader(Renderer* self, u32 index);

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
 * @param  on: true for wireframe, false for normal
 */
void rd_toggle_wireframe(bool on);

/**
 * @param  on: turns on face culling
 * @param  back: should cull the front or the back face
 */
void rd_cull_face(bool on, bool back);

void rd_toggle_vsync(bool on);

void rd_free(Renderer* self);

/**
 * @brief add a pane to editor ui
 * @param  user: pointer to bool that determines if you should display your editor pane (check before renderering it)
 * @note this function sets user to false so if you want your pane to display by default, set it to true after calling
 */
void rd_editor_add_pane(Renderer* self, const char* name, bool* user);

/**
 * engine/internal functions
 */
void rd_draw_triangles(u32 ind_count);

#endif
