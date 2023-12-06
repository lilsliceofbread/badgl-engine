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

void texture_create(Texture* self, const char* img_path, bool use_mipmap)
{
    double start_time = glfwGetTime();

    GLuint texture_id;
    glGenTextures(1, &texture_id);

    int width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image_data = stbi_load(img_path, &width, &height, &num_channels, 0);
    ASSERT(image_data, "TEXTURE: could not load image %s\n", img_path);

    GLint internal_format; // need to do this because they use separate values
    GLenum img_format;
    switch(num_channels)
    {
        case 1:
            img_format = GL_RED;
            internal_format = GL_RED;
            break;
        case 3:
            img_format = GL_RGB;
            internal_format = GL_RGB;
            break;
        case 4:
            img_format = GL_RGBA;
            internal_format = GL_SRGB8_ALPHA8;
            break;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, img_format, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);

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

    strncpy(self->path, img_path, 128);

    printf("TEXTURE: loading texture %s took %fs\n", img_path, glfwGetTime() - start_time);
}

void texture_cubemap_create(Texture* self, const char* generic_path)
{
    double start_time = glfwGetTime();

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    self->id = texture_id;
    self->type = TEXTURE_CUBEMAP;
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

    // 100 is max length sanity check in case no null terminator
    // (should really go from end of str to start and then strncpy as it would find ext far faster)
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
        //snprintf(img_path, 100, "%s%s%s", path_no_ext, suffixes[i], extension);
        strncpy(img_path, path_no_ext, 256);
        strcat(img_path, suffixes[i]); // don't need to use strncat here, known sizes
        strncat(img_path, extension, 32); // no extension is longer than this... right?

        stbi_set_flip_vertically_on_load(false);
        unsigned char* image_data = stbi_load(img_path, &width, &height, &num_channels, 0);
        ASSERT(image_data, "TEXTURE: could not load image %s\n", img_path);

        GLenum img_format;
        switch(num_channels)
        {
            case 1:
                img_format = GL_RED;
                break;
            case 3:
                img_format = GL_RGB;
                break;
            case 4:
                img_format = GL_RGBA;
                break;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, GL_RGB, width, height, 0, img_format, GL_UNSIGNED_BYTE, image_data);

        stbi_image_free(image_data);
    }

    self->width = width; // will be the size of 1 square
    self->height = height;

    strncpy(self->path, generic_path, 128);

    printf("TEXTURE: loading cubemap %s took %fs\n", generic_path, glfwGetTime() - start_time);
}

void texture_bind(Texture* self)
{
    GLenum target = self->type == TEXTURE_CUBEMAP ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D; 
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
    switch(type)
    {
        case TEXTURE_DIFFUSE:
            return "texture_diffuse";
            break; // unsure if necessary
        case TEXTURE_SPECULAR:
            return "texture_specular";
            break;
        case TEXTURE_NORMAL:
            return "texture_normal";
            break;
        case TEXTURE_CUBEMAP:
            return "cubemap";
            break;
    }
    ASSERT(false, "TEXTURE: invalid texture type\n");
}