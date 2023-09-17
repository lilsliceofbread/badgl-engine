#ifndef WINDOW_H
#define WINDOW_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// forward declarations
//typedef struct Renderer Renderer;
#include "renderer.h" // FIX LATER

typedef struct WinContext
{
    int width;
    int height;

    Renderer renderer;
} WinContext;

GLFWwindow* window_init(WinContext* ctx);

#endif