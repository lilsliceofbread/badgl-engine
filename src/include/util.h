#ifndef BGL_UTIL_H
#define BGL_UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "defines.h"
#include "glmath.h"
#include "platform.h"

typedef struct Transform {
    vec3 pos;
    vec3 euler;
    vec3 scale;
} Transform;

char* get_file_data(const char* filepath);

int str_find_last_of(const char* str, char c);

// assumes user provides a dest large enough
void find_directory_from_path(char* dest, const char* path);

void transform_reset(Transform* transform);

bool array_contains(u32* array, u32 length, u32 val);

#endif