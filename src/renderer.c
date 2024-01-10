#include "renderer.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "platform.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

void rd_resize_callback(GLFWwindow* win, int width, int height);

void rd_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

void rd_init(Renderer* self, int width, int height, const char* win_title)
{
    self->width = width;
    self->height = height;
    self->cursor_disabled = false;
    self->mouse_wait_update = 1;
    self->shader_count = 0;
    self->shaders = NULL;
    self->delta_time = 0.0;
    self->last_time = 0.0;
    self->framecount = 0;
    self->vsync_enabled = false;

    int ret = glfwInit();
    ASSERT(ret, "RENDERER: failed to init GLFW\n");

    #ifndef BADGL_NO_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    GLFWwindow* win = glfwCreateWindow(width, height, win_title, NULL, NULL);
    self->win = win;
    ASSERT(win != NULL, "RENDERER: failed to open window\n");

    glfwMakeContextCurrent(win);
    glfwSetCursorPos(win, width / 2, height / 2);
    glfwSetWindowUserPointer(win, self);
    glfwSetKeyCallback(self->win, rd_key_callback);
    glfwSetFramebufferSizeCallback(win, rd_resize_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ret = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    ASSERT(ret, "RENDERER: failed to init GLAD\n");

    rd_configure_gl(self);

    self->skybox_shader = rd_add_shader(self, "shaders/skybox.vert", "shaders/skybox.frag");
    self->quad_shader = rd_add_shader(self, "shaders/quad.vert", "shaders/quad.frag");
    self->light_shader = rd_add_shader(self, "shaders/light.vert", "shaders/light.frag");

    rd_imgui_init(self, "#version 430 core");
}

void rd_configure_gl(Renderer* self)
{
    #ifndef BADGL_NO_DEBUG
        int flags;
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
        self->vsync_enabled = true;
        platform_toggle_vsync(true);
    }
    else 
    {
        BADGL_LOG("RENDERER: OpenGL vsync extension not enabled");
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

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    platform_reset_time();
}

void rd_draw_triangles(uint32_t ind_count)
{
    glDrawElements(GL_TRIANGLES, (int)ind_count, GL_UNSIGNED_INT, 0);
}

// annoying fix, sometimes resize callback is delayed
void rd_update_viewport(Renderer* self)
{
    glfwGetFramebufferSize(self->win, &self->width, &self->height);
    glViewport(0, 0, self->width, self->height);
}

void rd_set_viewport(int x, int y, int width, int height)
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

void rd_reallocate_shaders(Renderer* self, uint32_t new_count)
{
    uint32_t shader_array_size = ALIGNED_SIZE(self->shader_count, SHADER_ALLOC_SIZE);
    if(new_count > shader_array_size)
    {
        uint32_t new_array_size = ALIGNED_SIZE(new_count, SHADER_ALLOC_SIZE);

        self->shaders = (Shader*)realloc(self->shaders, new_array_size * sizeof(Shader));
        ASSERT(self->shaders != NULL, "RENDERER: shader array reallocation failed");
        BADGL_LOG("RENDERER: shader array resize from %u to %u\n", shader_array_size, new_array_size);
    }
}

uint32_t rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src)
{
    rd_reallocate_shaders(self, self->shader_count + 1);
    shader_init(&self->shaders[self->shader_count++], vert_src, frag_src);

    return self->shader_count - 1;
}

Shader* rd_get_shader(Renderer* self, uint32_t index)
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

    if(self->mouse_wait_update > 0) // if mouse still needs to wait
        self->mouse_wait_update--;

    self->framecount++;
}

bool rd_key_pressed(Renderer* self, int key)
{
    return GLFW_PRESS == glfwGetKey(self->win, key);
}

bool rd_win_should_close(Renderer* self)
{
    return glfwWindowShouldClose(self->win);
}

void rd_free(Renderer* self)
{
    for(uint32_t i = 0; i < self->shader_count; i++)
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

void rd_resize_callback(GLFWwindow* win, int width, int height)
{
    glViewport(0, 0, width, height);
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win); 
    rd->width = width;
    rd->height = height;
}

void rd_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win);

    if(key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(rd->cursor_disabled) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->cursor_disabled = false;
            rd->mouse_wait_update = 2; // mouse updates should wait 2 frames to prevent flicking
        }
        else
        {
            glfwFocusWindow(win);
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            rd->cursor_disabled = true;
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

void APIENTRY rd_debug_callback(GLenum source, GLenum type, unsigned int id,
                                GLenum severity, GLsizei length,
                                const char *message, const void *user_param)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    BADGL_LOG("GLDEBUG: ID - %d\nMESSAGE: %s\n", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             BADGL_LOG("SOURCE: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   BADGL_LOG("SOURCE: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: BADGL_LOG("SOURCE: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     BADGL_LOG("SOURCE: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     BADGL_LOG("SOURCE: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER:           BADGL_LOG("SOURCE: Other\n"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               BADGL_LOG("TYPE: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: BADGL_LOG("TYPE: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  BADGL_LOG("TYPE: Undefined Behaviour\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         BADGL_LOG("TYPE: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         BADGL_LOG("TYPE: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              BADGL_LOG("TYPE: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          BADGL_LOG("TYPE: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           BADGL_LOG("TYPE: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               BADGL_LOG("TYPE: Other\n"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         BADGL_LOG("SEVERITY: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       BADGL_LOG("SEVERITY: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          BADGL_LOG("SEVERITY: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: BADGL_LOG("SEVERITY: notification\n"); break;
    }
}