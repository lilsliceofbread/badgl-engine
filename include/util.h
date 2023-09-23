#ifndef UTIL_H
#define UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#ifdef __linux__
#define DEBUG_BREAK() __builtin_trap()
#elif _WIN32
#define DEBUG_BREAK() __debug_break()
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#endif

// msg assert by cakez77
#define ASSERT(x, msg, ...)                 \
{                                           \
    if(!(x))                                \
    {                                       \
        fprintf(stderr, msg, ##__VA_ARGS__);\
        abort();                            \
    }                                       \
}

#define gl_check_error() gl_check_error_(__FILE__, __LINE__)

char* get_file_data(const char* filepath);

GLenum gl_check_error_(const char* file, int line);

void APIENTRY gl_debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *user_param);

#endif