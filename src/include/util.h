#ifndef BGL_UTIL_H
#define BGL_UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"
#include "platform.h"
#include "defines.h"

char* get_file_data(const char* filepath);

char* str_find_last_of(const char* str, char c);

// may truncate output string to size if size is smaller than required
void find_directory_from_path(char* buffer, u32 length, const char* path);
void find_file_from_path(char* buffer, u32 length, const char* path);
void prepend_executable_directory(char* buffer, u32 length, const char* path);

bool array_contains(u32* array, u32 length, u32 val);

#endif