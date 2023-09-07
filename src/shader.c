#include "shader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* get_shader_source(const char* shader_filepath)
{
    FILE* file;
    file = fopen(shader_filepath, "r");
    if(file == NULL || fseek(file, 0, SEEK_END)) // file open or fseek failed
    {
        fprintf(stderr, "ERR: could not open shader %s\n", shader_filepath);
        return NULL;
    }

    // get file size
    long file_size = ftell(file);
    if(file_size == -1)
    {
        fprintf(stderr, "ERR: could not open shader %s\n", shader_filepath);
        return NULL;
    }
    char* shader_source = (char*)malloc((size_t)file_size + 1); // should be big enough
    
    fseek(file, 0, SEEK_SET);
    ;
    if(shader_source == NULL || fread(shader_source, 1, file_size, file) != file_size)
    {
        fprintf(stderr, "ERR: could not open shader %s\n", shader_filepath);
        return NULL;
    }

    fclose(file);
    shader_source[file_size] = '\0';
    return shader_source;
}