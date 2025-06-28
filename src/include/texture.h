#ifndef BGL_TEXTURE_H
#define BGL_TEXTURE_H

#include <glad/glad.h>
#include "defines.h"

#define MAX_PATH_LENGTH 128

typedef enum TextureType
{
    BGL_TEXTURE_PHONG_DEFAULT  = 1 << 0, // * dummy white 1x1 texture
    BGL_TEXTURE_PHONG_DIFFUSE  = 1 << 1,
    BGL_TEXTURE_PHONG_SPECULAR = 1 << 2,
    BGL_TEXTURE_PHONG_NORMAL   = 1 << 3,
    BGL_TEXTURE_PHONG_CUBEMAP  = 1 << 4,
} TextureType;

typedef struct Texture
{
    u32 id;
    TextureType type; // TODO: remove for PBR?
    i32 width, height;
    // TODO: u32 mipmaps;
    char path[MAX_PATH_LENGTH];
} Texture;

void texture_create(Texture* self, TextureType type, const char* path, bool use_mipmap);

/**
 * @brief  copies the global 1x1 texture (don't create a new texture on GPU)
 * @param  is_cubemap: is the texture a cubemap
 */
void texture_global_default_create(Texture* self, TextureType type, bool is_cubemap);

/**
 * @brief  create 1x1 texture of greyscale colour
 * @param  brightness: brightness of greyscale colour
 * @param  type: type of texture
 */
void texture_default_create(Texture* self, u8 brightness, TextureType type);

/**
 * @brief  create 1x1 cubemap of greyscale colour
 * @param  brightness: brightness of greyscale colour
 * @param  type: type of texture
 */
void texture_default_cubemap_create(Texture* self, u8 brightness, TextureType type);

/**
 * @note accepts both single and multi image cubemaps
 * 
 * @note for single image cubemaps:
 * @note the image must be in an aspect ratio of 4:3 with each side of the cubemap being a square
 * @note the image should look like this:
 * @note -- py -- --
 * @note nx pz px nz
 * @note -- ny -- -- 
 * @note see example/res for a real example
 * 
 * @note for multi image cubemaps:
 * @note expects faces to have suffixes _px, _nx, etc for all sides
 * @note should all have the same extension and name
 * @note example file: res/img_px.png, res/img_nx.png - pass in "res/img.png"
 */
void texture_cubemap_create(Texture* self, TextureType type, const char* path);

void texture_free(Texture* self);

/**
 * engine/internal functions
 */
void textures_init(void);

void texture_bind(Texture* self);

void texture_unit_active(u32 unit);

const char* texture_type_get_str(TextureType type);

#endif
