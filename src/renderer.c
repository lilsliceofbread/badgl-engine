#include "renderer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "shader.h"
#include "texture.h"
#include "util.h"

void rd_resize_callback(GLFWwindow* win, int width, int height);

void rd_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

void rd_init(Renderer* self, int width, int height, const char* win_title)
{
    self->width = width;
    self->height = height;
    self->cursor_disabled = true;
    self->cam_wait_update = NULL;
    self->shader_count = 0;
    self->shaders = NULL;

    ASSERT(glfwInit(), "RENDERER: failed to init GLFW");

    #ifndef NO_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);  
    #endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    GLFWwindow* win = glfwCreateWindow(width, height, win_title, NULL, NULL);

    self->win = win;

    ASSERT(win != NULL, "RENDERER: failed to open window");

    glfwMakeContextCurrent(win);
    glfwSetCursorPos(win, width / 2, height / 2);
    glfwSwapInterval(1);

    ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "RENDERER: failed to init GLAD");

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
            glDebugMessageCallback(gl_debug_callback, NULL);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        }
    #endif

    glViewport(0, 0, width, height);


    glEnable(GL_BLEND); // enable transparent textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // which blending function for transparency
    glEnable(GL_FRAMEBUFFER_SRGB); // use standard rgb for framebuffer
    //glDisable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black
}

int rd_add_shader(Renderer* self, const char* vert_src, const char* frag_src)
{
    self->shaders = (Shader*)realloc(self->shaders, (self->shader_count + 1) * sizeof(Shader));
    ASSERT(self->shaders != NULL, "RENDERER: failed to reallocate shader array");
    shader_init(&self->shaders[self->shader_count], vert_src, frag_src);

    self->shader_count++;
    return self->shader_count - 1; // returns index to shader in array
}

Shader* rd_get_shader(Renderer* self, int index)
{
    return &self->shaders[index];
}

void rd_set_wireframe(bool useWireframe)
{
    GLenum mode = useWireframe ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, mode);
}

void rd_begin_frame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rd_end_frame(Renderer* self)
{
    glfwSwapBuffers(self->win);
    glfwPollEvents();
}

void rd_free(Renderer* self)
{
    if(self->shader_count) // if not allocated don't free
    {
        for(int i = 0; i < self->shader_count; i++)
        {
            shader_free(&self->shaders[i]);
        }
        free(self->shaders);
    }

    glfwDestroyWindow(self->win);
    glfwTerminate(); // might remove if multiple renderers used
}

bool rd_get_key(Renderer* self, int key)
{
    return GLFW_PRESS == glfwGetKey(self->win, key);
}

bool rd_win_should_close(Renderer* self)
{
    return glfwWindowShouldClose(self->win);
}

void rd_set_cam_bool(Renderer* self, bool* cam_bool)
{
    self->cam_wait_update = cam_bool;
    *cam_bool = true;
}

void rd_resize_callback(GLFWwindow* win, int width, int height)
{
    glViewport(0, 0, width, height);
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win); 
    rd->width = width;
    rd->height = height;
    
    //printf("window resized - w: %d h: %d\n", width, height);
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
        }
        else
        {
            glfwFocusWindow(win);
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->cursor_disabled = true;
            if(rd->cam_wait_update) *(rd->cam_wait_update) = true;
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

void rd_get_cursor_pos(Renderer* self, float* cursor_x, float* cursor_y)
{
    double xpos, ypos; // have to do this since glfw uses doubles
    glfwGetCursorPos(self->win, &xpos, &ypos);
    
    *cursor_x = (float)xpos;
    *cursor_y = (float)ypos;
}