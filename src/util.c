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
    char* file_data = (char*)malloc((u32)file_size + 1); // +1 for the null terminator
    
    fseek(file, 0, SEEK_SET); // i think since already checked fseek before should be fine
    if(file_data == NULL || fread(file_data, 1, (u32)file_size, file) != (u32)file_size)
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

void find_directory_from_path(char* buffer, u32 length, const char* path)
{
    char* last_char = str_find_last_of(path, FILEPATH_SEPARATOR);
    BGL_ASSERT(last_char != NULL, "invalid path %s\n", path);
    i32 offset = (i32)(last_char - path);

    u32 str_end = (u32)offset < length ? (u32)offset : length;
    strncpy(buffer, path, str_end); // copy up to final / into directory
    buffer[str_end] = '\0';
}

void find_file_from_path(char* buffer, u32 length, const char* path)
{
    char* last_char = str_find_last_of(path, FILEPATH_SEPARATOR);
    BGL_ASSERT(last_char != NULL, "invalid path %s\n", path);

    BGL_ASSERT(strlen(last_char) > 1, "path %s contains no file\n", path);
    strncpy(buffer, last_char + 1, length);
    buffer[length - 1] = '\0'; // just in case
}

void prepend_executable_directory(char* buffer, u32 length, const char* path)
{
    // TODO: cache this
    char exe_path[512];
    char directory[512];
    platform_get_executable_path(exe_path, 512);
    find_directory_from_path(directory, 512, exe_path);
    if(strstr(path, directory)) // if already full path
    {
        strncpy(buffer, path, length);
        return;
    }
    
    snprintf(buffer, length, "%s%c%s", directory, FILEPATH_SEPARATOR, path);
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