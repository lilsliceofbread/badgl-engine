#include "renderer.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "shader.h"
#include "texture.h"
#include "util.h"
#ifdef __linux__
    #include <GL/glx.h>
    #include <GL/glxext.h>

    PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
    // the GLX_EXT_swap_control extension does not have
    // a GetSwapIntervalEXT func, use glXQueryDrawable instead
#elif _WIN32
    #include <windows.h>
    #include "wglext.h"

    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
    //PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;
#endif

bool rd_get_vsync_functions(Renderer* self);

void rd_resize_callback(GLFWwindow* win, int width, int height);

void rd_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

void rd_init(Renderer* self, int width, int height, const char* win_title)
{
    self->width = width;
    self->height = height;
    self->cursor_disabled = true;
    self->mouse_wait_update = 1;
    self->shader_count = 0;
    self->shaders = NULL;
    self->delta_time = 0.0f;
    self->last_time = 0.0f;
    self->framecount = 0;
    self->vsync_enabled = false;

    ASSERT(glfwInit(), "RENDERER: failed to init GLFW\n");

    #ifndef NO_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // set to 1 if apple?
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    /* Mac doesn't support OpenGL 4.3 anyway :/
     * #ifdef __APPLE__
     *    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
     * #endif
     */
    
    GLFWwindow* win = glfwCreateWindow(width, height, win_title, NULL, NULL);

    self->win = win;

    ASSERT(win != NULL, "RENDERER: failed to open window\n");

    glfwMakeContextCurrent(win);
    glfwSetCursorPos(win, width / 2, height / 2);

    ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "RENDERER: failed to init GLAD\n");


    glfwSetWindowUserPointer(win, self);
    glfwSetKeyCallback(self->win, rd_key_callback);
    glfwSetFramebufferSizeCallback(win, rd_resize_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    #ifndef NO_DEBUG
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

    if(rd_get_vsync_functions(self))
    {
        rd_toggle_vsync(true);
    }
    else 
    {
        fprintf(stderr, "RENDERER: OpenGL vsync extension not enabled");
    }

    glViewport(0, 0, width, height);

    rd_imgui_init(self, "#version 430 core");

    glEnable(GL_BLEND); // enable transparent textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // which blending function for transparency
    glEnable(GL_FRAMEBUFFER_SRGB); // use standard rgb for framebuffer
    //glDisable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW); // front face has clockwise vertices
    glCullFace(GL_FRONT);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black
}

void rd_imgui_init(Renderer* self, const char* glsl_version)
{
    self->imgui_ctx = igCreateContext(NULL); 
    self->imgui_io = igGetIO(); 
    ImGui_ImplGlfw_InitForOpenGL(self->win, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);
}

uint32_t rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src)
{
    self->shaders = (Shader*)realloc(self->shaders, (self->shader_count + 1) * sizeof(Shader));
    ASSERT(self->shaders != NULL, "RENDERER: failed to reallocate shader array\n");
    shader_init(&self->shaders[self->shader_count], vert_src, frag_src);

    self->shader_count++;
    return self->shader_count - 1; // returns index to shader in array
}

void rd_set_wireframe(bool useWireframe)
{
    GLenum mode = useWireframe ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, mode);
}

void rd_begin_frame(Renderer* self)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float curr_time = rd_get_time();
    self->delta_time = curr_time - self->last_time;
    self->last_time = curr_time; 
    
    //printf("FPS: %f\n", 1.0f / self->delta_time);
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

float rd_get_time(void)
{
    return (float)glfwGetTime();
}

bool rd_key_pressed(Renderer* self, int key)
{
    return GLFW_PRESS == glfwGetKey(self->win, key);
}

bool rd_win_should_close(Renderer* self)
{
    return glfwWindowShouldClose(self->win);
}

bool rd_get_vsync_functions(Renderer* self) {
    #ifdef __linux__
        if(!gl_extension_supported("GLX_EXT_swap_control")) return false;

        glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
    #elif _WIN32
        if(!gl_extension_supported("WGL_EXT_swap_control")) return false;

        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
        //wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");
    #elif __APPLE__
        // no apple support yet
    #endif

    self->vsync_enabled = true;
    return true;
}

void rd_toggle_vsync(bool on) {
    #ifdef __linux__
        Display* display = glXGetCurrentDisplay();
        GLXDrawable drawable = glXGetCurrentDrawable();

        glXSwapIntervalEXT(display, drawable, (int)on);
    #elif _WIN32
        wglSwapIntervalEXT((int)on);
    #elif __APPLE__
        // no apple support yet
    #endif
}

void rd_free(Renderer* self)
{
    if(self->shader_count > 0) // if not allocated don't free
    {
        for(uint32_t i = 0; i < self->shader_count; i++)
        {
            shader_free(&self->shaders[i]);
        }
        free(self->shaders);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(self->imgui_ctx);

    glfwDestroyWindow(self->win);
    glfwTerminate(); // might remove if multiple renderers used
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
        glfwSetWindowShouldClose(win, 1); // true
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(rd->cursor_disabled) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            rd->cursor_disabled = false;
            rd->mouse_wait_update = 2; // mouse updates should wait 2 frames to prevent flicking
        }
        else
        {
            glfwFocusWindow(win);
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->cursor_disabled = true;
            rd->mouse_wait_update = 2;
        }
    }
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        rd_set_wireframe(true);
    }
    if(key == GLFW_KEY_P && action == GLFW_RELEASE)
    {
        rd_set_wireframe(false);
    }
}

void rd_get_cursor_pos(Renderer* self, float* cursor_x_out, float* cursor_y_out)
{
    double xpos, ypos; // have to do this since glfw uses doubles
    glfwGetCursorPos(self->win, &xpos, &ypos);
    
    *cursor_x_out = (float)xpos;
    *cursor_y_out = (float)ypos;
}

void APIENTRY rd_debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *user_param)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    printf("GLDEBUG: ID - %d\nMESSAGE: %s\n", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             printf("SOURCE: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("SOURCE: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("SOURCE: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("SOURCE: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     printf("SOURCE: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER:           printf("SOURCE: Other\n"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               printf("TYPE: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("TYPE: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("TYPE: Undefined Behaviour\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         printf("TYPE: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         printf("TYPE: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              printf("TYPE: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          printf("TYPE: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           printf("TYPE: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               printf("TYPE: Other\n"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         printf("SEVERITY: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       printf("SEVERITY: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          printf("SEVERITY: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("SEVERITY: notification\n"); break;
    }
}