#ifndef UTIL_H
#define UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#define ASSERT(x, msg, ...)                 \
{                                           \
    if(!(x))                                \
    {                                       \
        fprintf(stderr, msg, ##__VA_ARGS__);\
        exit(1);                            \
    }                                       \
}

#define gl_check_error() gl_check_error_(__FILE__, __LINE__)

char* get_file_data(const char* filepath);

GLenum gl_check_error_(const char* file, int line);

void APIENTRY gl_debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *user_param);

#endif