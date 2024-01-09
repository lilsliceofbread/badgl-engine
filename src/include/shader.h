#ifndef BADGL_SHADER_H
#define BADGL_SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"
#include <inttypes.h>

#define MAX_UNIFORM_NAME 64
#define UNIFORM_ALLOC_SIZE 8 

/* either takes arguments
   Shader* self, const char* name, void* arg
   Shader* self, const char* name (in this case pass NULL for void* arg) */

typedef struct Uniform {
    char name[MAX_UNIFORM_NAME]; // if you make a uniform name longer than 64 characters i don't like you
    int location;
} Uniform;

typedef struct Shader
{
    uint32_t id;
    Uniform* uniforms;
    uint32_t uniform_count; // found during shader comp
} Shader;

void shader_init(Shader* self, const char* vert_shader_src, const char* frag_shader_src);

uint32_t shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, int log_size, int* success_out);

void shader_find_uniforms_in_source(Shader* self, const char* src_code); // UNIFORMS ARE REPEATED IF IN VERT + FRAG SHADER

// if new_count does not fit in uniforms array reallocate
void shader_reallocate_uniforms(Shader* self, uint32_t new_count);

void shader_use(Shader* self);

int shader_find_uniform(Shader* self, const char* name);

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat);
void shader_uniform_vec4(Shader* self, const char* name, vec4* vec);
void shader_uniform_vec3(Shader* self, const char* name, vec3* vec);
void shader_uniform_1f(Shader* self, const char* name, float f);
void shader_uniform_1i(Shader* self, const char* name, int i);

void shader_free(Shader* self);

#endif