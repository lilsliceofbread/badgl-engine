#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

typedef struct WinContext
{
    int width;
    int height;
    GLuint shader_program;
    GLuint vbo;
    GLuint vao;
    GLuint ebo;
    GLuint textures[2];
} WinContext;

GLFWwindow* window_init(WinContext* ctx);
void window_loop(GLFWwindow* window, WinContext* ctx);