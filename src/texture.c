#include "texture.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>

int texture_create(Texture2D* texture_ptr, const char* img_filename, bool use_mipmap, bool use_alpha)
{
    GLuint texture;
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
    int forced_channels = 3 + (use_alpha * 1); // if using alpha will be 4
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image_data = stbi_load(img_filename, &width, &height, &num_channels, forced_channels);
    if(!image_data)
    {
        fprintf(stderr, "ERR: could not load image");
        return false;
    }

    if(use_alpha)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
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

    texture_ptr->id = texture;
    texture_ptr->width = width;
    texture_ptr->height = height;

    return true;
}