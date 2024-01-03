#ifndef BADGL_MATERIAL_H
#define BADGL_MATERIAL_H

#include <inttypes.h>
#include "glmath.h"
#include "texture.h"

typedef enum MaterialFlags {
    HAS_DIFFUSE_TEXTURE = 1 << 0, 
    HAS_SPECULAR_TEXTURE = 1 << 1, 
    NO_LIGHTING = 1 << 2
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

#endif