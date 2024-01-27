#ifndef BGL_MATERIAL_H
#define BGL_MATERIAL_H

#include "glmath.h"
#include "texture.h"
#include "shader.h"

typedef enum MaterialFlags {
    NO_LIGHTING          = 1 << 0,
    //HAS_DIFFUSE_TEXTURE  = 1 << 1, 
    //HAS_SPECULAR_TEXTURE = 1 << 2, 
    //HAS_NORMAL_TEXTURE   = 1 << 3,
    IS_LIGHT             = 1 << 1,
    USE_CUBEMAP_TEXTURES = 1 << 2, 
} MaterialFlags;

typedef struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    MaterialFlags flags;

    Texture* textures;
    u32 tex_count;
} Material;

// * textures take priority over colour values
// will this material be used in a shader that uses cubemap textures
// has default global textures
void material_create(Material* mat, bool is_cubemap_shader, vec3 ambient, vec3 diffuse, vec3 specular, float shininess);

// type must be of TEXTURE_DIFFUSE, TEXTURE_SPECULAR, or TEXTURE_NORMAL
// (TEXTURE_CUBEMAP and TEXTURE_DEFAULT are irrelevant flags and are ignored)
// if a texture of that type already exists the old texture will be freed and replaced
void material_add_texture(Material* mat, TextureType type, const char* texture_path);

void material_set_uniforms(Material* mat, Shader* shader);

void material_free(Material* mat);

#endif