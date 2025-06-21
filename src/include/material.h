#ifndef BGL_MATERIAL_H
#define BGL_MATERIAL_H

#include "glmath.h"
#include "texture.h"
#include "shader.h"

typedef enum MaterialFlags {
    BGL_MATERIAL_NO_LIGHTING          = 1 << 0,
    BGL_MATERIAL_IS_LIGHT             = 1 << 1,
    BGL_MATERIAL_USE_CUBEMAP_TEXTURES = 1 << 2, 
} MaterialFlags;

typedef struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    f32 shininess;

    MaterialFlags flags;

    Texture* textures;
    u32 tex_count;
} Material;

/**
 * @brief  creates material with colour materials and global default white 1x1 textures
 * @note   if you are using textures then the respective colours will be ignored
 * @param  is_cubemap_shader: if the object for this material uses cubemaps put true
 */
void material_create(Material* mat, bool is_cubemap_shader, vec3 ambient, vec3 diffuse, vec3 specular, f32 shininess);

/**
 * @brief  add texture to material
 * @note   if a texture of type already exists, and is not the default texture, it will be freed and replaced with the new texture
 * @note   textures always take priority over colour values i.e. if a material has a specular texture, the specular colour will be set to white and ignored
 * @param  mat: an already created material
 * @param  type: type must be of TEXTURE_DIFFUSE, TEXTURE_SPECULAR, or TEXTURE_NORMAL (TEXTURE_CUBEMAP and TEXTURE_DEFAULT are irrelevant flags and are ignored)
 * @param  texture_path: path to texture (if using cubemap, see texture_cubemap_create for more details)
 */
void material_add_texture(Material* mat, TextureType type, const char* texture_path);

void material_free(Material* mat);

/**
 * engine/internal functions
 */
void material_set_uniforms(Material* mat, Shader* shader);

#endif
