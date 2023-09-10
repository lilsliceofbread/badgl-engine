#include "shader.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int create_shader(GLuint* shader_ptr, const char* shader_filepath, GLenum shader_type, char* info_log, int log_size)
{
    char* shader_src;
    GLuint shader;

    shader_src = get_file_data(shader_filepath);
    if(shader_src == NULL)
    {
        return false;
    }
    shader = glCreateShader(shader_type);

    *shader_ptr = shader;

    glShaderSource(shader, 1, (const char* const*)&shader_src, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);
    free(shader_src);

    int comp_success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &comp_success);
    if(!comp_success)
    {
        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        return false;
    }

    return true;
}

int create_shader_program(GLuint* shader_program_ptr, const char* vert_shader_src, const char* frag_shader_src)
{
    GLuint vert_shader, frag_shader, shader_program;
    int success;
    char info_log[512];
    info_log[0] = '\0'; // if shader creation failed but no info log

    success = create_shader(&vert_shader, vert_shader_src, GL_VERTEX_SHADER, info_log, sizeof(info_log));
    if(!success)
    {
        fprintf(stderr, "ERR: failed to create vertex shader\n%s", info_log);
        return false;
    }

    success = create_shader(&frag_shader, frag_shader_src, GL_FRAGMENT_SHADER, info_log, sizeof(info_log));
    if(!success)
    {
        fprintf(stderr, "ERR: failed to create fragment shader\n%s", info_log);
        return false;
    }

    shader_program = glCreateProgram();

    *shader_program_ptr = shader_program;

    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "ERR: could not create shader program\n%s", info_log);
        return false;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return true;
}