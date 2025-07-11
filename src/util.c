#include "util.h"

#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* get_file_data(const char* filepath)
{
    FILE* file;
    file = fopen(filepath, "rb"); // windows will add carriage returns to \n in ftell count unless using binary read
    if(file == NULL || fseek(file, 0, SEEK_END))
    {
        return NULL;
    }

    i64 file_size = (i64)ftell(file); // ftell after seeking end gives file size
    if(file_size == -1)
    {
        return NULL;
    }
    char* file_data = (char*)BGL_MALLOC((u32)file_size + 1); // +1 for the null terminator
    
    fseek(file, 0, SEEK_SET);
    if(file_data == NULL || fread(file_data, sizeof(char), (u32)file_size, file) != (u32)file_size)
    {
        BGL_FREE(file_data); // free on NULL does nothing
        return NULL;
    }
    file_data[file_size] = '\0';

    fclose(file);
    return file_data;
}

/* returns NULL if str has no occurence of c */
char* str_find_last_of(const char* str, char c)
{
    if(str == NULL) return NULL;
    char* latest_occurrence = (char*)str;

    while(*str != '\0')
    {
        if(*str == c) latest_occurrence = (char*)str; // discarding const
        str++;
    }

    return latest_occurrence;
}

void find_directory_from_path(char* buffer, u32 length, const char* path)
{
    char* _last_slash = str_find_last_of(path, '/');
    char* _last_backslash = str_find_last_of(path, '\\'); // allow for windows (would prefer to have this in platform.h but oh well)
    char* name = _last_slash > _last_backslash ? _last_slash : _last_backslash

    BGL_ASSERT(last_char != NULL, "invalid path %s\n", path);
    u64 offset = (u64)(last_char - path);

    u64 str_end = offset < length ? offset : (u64)length;
    strncpy(buffer, path, str_end); // copy up to final / into directory
    buffer[str_end] = '\0';
}

bool array_contains(u32* array, u32 length, u32 val)
{
    for(u32 i = 0; i < length; i++)
    {
        if(val == array[i]) return true;
    }

    return false;
}
