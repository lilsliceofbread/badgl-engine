#ifndef BGL_SHADER_H
#define BGL_SHADER_H

#include "bgl_math.h"
#include "defines.h"
#include "arena.h"

/* internal definitions */
#define MAX_UNIFORM_NAME 128 
#define MAX_SHADER_FILEPATH 128
typedef struct Uniform {
    char name[MAX_UNIFORM_NAME];
    i32 location;
} Uniform;

typedef struct Shader
{
    u32 id;
    Uniform* uniforms;
    u32 uniform_count;
    #ifdef BGL_EDITOR
    char sources[3][MAX_SHADER_FILEPATH];
    char name[MAX_SHADER_FILEPATH];
    #endif
} Shader;

bool shader_create(Shader* self, Arena* scratch, const char* const* shader_filepaths, u32 shader_count, const char* version_str, bool no_uniform_bindings);

i32 shader_find_uniform(Shader* self, const char* name);

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat);
void shader_uniform_vec4(Shader* self, const char* name, vec4* vec);
void shader_uniform_vec3(Shader* self, const char* name, vec3* vec);
void shader_uniform_vec2(Shader* self, const char* name, vec2* vec);
void shader_uniform_f32(Shader* self, const char* name, f32 f);
void shader_uniform_int(Shader* self, const char* name, i32 i);

/* set binding of uniform block - only required for opengl versions < 4.2 */
void shader_ubo_set_binding(Shader* self, const char* uniform_block, u32 binding);

void shader_free(Shader* self);

/**
 * internal function - use rd_use_shader
 */
void shader_use(Shader* self);

#endif
