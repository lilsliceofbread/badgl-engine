#ifndef BADGL_TEXTURE_H
#define BADGL_TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// 'forward declare' bool
#undef bool
#define bool _Bool

#define MAX_STR_LENGTH 100

typedef enum TextureType
{
    TEXTURE_DIFFUSE, // colour texture
    TEXTURE_SPECULAR,
    TEXTURE_NORMAL // normal map
} TextureType;

typedef struct Texture
{
    GLuint id;
    TextureType type;
    int width, height;
    char path[MAX_STR_LENGTH];
} Texture;

void texture_create(Texture* self, const char* img_filename, bool use_mipmap);

void texture_bind(Texture self);

void texture_unit_active(uint8_t unit);

const char* texture_type_get_str(TextureType type);

#endif