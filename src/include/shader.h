#ifndef BGL_SHADER_H
#define BGL_SHADER_H

#include "glmath.h"
#include "defines.h"

/* internal definitions */
#define MAX_UNIFORM_NAME 128 
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

/**
 * @brief  creates a shader program. pass in your shaders in any order, as long as there is all the required shaders
 * @note   the shaders can either be all contained in one file separated by "#type vertex|fragment|geometry| or each in a separate file
 * @note   #includes should be specified relative to the specific file
 * @returns bool denoting if the shader program was created successfully
 */
bool shader_create(Shader* self, const char** shader_filepaths, u32 shader_count, const char* version_str);

void shader_use(Shader* self);

i32 shader_find_uniform(Shader* self, const char* name);

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat);
void shader_uniform_vec4(Shader* self, const char* name, vec4* vec);
void shader_uniform_vec3(Shader* self, const char* name, vec3* vec);
void shader_uniform_vec2(Shader* self, const char* name, vec2* vec);
void shader_uniform_f32(Shader* self, const char* name, f32 f);
void shader_uniform_int(Shader* self, const char* name, i32 i);

void shader_free(Shader* self);

#endif
