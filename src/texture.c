#include "texture.h"

// prevent warnings for stb_image
#ifndef _MSC_VER
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsign-conversion"
    #pragma GCC diagnostic ignored "-Wconversion"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef _MSC_VER
    #pragma GCC diagnostic pop
#endif

#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "util.h"
#include "renderer.h"

/**
 * these variables are only set once by textures_init so it's probably fine
 * to them as static global; gets more complicated include this in some state like the Renderer
 */
static struct
{
    i32 max_texture_units;
    Texture texture_default;
    Texture cubemap_default;
} texture_ctx;

/**
 * internal functions
 */
void texture_single_image_cubemap_create(Texture* self, const char* texture_path);
void texture_multi_image_cubemap_create(Texture* self, const char* generic_path);

void textures_init(void) {
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_ctx.max_texture_units);
    BGL_LOG_NO_CTX(LOG_INFO, "max texture units: %u\n", texture_ctx.max_texture_units);

    texture_default_create(&texture_ctx.texture_default, 255, 0);
    texture_default_cubemap_create(&texture_ctx.cubemap_default, 255, 0);
}

void texture_global_default_create(Texture* self, TextureType type, bool is_cubemap)
{
    Texture* global = is_cubemap ? &texture_ctx.cubemap_default : &texture_ctx.texture_default;

    memcpy(self, global, sizeof(Texture));

    self->type |= type;
}

void texture_create(Texture* self, TextureType type, const char* img_path, bool use_mipmap)
{
    PERF_TIMER_START();

    glGenTextures(1, &self->id);

    i32 width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    u8* img_data = stbi_load(img_path, &width, &height, &num_channels, 4);
    BGL_ASSERT(img_data, "could not load image %s\n", img_path);

    glBindTexture(GL_TEXTURE_2D, self->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

    stbi_image_free(img_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(use_mipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    self->width = width;
    self->height = height;
    self->type = type;

    strncpy(self->path, img_path, 128);

    PERF_TIMER_END("loading texture");
}

void texture_default_create(Texture* self, u8 brightness, TextureType type)
{
    glGenTextures(1, &self->id);

    u8 img_data[] = {brightness, brightness, brightness, 255}; // 1 white pixel

    glBindTexture(GL_TEXTURE_2D, self->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    self->width = 1;
    self->height = 1;
    self->type = type | TEXTURE_DEFAULT;

    memset(self->path, 0, MAX_PATH_LENGTH);
    BGL_LOG(LOG_INFO, "created 1x1 default texture of id: %u\n", self->id);
}

void texture_default_cubemap_create(Texture* self, u8 brightness, TextureType type)
{
    glGenTextures(1, &self->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // since it can be sampled in 3 dimensions

    u8 img_data[] = {brightness, brightness, brightness, 255}; // 1 white pixel
    for(int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_RGB,
                     1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
    }

    self->width = 1; 
    self->height = 1;
    self->type = type | TEXTURE_DEFAULT | TEXTURE_CUBEMAP;

    memset(self->path, 0, MAX_PATH_LENGTH);
    BGL_LOG(LOG_INFO, "created 1x1 default cubemap of id: %u\n", self->id);
}

void texture_cubemap_create(Texture* self, TextureType type, const char* path)
{
    PERF_TIMER_START();

    glGenTextures(1, &self->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // since it can be sampled in 3 dimensions

    if(platform_file_exists(path))
    {
        texture_single_image_cubemap_create(self, path);
    }
    else
    {
        texture_multi_image_cubemap_create(self, path); // if path is incorrect, will error in this function
    }

    self->type = type | TEXTURE_CUBEMAP;

    strncpy(self->path, path, 128);

    PERF_TIMER_END("loading cubemap texture");
}

void texture_bind(Texture* self)
{
    GLenum target = self->type & TEXTURE_CUBEMAP ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D; 
    glBindTexture(target, self->id);
}

void texture_unit_active(u32 num)
{
    BGL_ASSERT((i32)num <= texture_ctx.max_texture_units - 1, " GL texture unit out of range\n");
    glActiveTexture(GL_TEXTURE0 + num);
}

void texture_free(Texture* self)
{
    glDeleteTextures(1, &self->id);
}

const char* texture_type_get_str(TextureType type)
{
    if(type & TEXTURE_DIFFUSE) return "texture_diffuse";
    if(type & TEXTURE_SPECULAR) return "texture_specular";
    if(type & TEXTURE_NORMAL) return "texture_normal";
    BGL_ASSERT(false, " invalid texture type %d\n", (i32)type);
    return NULL;
}

void texture_single_image_cubemap_create(Texture* self, const char* texture_path)
{
    i32 width, height, num_channels;
    stbi_set_flip_vertically_on_load(false); // don't need to flip, not using uv coordinate space
    u8* img_data = stbi_load(texture_path, &width, &height, &num_channels, 4);

    BGL_ASSERT(img_data, "could not load image %s\n", texture_path);
    BGL_ASSERT((width * 3) == (height * 4), "image %s aspect ratio is incorrect\n", texture_path);

    i32 grid_size = width / 4;

    i32 sub_image_starts[] = { // x and y
        2 * grid_size, grid_size, // px
        0,             grid_size, // nx
        grid_size,     0,         // py 
        grid_size, 2 * grid_size, // ny 
        grid_size,     grid_size, // pz 
        3 * grid_size, grid_size, // nz 
    };

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width); // ! won't work on OpenGL ES
    for(int i = 0; i < 6; i++)
    {
        i32 idx = 2 * i;
        i32 sub_x = sub_image_starts[idx];
        i32 sub_y = sub_image_starts[idx + 1];
        u8* sub_image = img_data + ((sub_y * width) + sub_x) * 4;

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_RGB,
                     grid_size, grid_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, sub_image);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    stbi_image_free(img_data);

    self->width = width;
    self->height = height;
}

void texture_multi_image_cubemap_create(Texture* self, const char* generic_path)
{
    const char* suffixes[6] = {
        "_px",
        "_nx",
        "_py",
        "_ny",
        "_pz",
        "_nz",
    };

    u32 path_idx = 0;
    char curr;
    char path_no_ext[128];

    while((curr = generic_path[path_idx]) != '.' && generic_path[path_idx] != '\0' && path_idx < 128)
    {
        path_no_ext[path_idx] = curr;
        path_idx++;
    }
    path_no_ext[path_idx] = '\0'; // must manually null terminate
    char* extension = (char*)(generic_path + path_idx);

    char img_path[256];
    i32 width, height, num_channels;
    for(int i = 0; i < 6; i++)
    {
        memset(img_path, 0, sizeof(img_path));
        strncpy(img_path, path_no_ext, 256);
        strncat(img_path, suffixes[i], 3);
        strncat(img_path, extension, 32); // 32 should be fine...

        stbi_set_flip_vertically_on_load(false); // don't need to flip, not using uv coordinate space
        u8* img_data = stbi_load(img_path, &width, &height, &num_channels, 4);
        BGL_ASSERT(img_data, "could not load image %s\n", img_path);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_RGB,
                     width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

        stbi_image_free(img_data);
    }
    
    self->width = width;
    self->height = height;
}