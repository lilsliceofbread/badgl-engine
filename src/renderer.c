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

/**
 * internal functions
 */
void rd_configure_gl(Renderer* self);
void rd_imgui_init(Renderer* self, const char* glsl_version);
void rd_resize_callback(GLFWwindow* win, i32 width, i32 height);
void rd_key_callback(GLFWwindow* win, i32 key, i32 scancode, i32 action, i32 mods);
void rd_reallocate_shaders(Renderer* self, u32 new_count);
void APIENTRY rd_debug_callback(GLenum source, GLenum type, u32 id,
                                GLenum severity, GLsizei length,
                                const char *message, const void *user_param);

void rd_init(Renderer* self, i32 width, i32 height, const char* win_title, RendererFlags flags)
{
    self->width = width;
    self->height = height;
    self->mouse_wait_update = 1;
    self->shader_count = 0;
    self->shaders = NULL;
    self->delta_time = 0.0;
    self->last_time = 0.0;
    self->framecount = 0;
    self->flags = flags & (RendererFlags)~(RD_INTERNAL_CURSOR_DISABLED | RD_INTERNAL_VSYNC_ENABLED); // set both to false by default

    i32 ret = glfwInit();
    BGL_ASSERT(ret, "failed to init GLFW\n");

    #ifndef BGL_NO_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    GLFWwindow* win = glfwCreateWindow(width, height, win_title, NULL, NULL);
    self->win = win;
    BGL_ASSERT(win != NULL, "failed to open window\n");

    glfwMakeContextCurrent(win);
    glfwSetCursorPos(win, width / 2, height / 2);
    glfwSetWindowUserPointer(win, self);
    glfwSetKeyCallback(self->win, rd_key_callback);
    glfwSetFramebufferSizeCallback(win, rd_resize_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ret = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    BGL_ASSERT(ret, "failed to init GLAD\n");

    rd_configure_gl(self);

    self->skybox_shader = self->flags & RD_USE_SKYBOX
                                      ? rd_add_shader(self, "shaders/skybox.vert", "shaders/skybox.frag") : 0;
    self->quad_shader   = self->flags & RD_USE_UI
                                      ? rd_add_shader(self, "shaders/quad.vert", "shaders/quad.frag") : 0;
    self->light_shader  = self->flags & RD_USE_LIGHTING
                                      ? rd_add_shader(self, "shaders/light.vert", "shaders/light.frag") : 0;

    rd_imgui_init(self, "#version 430 core");

    platform_reset_time();
}

void rd_configure_gl(Renderer* self)
{
    #ifndef BGL_NO_DEBUG
        i32 flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(rd_debug_callback, NULL);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        }
    #endif

    if(platform_init_vsync())
    {
        self->flags |= RD_INTERNAL_VSYNC_ENABLED;
        platform_toggle_vsync(true);
    }
    else 
    {
        BGL_LOG(LOG_DEBUG, "OpenGL vsync extension not enabled");
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

void rd_set_viewport(int x, i32 y, i32 width, i32 height)
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
    ImGui_ImplGlfw_InitForOpenGL(self->win, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);
}

void rd_reallocate_shaders(Renderer* self, u32 new_count)
{
    u32 shader_array_size = ALIGNED_SIZE(self->shader_count, RD_SHADER_ALLOC_SIZE);
    if(new_count > shader_array_size)
    {
        u32 new_array_size = ALIGNED_SIZE(new_count, RD_SHADER_ALLOC_SIZE);

        self->shaders = (Shader*)realloc(self->shaders, new_array_size * sizeof(Shader));
        BGL_ASSERT(self->shaders != NULL, "shader array reallocation failed");
        BGL_LOG(LOG_DEBUG, "shader array resize from %u to %u\n", shader_array_size, new_array_size);
    }
}

u32 rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src)
{
    rd_reallocate_shaders(self, self->shader_count + 1);
    shader_create(&self->shaders[self->shader_count++], vert_src, frag_src);

    return self->shader_count - 1;
}

Shader* rd_get_shader(Renderer* self, u32 index)
{
    return &self->shaders[index];
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
    double curr_time = platform_get_time();
    self->delta_time = curr_time - self->last_time;
    self->last_time = curr_time; 
}

void rd_end_frame(Renderer* self)
{
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    glfwSwapBuffers(self->win);
    glfwPollEvents();

    if(self->mouse_wait_update > 0) self->mouse_wait_update--;

    self->framecount++;
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
    if(self->shaders != NULL) free(self->shaders);

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

void rd_key_callback(GLFWwindow* win, i32 key, i32 scancode, i32 action, i32 mods)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win);

    if(key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(rd->flags & RD_INTERNAL_CURSOR_DISABLED) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->flags &= (RendererFlags)~(RD_INTERNAL_CURSOR_DISABLED);
            rd->mouse_wait_update = 2; // mouse updates should wait 2 frames to prevent flicking
        }
        else
        {
            glfwFocusWindow(win);
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            rd->flags |= RD_INTERNAL_CURSOR_DISABLED;
        }
    }
    if(key == GLFW_KEY_P)
    {
        rd_set_wireframe(action != GLFW_RELEASE);
    }
}

void rd_get_cursor_pos(Renderer* self, float* cursor_x_out, float* cursor_y_out)
{
    double xpos, ypos; // have to do this since glfw uses doubles
    glfwGetCursorPos(self->win, &xpos, &ypos);
    
    *cursor_x_out = (float)xpos;
    *cursor_y_out = (float)ypos;
}

void APIENTRY rd_debug_callback(GLenum source, GLenum type,  u32 id,
                                GLenum severity, GLsizei length,
                                const char *message, const void *user_param)
{
    // ignore non-significant error/warning codes
    //if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 
    if(id == 131185) return; // ignore this garbage

    LogType log_type;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         log_type = LOG_ERROR; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       log_type = LOG_WARN; break;
        case GL_DEBUG_SEVERITY_LOW:          log_type = LOG_DEBUG; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: log_type = LOG_DEBUG; break;
    }

    BGL_LOG_NO_CTX(log_type, "OPENGLDEBUG ID - %d\nMESSAGE: %s\n\n", id, message);
}