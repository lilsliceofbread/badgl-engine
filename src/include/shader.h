#ifndef BGL_SHADER_H
#define BGL_SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"
#include "defines.h"

#define MAX_UNIFORM_NAME 128 
#define SHADER_UNIFORM_ALLOC_SIZE 8 

typedef struct Uniform {
    char name[MAX_UNIFORM_NAME];
    i32 location;
} Uniform;

typedef struct Shader
{
    u32 id;
    Uniform* uniforms;
    u32 uniform_count;
} Shader;

void shader_create(Shader* self, const char* vert_shader_src, const char* frag_shader_src);

void shader_use(Shader* self);

i32 shader_find_uniform(Shader* self, const char* name);

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat);
void shader_uniform_vec4(Shader* self, const char* name, vec4* vec);
void shader_uniform_vec3(Shader* self, const char* name, vec3* vec);
void shader_uniform_vec2(Shader* self, const char* name, vec2* vec);
void shader_uniform_float(Shader* self, const char* name, float f);
void shader_uniform_int(Shader* self, const char* name, i32 i);

void shader_free(Shader* self);

#endif