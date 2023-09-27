#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
    ASSERT(image_data, "ERR: could not load image %s\n", img_path);

    GLenum format;
    switch(num_channels)
    {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }

    // GL_SRGB8_ALPHA8?
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);

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
    strcpy(self->path, img_path);
}

void texture_bind(Texture self)
{
    glBindTexture(GL_TEXTURE_2D, self.id);
}

void texture_unit_active(uint8_t num)
{
    ASSERT(num <= 31, "ERR: GL texture unit out of range");
    glActiveTexture(GL_TEXTURE0 + num);
}