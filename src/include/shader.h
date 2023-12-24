#ifndef BADGL_SHADER_H
#define BADGL_SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"

#define MAX_UNIF_NAME 64
#define MAX_UNIFORMS 16

typedef struct uniform_pair {
    char name[MAX_UNIF_NAME]; // if you make a uniform name longer than 64 characters i don't like you
    GLint location;
} uniform_pair;

// shader program
typedef struct Shader
{
    GLuint id;
    uniform_pair stored_uniforms[MAX_UNIFORMS];
    uint32_t uniform_count; // found during shader comp
} Shader;

void shader_init(Shader* self, const char* vert_shader_src, const char* frag_shader_src);

GLuint shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, int log_size, int* success_out);

void shader_find_uniforms_in_source(Shader* self, const char* src_code); // UNIFORMS ARE REPEATED IF DUPLICATED IN SEPARATE SHADER SOURCES

void shader_use(Shader* self);

GLint shader_find_uniform(Shader* self, const char* name);

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat);

void shader_uniform_1f(Shader* self, const char* name, float f);

void shader_uniform_1i(Shader* self, const char* name, int i);

void shader_free(Shader* self);

#endif