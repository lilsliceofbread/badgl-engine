#ifndef BGL_UTIL_H
#define BGL_UTIL_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"
#include "defines.h"

#define CHAR_IS_NUMBER(c) (0x30 <= (c) && (c) <= 0x39)
#define CHAR_TO_INT(c) (i32)((c) - 0x30)
#define INT_TO_CHAR(i) (char)((i) + 0x30)

char* get_file_data(const char* filepath);

char* str_find_last_of(const char* str, char c);

// may truncate output string to size if size is smaller than required
void find_directory_from_path(char* buffer, u32 length, const char* path);
void find_file_from_path(char* buffer, u32 length, const char* path);

bool array_contains(u32* array, u32 length, u32 val);

#endif
