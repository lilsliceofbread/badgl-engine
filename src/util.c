#include "util.h"

#include "defines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* get_file_data(const char* filepath)
{
    FILE* file;
    file = fopen(filepath, "rb"); // windows will add carriage returns to \n in ftell count unless using binary read
    if(file == NULL || fseek(file, 0, SEEK_END)) // file not open or fseek failed
    {
        return NULL;
    }

    // since we seeked the end of file, ftell will return the position of the EOF or the file size
    i64 file_size = (i64)ftell(file);
    if(file_size == -1)
    {
        return NULL;
    }
    char* file_data = (char*)malloc((size_t)file_size + 1); // +1 for the null terminator
    
    fseek(file, 0, SEEK_SET); // i think since already checked fseek before should be fine
    if(file_data == NULL || fread(file_data, 1, (size_t)file_size, file) != (size_t)file_size)
    {
        free(file_data);
        return NULL;
    }
    file_data[file_size] = '\0';

    fclose(file);
    return file_data;
}

char* str_find_last_of(const char* str, char c)
{
    char* curr = (char*)str; // discarding const but not modifying the string
    char* latest_occurrence = NULL;

    while(curr != NULL && *curr != '\0')
    {
        if(*curr == c) latest_occurrence = curr;
        curr++;
    }

    return latest_occurrence;
}

void find_directory_from_path(char* dest, size_t size, const char* path)
{
    char* last_char = str_find_last_of(path, '/');
    BGL_ASSERT(last_char != NULL, "invalid path %s\n", path);
    i32 offset = (i32)(last_char - path);

    strncpy(dest, path, (size_t)offset < size ? (size_t)offset : size); // copy up to final / into directory
    dest[offset] = '\0';
}

void find_file_from_path(char* dest, size_t size, const char* path)
{
    char* last_char = str_find_last_of(path, '/');
    BGL_ASSERT(last_char != NULL, "invalid path %s\n", path);

    BGL_ASSERT(strlen(last_char) > 1, "path %s contains no file\n", path);
    strncpy(dest, last_char + 1, size);
}

bool array_contains(u32* array, u32 length, u32 val)
{
    for(u32 i = 0; i < length; i++)
    {
        if(val == array[i]) return true;
    }

    return false;
}

void transform_reset(Transform* transform)
{
    transform->pos = VEC3(0.0f, 0.0f, 0.0f);
    transform->euler = VEC3(0.0f, 0.0f, 0.0f);
    transform->scale = VEC3(1.0f, 1.0f, 1.0f);
}