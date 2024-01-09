#ifndef BADGL_UTIL_H
#define BADGL_UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "glmath.h"
#include "platform.h"

#ifdef BADGL_NO_DEBUG
    #define ASSERT(x, msg, ...)

    #define PERF_TIMER_START()
    #define PERF_TIMER_END(str)

    #define BADGL_LOG(str, ...)
#else
    #define BADGL_LOG(str, ...) printf(str, ##__VA_ARGS__)

    #define ASSERT(x, msg, ...)                 \
    {                                           \
        if(!(x))                                \
        {                                       \
            fprintf(stderr, msg, ##__VA_ARGS__);\
            exit(1);                            \
        }                                       \
    }

    #define PERF_TIMER_START() double perf_start_time = platform_get_time()
    #define PERF_TIMER_END(str) printf("%s took %lfs\n", str, platform_get_time() - perf_start_time) 
#endif

#define ALIGNED_SIZE(size, alignment) ((size) % (alignment) == 0) ? (size) : (size) + ((alignment) - ((size) % (alignment)))

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

bool array_contains(uint32_t* array, uint32_t length, uint32_t val);

#endif