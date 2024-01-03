#ifndef BADGL_SHADER_H
#define BADGL_SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"
#include <inttypes.h>

#define MAX_UNIF_NAME 64
#define MAX_UNIFORMS 16
/* either takes arguments
   Shader* self, const char* name, void* arg
   Shader* self, const char* name (in this case pass NULL for void* arg) */
typedef int(*FindUniformFunc)(); 

typedef struct Uniform {
    char name[MAX_UNIF_NAME]; // if you make a uniform name longer than 64 characters i don't like you
    GLint location;
} Uniform;

// shader program
typedef struct Shader
{
    GLuint id;
    Uniform stored_uniforms[MAX_UNIFORMS];
    uint32_t uniform_count; // found during shader comp
} Shader;

void shader_init(Shader* self, const char* vert_shader_src, const char* frag_shader_src);

uint32_t shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, int log_size, int* success_out);

void shader_find_uniforms_in_source(Shader* self, const char* src_code); // UNIFORMS ARE REPEATED IF IN VERT + FRAG SHADER

void shader_use(Shader* self);

int shader_find_uniform(Shader* self, const char* name, FindUniformFunc func, void* arg);

// arg should be a uint32_t that is the index within the array
int shader_find_uniform_array(Shader* self, const char* name, void* arg);

// arg should be a const char* that is the member of the struct
int shader_find_uniform_struct(Shader* self, const char* name, void* arg);

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat, FindUniformFunc func, void* arg);
void shader_uniform_vec4(Shader* self, const char* name, vec4* vec, FindUniformFunc func, void* arg);
void shader_uniform_vec3(Shader* self, const char* name, vec3* vec, FindUniformFunc func, void* arg);
void shader_uniform_1f(Shader* self, const char* name, float f, FindUniformFunc func, void* arg);
void shader_uniform_1i(Shader* self, const char* name, int i, FindUniformFunc func, void* arg);

void shader_free(Shader* self);

#endif