#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct Texture2D
{
    GLuint id;
    int width, height; // is this necessary?
} Texture2D;

int texture2d_create(Texture2D* texture_ptr, const char* img_filename, bool use_mipmap, bool use_alpha);