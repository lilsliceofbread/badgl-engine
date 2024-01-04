#include "material.h"

#include <stdlib.h>

void material_textureless(Material* mat, bool is_cubemap_shader,
                          vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
{
    mat->ambient = ambient;
    mat->diffuse = diffuse;
    mat->specular = specular;
    mat->shininess = shininess;

    mat->flags = 0;

    mat->textures = (Texture*)malloc(2 * sizeof(Texture)); // diffuse and specular
    mat->tex_count = 2;
    mat->textures[0].type = TEXTURE_DIFFUSE;
    mat->textures[1].type = TEXTURE_SPECULAR;

    if(!is_cubemap_shader) // create default white texture
    {

        texture_default_create(&mat->textures[0]);
        texture_default_create(&mat->textures[1]);
    }
    else
    {
        texture_default_cubemap_create(&mat->textures[0]);
        texture_default_cubemap_create(&mat->textures[1]);
    }
}

// TODO: allow multiple textures / specular textures
void material_texture_diffuse(Material* mat, bool is_cubemap, const char* texture_path,
                              vec3 specular, float shininess)
{
    mat->ambient = (vec3){1.0f, 1.0f, 1.0f};
    mat->diffuse = (vec3){1.0f, 1.0f, 1.0f};
    mat->specular = specular;
    mat->shininess = shininess;

    mat->flags = 0;

    mat->textures = (Texture*)malloc(2 * sizeof(Texture));
    mat->tex_count = 2;
    mat->textures[0].type = TEXTURE_DIFFUSE;
    mat->textures[1].type = TEXTURE_SPECULAR;

    if(!is_cubemap)
    {
        texture_create(&mat->textures[0], texture_path, false);
        texture_default_create(&mat->textures[1]);
    }
    else
    {
        texture_cubemap_create(&mat->textures[0], texture_path);
        texture_default_cubemap_create(&mat->textures[1]);
    }
}
