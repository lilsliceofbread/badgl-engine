#ifndef WINDOW_H
#define WINDOW_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "renderer.h"

GLFWwindow* window_init(Renderer* renderer);

#endif