#ifndef BADGL_MATERIAL_H
#define BADGL_MATERIAL_H

#include <inttypes.h>
#include "glmath.h"
#include "texture.h"

typedef enum MaterialFlags {
    NO_LIGHTING          = 1 << 0,
    HAS_DIFFUSE_TEXTURE  = 1 << 1, 
    HAS_SPECULAR_TEXTURE = 1 << 2, 
    HAS_NORMAL_TEXTURE   = 1 << 3
} MaterialFlags;

typedef struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    MaterialFlags flags;

    Texture* textures;
    uint32_t tex_count;
} Material;

//void material_add_texture(Material* mat, const char* texture_path, TextureType type);

void material_textureless(Material* mat, bool is_cubemap_shader,
                          vec3 ambient, vec3 diffuse, vec3 specular, float shininess);

void material_texture_diffuse(Material* mat, bool is_cubemap, const char* texture_path,
                              vec3 specular, float shininess);



#endif