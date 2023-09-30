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
#include <stdbool.h>
#include "util.h"

void texture_create(Texture* self, const char* img_path, bool use_mipmap)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    int width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image_data = stbi_load(img_path, &width, &height, &num_channels, 0);
    ASSERT(image_data, "RENDERER: could not load image %s\n", img_path);

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
    // super scuffed but works (must create new array since img_path will change)
    // idk why img_path has a lifetime/ is created in the same location everytime but whatever
    strncpy(self->path, img_path, 100);
}

void texture_bind(Texture self)
{
    glBindTexture(GL_TEXTURE_2D, self.id);
}

void texture_unit_active(uint32_t num)
{
    ASSERT(num <= 31, "RENDERER: GL texture unit out of range");
    glActiveTexture(GL_TEXTURE0 + num);
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
    }
    ASSERT(false, "TEXTURE: invalid texture type");
}