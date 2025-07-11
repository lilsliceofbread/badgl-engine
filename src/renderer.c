#include "renderer.h"

#include "force_dgpu.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"
#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "platform.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

#define BGL_RD_VERSION_STRLEN 24 // bit extra to make it multiple of 8
#define RD_NO_BLOCK_BINDINGS(self) !(CHAR_TO_INT((self)->version[0]) == 4 && CHAR_TO_INT((self)->version[2]) >= 2)

/**
 * internal functions
 */
void rd_configure_gl(Renderer* self);
void rd_imgui_init(Renderer* self, const char* glsl_version);
void rd_resize_callback(BGLWindow* window);
void APIENTRY rd_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length,
                                const char *message, const void *user_param);
void rd_get_version_major_minor(Renderer* self, i32* major, i32* minor);
void rd_get_version_string(Renderer* self, char* buffer);
void rd_editor_toggle_open_panes(Renderer* self);
void rd_editor_pane(Renderer* self);

void rd_init(Renderer* self, i32 width, i32 height, const char* win_title, RendererFlags flags, const char* version)
{
    self->shader_count = 0;
    self->shaders = NULL;
    self->current_shader = 9999999; // some large value that won't be true for the first check
    self->delta_time = 0.0;
    self->last_time = 0.0;
    self->framecount = 0;
    self->flags = flags & (RendererFlags)~(_BGL_RD_VSYNC_ENABLED); // set to false by default
    #ifdef BGL_EDITOR                                                                   
    self->pane_count = 0;
    rd_editor_add_pane(self, "renderer", &self->editor_open);
    #endif

    i32 major, minor;
    BGL_ASSERT(strlen(version) >= 3, "invalid opengl version");
    memcpy(self->version, version, 3);
    rd_get_version_major_minor(self, &major, &minor);
    BGL_ASSERT((major == 3 && minor >= 3) || (major == 4 && minor <= 6), "opengl versions supported: 3.3 - 4.6");

    window_init(&self->window, width, height, win_title, major, minor);
    window_set_resize_callback(&self->window, (BGLWindowResizeFunc)rd_resize_callback);

    BGL_ASSERT(gladLoadGL(), "failed to init GLAD");

    rd_configure_gl(self);

    platform_init();

    Arena arena;
    arena_create_sized(&arena, MEGABYTES(1)); // definitely big enough for these files
    const char* shader_filepaths[] = {"shaders/skybox.glsl", "shaders/quad.glsl", "shaders/light.glsl"};

    self->skybox_shader = self->quad_shader = self->light_shader = 0;
    if(!(self->flags & BGL_RD_SKYBOX_OFF))
        BGL_ASSERT_NO_MSG(rd_add_shader(self, &arena, &shader_filepaths[0], 1, &self->skybox_shader));
    if(!(self->flags & BGL_RD_QUAD_OFF))
        BGL_ASSERT_NO_MSG(rd_add_shader(self, &arena, &shader_filepaths[1], 1, &self->quad_shader));
    if(!(self->flags & BGL_RD_LIGHTING_OFF))
        BGL_ASSERT_NO_MSG(rd_add_shader(self, &arena, &shader_filepaths[2], 1, &self->light_shader));

    arena_free(&arena);

    char imgui_version[BGL_RD_VERSION_STRLEN];
    rd_get_version_string(self, imgui_version);
    rd_imgui_init(self, imgui_version);
}

void rd_configure_gl(Renderer* self)
{
    #ifndef BGL_NO_DEBUG
    i32 flags, major, minor;
    rd_get_version_major_minor(self, &major, &minor);

    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(major == 4 && minor >= 3 && flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(rd_debug_callback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
    #endif

    if(platform_init_vsync())
    {
        self->flags |= _BGL_RD_VSYNC_ENABLED;
        platform_toggle_vsync(true);
    }
    else 
    {
        BGL_LOG_INFO("OpenGL vsync extension not enabled");
    }

    textures_init();

    glViewport(0, 0, self->window.width, self->window.height);

    glEnable(GL_BLEND); // enable transparent textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_FRAMEBUFFER_SRGB); // this makes everything look oversaturated and garbage

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW); // front face has counter-clockwise vertices
    glCullFace(GL_BACK);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void rd_draw_triangles(u32 ind_count)
{
    glDrawElements(GL_TRIANGLES, (i32)ind_count, GL_UNSIGNED_INT, 0);
}

// annoying fix, sometimes resize callback is delayed
void rd_update_viewport(Renderer* self)
{
    window_update_size(&self->window);
    glViewport(0, 0, self->window.width, self->window.height);
}

void rd_set_viewport(i32 x, i32 y, i32 width, i32 height)
{
    glViewport(x, y, width, height);
}

void rd_imgui_init(Renderer* self, const char* glsl_version)
{
    self->imgui_ctx = igCreateContext(NULL); 
    self->imgui_io = igGetIO(); 

    // force imgui to use executable directory for imgui.ini
    self->imgui_io->IniFilename = NULL;
    char imgui_ini_path[256];
    platform_prepend_executable_directory(imgui_ini_path, 256, "imgui.ini");
    igLoadIniSettingsFromDisk(imgui_ini_path);

    ImGui_ImplGlfw_InitForOpenGL(self->window.win, true); // can't be bothered to abstract
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);
}

bool rd_add_shader(Renderer* self, Arena *scratch, const char** shader_filepaths, u32 shader_count, u32* shader_out)
{
    *shader_out = 0;
    char version_str[BGL_RD_VERSION_STRLEN];
    rd_get_version_string(self, version_str);

    BLOCK_RESIZE_ARRAY(&self->shaders, Shader, self->shader_count, 1);
    bool ret = shader_create(&self->shaders[self->shader_count++], scratch, shader_filepaths, shader_count, version_str, RD_NO_BLOCK_BINDINGS(self));

    *shader_out = self->shader_count - 1;

    return ret;
}

void rd_use_shader(Renderer* self, u32 index)
{
    if(index != self->current_shader)
    {
        shader_use(&self->shaders[index]);
        self->current_shader = index;
    }
}

void rd_reload_shader(Renderer* self, u32 index)
{
    Shader shader;
    Arena scratch;
    char version_str[BGL_RD_VERSION_STRLEN];
    arena_create_sized(&scratch, KILOBYTES(512));
    rd_get_version_string(self, version_str);
    
    u32 source_count = 0;
    const char* sources[3];
    for(int i = 0; i < 3; i++)
    {
        const char* current_source = self->shaders[index].sources[i];
        sources[i] = current_source; // need to do this because function requires const char**, cannot take const char* []

        if(current_source[0] != '\0')
            source_count++;
    }
    if(shader_create(&shader, &scratch, sources, source_count, version_str, RD_NO_BLOCK_BINDINGS(self)))
    {
        shader_free(&self->shaders[index]);
        self->shaders[index] = shader;
    }
    
    arena_free(&scratch);
}

void rd_toggle_wireframe(bool on)
{
    if(on)
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void rd_cull_face(bool on, bool back)
{
    glCullFace(back ? GL_BACK : GL_FRONT);
    if(on)
    {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
    }
}

void rd_toggle_vsync(bool on)
{
    platform_toggle_vsync(on);
}

void rd_begin_frame(Renderer* self)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f64 curr_time = platform_get_time();
    self->delta_time = curr_time - self->last_time;
    self->last_time = curr_time; 

    #ifdef BGL_EDITOR
    if(!self->window.mouse_enabled) rd_editor_toggle_open_panes(self);
    if(self->editor_open) rd_editor_pane(self); 
    #endif
}

void rd_editor_toggle_open_panes(Renderer* self)
{
    u32 button_count = 0;
    igPushStyleVar_Vec2(ImGuiStyleVar_FramePadding, (ImVec2){10.0f, 10.0f});
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2){0, 0});
    igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, (ImVec2){0, 0});
    igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0, 0, 0, 0});
    igBeginMainMenuBar();
        for(u32 i = 0; i < self->pane_count; i++)
        {
            if(self->editor_panes[i].null_terminator == 0) // if string empty - another pane of same name
            {
                /* set bool of pane to copy pane of same name and don't make a button */
                *self->editor_flags[i] = *self->editor_flags[self->editor_panes[i].index];
                continue;
            }

            bool user = *self->editor_flags[i]; // flag might change after button call so store
            if(user) igPopStyleColor(1); // highlight button if pane activated
            if(igButton(self->editor_panes[i].str, (ImVec2){0, 0}))
            {
                *self->editor_flags[i] = !(*self->editor_flags[i]);
            }
            if(user) igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0, 0, 0, 0});

            button_count++;
        }
        igText(" %lu editor panes", button_count);
    igEndMainMenuBar();
    igPopStyleColor(1);
    igPopStyleVar(3);
}

void rd_editor_pane(Renderer* self)
{
    igBegin("renderer", NULL, 0);
        igText("fps: %f", 1.0f / self->delta_time);

        static bool vsync_on = true;
        if(igButton("toggle v-sync", (ImVec2){0, 0}))
        {
            vsync_on = !vsync_on;
            rd_toggle_vsync(vsync_on);
        }
        static bool wireframe_on = false;
        if(igButton("toggle wireframe", (ImVec2){0, 0}))
        {
            wireframe_on = !wireframe_on;
            rd_toggle_wireframe(wireframe_on);
        }

        /* shader hot reloading */
        static const char* current_item = NULL;
        static u32 current_index = 0;
        if(igBeginCombo("##shaders", current_item, 0))
        {
            for (u32 i = 0; i < self->shader_count; i++)
            {
                const char* name = self->shaders[i].name;
                                   
                bool current_item_selected = current_item == name;
                if(igSelectable_Bool(name, current_item_selected, 0, (ImVec2){0, 0}))
                {
                    current_item = name;
                    current_index = i;
                }
                if(current_item_selected)
                {
                    igSetItemDefaultFocus();
                }
            }
            igEndCombo();
        }
        if(igButton("hot reload shader", (ImVec2){0, 0}))
        {
            rd_reload_shader(self, current_index);
        }
    igEnd();
}

void rd_end_frame(Renderer* self)
{
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    window_swap_buffers(&self->window);
    window_poll_events(&self->window);

    self->framecount++;

}

void rd_free(Renderer* self)
{
    for(u32 i = 0; i < self->shader_count; i++)
    {
        shader_free(&self->shaders[i]);
    }
    if(self->shaders != NULL) BGL_FREE(self->shaders);

    char imgui_ini_path[256];
    platform_prepend_executable_directory(imgui_ini_path, 256, "imgui.ini");
    igSaveIniSettingsToDisk(imgui_ini_path);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(self->imgui_ctx);

    window_free(&self->window);
}

void rd_editor_add_pane(Renderer* self, const char* name, bool* user)
{
    #ifdef BGL_EDITOR
    BGL_ASSERT(self->pane_count < BGL_MAX_EDITOR_STR, "too many editor panes");
    *user = false;
    self->editor_flags[self->pane_count] = user;

    for(u32 i = 0; i < self->pane_count; i++)
    {
        if(strncmp(self->editor_panes[i].str, name, BGL_MAX_EDITOR_STR) == 0)
        {
            /* if we have a duplicate name we want it to have the same button,
             * so store index of matching name using union which we can use later */
            self->editor_panes[self->pane_count].null_terminator = 0;
            self->editor_panes[self->pane_count].index = i;
            self->pane_count++;
            return;
        }
    }

    memset(self->editor_panes[self->pane_count].str, 0, BGL_MAX_EDITOR_STR);
    strncpy(self->editor_panes[self->pane_count].str, name, BGL_MAX_EDITOR_STR);
    self->pane_count++;
    #endif
}

void rd_resize_callback(BGLWindow* window)
{
    glViewport(0, 0, window->width, window->height);
}

void APIENTRY rd_debug_callback(BGL_UNUSED GLenum source, BGL_UNUSED GLenum type,  u32 id,
                                GLenum severity, BGL_UNUSED GLsizei length,
                                const char *message, BGL_UNUSED const void *user_param)
{
    if(id == 131185) return; // ignore this garbage

    LogType log_type = BGL_LOG_INFO;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         log_type = BGL_LOG_ERROR; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       log_type = BGL_LOG_WARN; break;
        case GL_DEBUG_SEVERITY_LOW:          log_type = BGL_LOG_INFO; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: log_type = BGL_LOG_INFO; break;
    }

    BGL_LOG_NO_CTX(log_type, "OPENGLDEBUG ID - %d\n%s", id, message);
}

void rd_get_version_major_minor(Renderer* self, i32* major, i32* minor)
{
    char major_char = self->version[0];
    char minor_char = self->version[2];
    BGL_ASSERT(CHAR_IS_NUMBER(major_char) && CHAR_IS_NUMBER(minor_char), "invalid opengl version, missing numbers");
    BGL_ASSERT(self->version[1] == '.', "invalid opengl version, missing dot");

    *major = CHAR_TO_INT(major_char);    
    *minor = CHAR_TO_INT(minor_char);    
}

/* assumes buffer is set to BGL_RD_VERSION_STRLEN. since i only use this twice, keeping as is */
void rd_get_version_string(Renderer* self, char* buffer)
{
    /* index:                       0        9       */
    const char* version_template = "#version xx0 core";
    u32 length = (u32)strlen(version_template);

    i32 major, minor;
    rd_get_version_major_minor(self, &major, &minor);
    memcpy(buffer, version_template, length);
    buffer[length] = '\0';

    buffer[9] = INT_TO_CHAR(major);
    buffer[10] = INT_TO_CHAR(minor);
}
