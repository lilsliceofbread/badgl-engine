#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define MAX_UNIFORMS 10

typedef struct uniform_pair {
    const char* name;
    GLuint location;
} uniform_pair;

// shader program
typedef struct Shader
{
    GLuint id;
    uniform_pair stored_uniforms[MAX_UNIFORMS]; // should really be sized during shader compilation and freed
    int uniform_count; // found during shader comp
} Shader;

int shader_compile(GLuint* shader_id, const char* shader_filepath, GLenum shader_type, char* info_log, int log_size);

void shader_init(Shader* self, const char* vert_shader_src, const char* frag_shader_src);

void shader_use(Shader* self);

GLuint shader_find_uniform(Shader* self, const char* name);

void shader_uniform_mat4(Shader* self, const char* name, mat4 mat);

void shader_uniform_1f(Shader* self, const char* name, float f);

void shader_uniform_1i(Shader* self, const char* name, int i);

void shader_free(Shader* self);

#endif