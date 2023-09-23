#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// 'forward declare' bool
#undef bool
#define bool _Bool

void texture2d_create(int* texture_ptr, const char* img_filename, bool use_mipmap, bool use_alpha);

#endif