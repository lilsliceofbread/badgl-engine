#ifndef BGL_UTIL_H
#define BGL_UTIL_H

#include "defines.h"

char* get_file_data(const char* filepath);

char* str_find_last_of(const char* str, char c);

/* may truncate output string to size if length is smaller than required */
void find_directory_from_path(char* buffer, u32 length, const char* path);

bool array_contains(u32* array, u32 length, u32 val);

#endif
