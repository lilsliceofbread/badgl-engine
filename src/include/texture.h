#ifndef BGL_TEXTURE_H
#define BGL_TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "defines.h"

#define MAX_PATH_LENGTH 128

typedef enum TextureType
{
    TEXTURE_DIFFUSE  = 1 << 0,
    TEXTURE_SPECULAR = 1 << 1,
    TEXTURE_NORMAL   = 1 << 2,
    TEXTURE_CUBEMAP  = 1 << 3,
    TEXTURE_DEFAULT  = 1 << 4  // * dummy white 1x1 texture
} TextureType;

typedef struct Texture
{
    GLuint id;
    TextureType type;
    i32 width, height;
    char path[MAX_PATH_LENGTH];
} Texture;

/**
 * @brief setup texture context
 * @note this function must be called before using any texture functions
 * @retval None
 */
void textures_init(void);

// type must be set by user
void texture_create(Texture* self, TextureType type, const char* img_path, bool use_mipmap);

// copy the global 1x1 texture (don't create a new texture on GPU)
// TODO: allow black for normal maps
void texture_global_default_create(Texture* self, TextureType type, bool is_cubemap);

// create 1x1 texture
void texture_default_create(Texture* self, u8 brightness, TextureType type);

void texture_default_cubemap_create(Texture* self, u8 brightness, TextureType type);

/**
 * expects faces to have suffixes _px, _nx, etc for all sides
 * should all have the same extension and name
 * example file: res/img_px.png, res/img_nx.png - pass in ("res/img.png")
 */
void texture_cubemap_create(Texture* self, TextureType type, const char* path);

void texture_bind(Texture* self);

void texture_unit_active(u32 unit);

void texture_free(Texture* self);

const char* texture_type_get_str(TextureType type);

#endif