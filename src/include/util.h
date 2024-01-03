#ifndef BADGL_UTIL_H
#define BADGL_UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include "glmath.h"

#ifdef BADGL_NO_DEBUG
    #define ASSERT(x) 
#else
    #define ASSERT(x, msg, ...)                 \
    {                                           \
        if(!(x))                                \
        {                                       \
            fprintf(stderr, msg, ##__VA_ARGS__);\
            exit(1);                            \
        }                                       \
    }
#endif

typedef struct Transform {
    vec3 pos;
    vec3 euler;
    vec3 scale;
} Transform;

char* get_file_data(const char* filepath);

int str_find_last_of(const char* str, char c);

void transform_reset(Transform* transform);

#endif