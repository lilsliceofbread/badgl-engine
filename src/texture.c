#include "texture.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdbool.h>

void texture2d_create(int* texture_ptr, const char* img_filename, bool use_mipmap, bool use_alpha)
{
    int texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    // could add param to change wrap method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if(use_mipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    int width, height, num_channels;
    int forced_channels = 4 * use_alpha; // if using alpha will be 4, else 0
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image_data = stbi_load(img_filename, &width, &height, &num_channels, forced_channels);
    ASSERT(image_data, "ERR: could not load image");

    if(use_alpha)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    }

    if(use_mipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(image_data);

    *texture_ptr = texture;
}