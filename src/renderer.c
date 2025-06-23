#include "renderer.h"

#include "force_dgpu.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "platform.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

#define BGL_RD_VERSION_STRLEN 24 // bit extra to make it multiple of 8

/**
 * internal functions
 */ void rd_configure_gl(Renderer* self);
void rd_imgui_init(Renderer* self, const char* glsl_version);
void rd_resize_callback(GLFWwindow* win, i32 width, i32 height);
void rd_key_callback(GLFWwindow* win, i32 key, i32 scancode, i32 action, i32 mods);
void APIENTRY rd_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length,
                                const char *message, const void *user_param);
void rd_get_version_major_minor(Renderer* self, i32* major, i32* minor);
void rd_get_version_string(Renderer* self, char* buffer);

void rd_init(Renderer* self, i32 width, i32 height, const char* win_title, RendererFlags flags, const char* version)
{
    self->width = width;
    self->height = height;
    self->mouse_wait_update = 1;
    self->mouse_should_update = false;
    self->shader_count = 0;
    self->shaders = NULL;
    self->delta_time = 0.0;
    self->last_time = 0.0;
    self->framecount = 0;
    self->flags = flags & (RendererFlags)~(_BGL_RD_CURSOR_DISABLED | _BGL_RD_VSYNC_ENABLED); // set both to false by default

    i32 major, minor;
    BGL_ASSERT(strlen(version) >= 3, "invalid opengl version");
    memcpy(self->version, version, 3);
    rd_get_version_major_minor(self, &major, &minor);

    i32 ret = glfwInit();
    BGL_ASSERT(ret, "failed to init GLFW");

    #ifndef BGL_NO_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    GLFWwindow* win = glfwCreateWindow(width, height, win_title, NULL, NULL);
    self->win = win;
    BGL_ASSERT(win != NULL, "failed to open window");

    glfwMakeContextCurrent(win);
    glfwSetCursorPos(win, width / 2, height / 2);
    glfwSetWindowUserPointer(win, self);
    glfwSetKeyCallback(self->win, rd_key_callback);
    glfwSetFramebufferSizeCallback(win, rd_resize_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ret = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    BGL_ASSERT(ret, "failed to init GLAD");

    rd_configure_gl(self);

    const char* shader_filepaths[] = {"shaders/skybox.glsl", "shaders/quad.glsl", "shaders/light.glsl"};

    self->skybox_shader = self->flags & BGL_RD_SKYBOX_OFF
                                      ? 0 : rd_add_shader(self, &shader_filepaths[0], 1);
    self->quad_shader   = self->flags & BGL_RD_UI_OFF
                                      ? 0 : rd_add_shader(self, &shader_filepaths[1], 1);
    self->light_shader  = self->flags & BGL_RD_LIGHTING_OFF
                                      ? 0 : rd_add_shader(self, &shader_filepaths[2], 1);
    
    platform_init();

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

    glViewport(0, 0, self->width, self->height);

    glEnable(GL_BLEND); // enable transparent textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_FRAMEBUFFER_SRGB) // this makes everything look oversaturated and garbage

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
    glfwGetFramebufferSize(self->win, &self->width, &self->height);
    glViewport(0, 0, self->width, self->height);
}

void rd_set_viewport(i32 x, i32 y, i32 width, i32 height)
{
    glViewport(x, y, width, height);
}

// allow manual swapping of buffers
void rd_swap_buffers(Renderer* self)
{
    glfwSwapBuffers(self->win);
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

    ImGui_ImplGlfw_InitForOpenGL(self->win, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);
}

u32 rd_add_shader(Renderer* self, const char** shader_filepaths, u32 shader_count)
{
    char version_str[BGL_RD_VERSION_STRLEN];
    rd_get_version_string(self, version_str);

    BLOCK_RESIZE_ARRAY(&self->shaders, Shader, self->shader_count, 1);
    // TODO: make shader_create return bool if failed and check
    shader_create(&self->shaders[self->shader_count++], shader_filepaths, shader_count, version_str);

    return self->shader_count - 1;
}

void rd_set_wireframe(bool useWireframe)
{
    if(useWireframe)
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

void rd_begin_frame(Renderer* self)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f64 curr_time = platform_get_time();
    self->delta_time = curr_time - self->last_time;
    self->last_time = curr_time; 
}

void rd_end_frame(Renderer* self)
{
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    glfwSwapBuffers(self->win);
    glfwPollEvents();

    self->framecount++;

    if(self->mouse_wait_update > 0)
    {
        self->mouse_wait_update--;
        self->mouse_should_update = false;
        return;
    }
    else if(self->flags & _BGL_RD_CURSOR_DISABLED)
    {
        self->mouse_should_update = false;
        return;
    }

    self->mouse_should_update = true;
}

bool rd_key_pressed(Renderer* self, i32 key)
{
    return GLFW_PRESS == glfwGetKey(self->win, key);
}

bool rd_win_should_close(Renderer* self)
{
    return glfwWindowShouldClose(self->win);
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

    glfwDestroyWindow(self->win);
    glfwTerminate();
}

void rd_resize_callback(GLFWwindow* win, i32 width, i32 height)
{
    glViewport(0, 0, width, height);
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win); 
    rd->width = width;
    rd->height = height;
}

void rd_key_callback(GLFWwindow* win, i32 key, BGL_UNUSED i32 scancode, i32 action, BGL_UNUSED i32 mods)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win);

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(rd->flags & _BGL_RD_CURSOR_DISABLED) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(win, rd->width / 2, rd->height / 2);

            rd->flags &= (RendererFlags)~(_BGL_RD_CURSOR_DISABLED);
            rd->mouse_wait_update = 1; // mouse updates should wait 1 frame to prevent flicking
        }
        else
        {
            glfwFocusWindow(win);
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            rd->flags |= _BGL_RD_CURSOR_DISABLED;
        }
    }
    if(key == GLFW_KEY_P)
    {
        rd_set_wireframe(action != GLFW_RELEASE);
    }
}

void rd_get_cursor_pos(Renderer* self, f64* cursor_x_out, f64* cursor_y_out)
{
    glfwGetCursorPos(self->win, cursor_x_out, cursor_y_out);
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
