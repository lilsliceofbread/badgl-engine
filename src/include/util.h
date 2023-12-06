#ifndef BADGL_UTIL_H
#define BADGL_UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define ASSERT(x, msg, ...)                 \
{                                           \
    if(!(x))                                \
    {                                       \
        fprintf(stderr, msg, ##__VA_ARGS__);\
        exit(1);                            \
    }                                       \
}

char* get_file_data(const char* filepath);

int str_find_last_of(const char* str, char c);

bool gl_extension_supported(const char* extension);

#endif