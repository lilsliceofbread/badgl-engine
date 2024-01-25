#include "material.h"

#include <stdlib.h>
#include "defines.h"

static const char* material_members[] = { // ? mildly scuffed
    "material.ambient",
    "material.diffuse",
    "material.specular",
    "material.shininess"
};

void material_textureless(Material* mat, bool is_cubemap_shader, vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
{
    mat->ambient = ambient;
    mat->diffuse = diffuse;
    mat->specular = specular;
    mat->shininess = shininess;

    mat->flags = 0;

    mat->textures = (Texture*)malloc(2 * sizeof(Texture)); // diffuse and specular
    mat->tex_count = 2;

    texture_global_default_create(&mat->textures[0], TEXTURE_DIFFUSE, is_cubemap_shader);
    texture_global_default_create(&mat->textures[1], TEXTURE_SPECULAR, is_cubemap_shader);
}

// TODO: allow multiple textures / specular textures
void material_texture_diffuse(Material* mat, bool is_cubemap, const char* texture_path, vec3 specular, float shininess)
{
    mat->ambient = VEC3(1.0f, 1.0f, 1.0f);
    mat->diffuse = VEC3(1.0f, 1.0f, 1.0f);
    mat->specular = specular;
    mat->shininess = shininess;

    mat->flags = HAS_DIFFUSE_TEXTURE;

    mat->textures = (Texture*)malloc(2 * sizeof(Texture));
    mat->tex_count = 2;

    if(!is_cubemap)
    {
        texture_create(&mat->textures[0], TEXTURE_DIFFUSE, texture_path, false);
    }
    else
    {
        texture_cubemap_create(&mat->textures[0], TEXTURE_DIFFUSE, texture_path);
    }

    texture_global_default_create(&mat->textures[1], TEXTURE_SPECULAR, is_cubemap);
}

void material_set_uniforms(Material* mat, Shader* shader)
{
    shader_uniform_vec3(shader, material_members[0], &mat->ambient);
    shader_uniform_vec3(shader, material_members[1], &mat->diffuse);
    shader_uniform_vec3(shader, material_members[2], &mat->specular);
    shader_uniform_float(shader, material_members[3], mat->shininess);
}