#include "renderer.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "shader.h"
#include "texture.h"
#include "util.h"

void rd_resize_callback(GLFWwindow* win, int width, int height);

void rd_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

void rd_mouse_callback(GLFWwindow* win, double cursor_x, double cursor_y);

void rd_init(Renderer* self, int width, int height, const char* win_title, const char* vert_src, const char* frag_src)
{
    self->height = height;
    self->width = width;
    self->cursor_disabled = true;
    self->mouse_wait_update = false;

    ASSERT(glfwInit(), "ERR: failed to init GLFW");

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

    ASSERT(win != NULL, "ERR: failed to open window");

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "ERR: failed to init GLAD");

    glfwSetWindowUserPointer(win, self);
    glfwSetKeyCallback(self->win, rd_key_callback);
    glfwSetCursorPosCallback(self->win, rd_mouse_callback);
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
    
    // make user defined shaders
    shader_init(&(self->shader), vert_src, frag_src);
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
    shader_free(&(self->shader));
    glfwTerminate();
}

bool rd_get_key(Renderer* self, int key)
{
    return GLFW_PRESS == glfwGetKey(self->win, key);
}

bool rd_win_should_close(Renderer* self)
{
    return glfwWindowShouldClose(self->win);
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
            rd->mouse_wait_update = true;
        }
        else
        {
            glfwFocusWindow(win);
            glfwSetCursorPos(win, rd->width/2, rd->height/2); // move cursor to middle of screen
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->cursor_disabled = true;
            rd->mouse_wait_update = false;
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

void rd_mouse_callback(GLFWwindow* win, double cursor_x, double cursor_y)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(win);
    rd->cursor_x = cursor_x;
    rd->cursor_y = cursor_y;
}