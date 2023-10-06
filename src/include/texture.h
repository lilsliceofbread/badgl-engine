#ifndef BADGL_TEXTURE_H
#define BADGL_TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// 'forward declare' bool
#undef bool
#define bool _Bool

#define MAX_STR_LENGTH 128

typedef enum TextureType
{
    TEXTURE_DIFFUSE, // colour texture
    TEXTURE_SPECULAR,
    TEXTURE_NORMAL, // normal map
    TEXTURE_CUBEMAP
} TextureType;

typedef struct Texture
{
    GLuint id;
    TextureType type;
    int width, height;
    char path[MAX_STR_LENGTH];
} Texture;

void texture_create(Texture* self, const char* img_path, bool use_mipmap);

/* expects faces to have suffixes _px, _nx, etc for all sides
 * should all have the same extension and name
 * example file: res/img_px.png, res/img_nx.png - pass in ("res/img.png")
 */
void texture_cubemap_create(Texture* self, const char* generic_path);

void texture_bind(Texture* self);

void texture_unit_active(uint32_t unit);

void texture_free(Texture* self);

const char* texture_type_get_str(TextureType type);

#endif