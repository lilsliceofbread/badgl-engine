#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// 'forward declare' bool
#undef bool
#define bool _Bool

typedef enum TextureType
{
    TEXTURE_DIFFUSE, // colour texture
    TEXTURE_SPECULAR
} TextureType;

typedef struct Texture
{
    GLuint id;
    TextureType type;
    int width, height;
} Texture;

void texture_create(Texture* self, const char* img_filename, bool use_mipmap, bool use_alpha);

void texture_bind(Texture* self);

void texture_unit_active(uint8_t unit);

#endif