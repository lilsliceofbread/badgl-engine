#include "shader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
    
    fseek(file, 0, SEEK_SET); // i think since already checked fseek before should be fine
    if(shader_source == NULL || fread(shader_source, 1, file_size, file) != file_size)
    {
        fprintf(stderr, "ERR: could not open shader %s\n", shader_filepath);
        free(shader_source);
        return NULL;
    }
    shader_source[file_size] = '\0';

    fclose(file);
    return shader_source;
}

GLuint create_shader(const char* shader_filepath, GLenum shader_type, char* info_log, int log_size, int* success)
{
    GLuint shader;
    char* shader_src;

    shader_src = get_shader_source(shader_filepath);
    if(shader_src == NULL)
    {
        *success = false;
        return 0;
    }
    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char* const*)&shader_src, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);

    free(shader_src);

    int comp_success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &comp_success);
    if(!comp_success)
    {
        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        *success = false;
        return 0;
    }

    *success = true;
    return shader;
}