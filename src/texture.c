#include "texture.h"

// prevent warnings for stb_image
#ifndef _MSC_VER // doesn't work for MSVC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsign-conversion"
    #pragma GCC diagnostic ignored "-Wconversion"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef _MSC_VER // doesn't work for MSVC
    #pragma GCC diagnostic pop
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "util.h"
#include "renderer.h"

void texture_create(Texture* self, TextureType type, const char* img_path, bool use_mipmap)
{
    PERF_TIMER_START();

    GLuint texture_id;
    glGenTextures(1, &texture_id);

    int width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    uint8_t* img_data = stbi_load(img_path, &width, &height, &num_channels, 4);
    ASSERT(img_data, "TEXTURE: could not load image %s\n", img_path);

    glBindTexture(GL_TEXTURE_2D, texture_id);
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


    self->id = texture_id;
    self->width = width;
    self->height = height;
    self->type = type;

    strncpy(self->path, img_path, 128);

    PERF_TIMER_END("TEXTURE: loading texture");
}

void texture_default_create(Texture* self, TextureType type)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    uint8_t img_data[] = {255, 255, 255, 255}; // 1 white pixel

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    self->id = texture_id;
    self->width = 1;
    self->height = 1;
    self->type = type;

    memset(self->path, 0, MAX_PATH_LENGTH);
}

void texture_default_cubemap_create(Texture* self, TextureType type)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // since it can be sampled in 3 dimensions

    uint8_t img_data[] = {255, 255, 255, 255}; // 1 white pixel
    for(int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
    }

    self->id = texture_id;
    self->width = 1; 
    self->height = 1;
    self->type = type;
    self->type |= TEXTURE_CUBEMAP;

    memset(self->path, 0, MAX_PATH_LENGTH);
}

void texture_cubemap_create(Texture* self, TextureType type, const char* generic_path)
{
    PERF_TIMER_START();

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    const char* suffixes[6] = {
        "_px",
        "_nx",
        "_py",
        "_ny",
        "_pz",
        "_nz"
    };

    uint32_t path_idx = 0;
    char curr;
    char path_no_ext[128];

    while((curr = generic_path[path_idx]) != '.' && generic_path[path_idx] != '\0' && path_idx < 128)
    {
        path_no_ext[path_idx] = curr;
        path_idx++;
    }
    path_no_ext[path_idx] = '\0'; // must manually null terminate
    char* extension = (char*)(generic_path + path_idx);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // since it can be sampled in 3 dimensions


    char img_path[256];
    int width, height, num_channels;
    for(int i = 0; i < 6; i++)
    {
        memset(img_path, 0, sizeof(img_path));
        strncpy(img_path, path_no_ext, 256);
        strcat(img_path, suffixes[i]); // don't need to use strncat here, known sizes
        strncat(img_path, extension, 32); // no extension is longer than this... right?

        stbi_set_flip_vertically_on_load(false);
        uint8_t* img_data = stbi_load(img_path, &width, &height, &num_channels, 4);
        ASSERT(img_data, "TEXTURE: could not load image %s\n", img_path);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

        stbi_image_free(img_data);
    }

    self->id = texture_id;
    self->width = width; // will be the size of 1 square
    self->height = height;
    self->type = type;
    self->type |= TEXTURE_CUBEMAP;

    strncpy(self->path, generic_path, 128);

    PERF_TIMER_END("TEXTURE: loading cubemap");
}

void texture_bind(Texture* self)
{
    GLenum target = self->type & TEXTURE_CUBEMAP ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D; 
    glBindTexture(target, self->id);
}

void texture_unit_active(uint32_t num)
{
    ASSERT(num <= 31, "TEXTURE: GL texture unit out of range\n");
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
    ASSERT(false, "TEXTURE: invalid texture type %d\n", (int)type);
    return NULL;
}