#include "material.h"

#include <stdlib.h>

static const char* material_members[] = {
    "material.ambient",
    "material.diffuse",
    "material.specular",
    "material.shininess"
};

Material material_textureless(bool is_cubemap_shader, vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
{
    Material mat;

    mat.ambient = ambient;
    mat.diffuse = diffuse;
    mat.specular = specular;
    mat.shininess = shininess;

    mat.flags = 0;

    mat.textures = (Texture*)malloc(2 * sizeof(Texture)); // diffuse and specular
    mat.tex_count = 2;

    if(!is_cubemap_shader) // create default white texture
    {

        texture_default_create(&mat.textures[0], TEXTURE_DIFFUSE);
        texture_default_create(&mat.textures[1], TEXTURE_SPECULAR);
    }
    else
    {
        texture_default_cubemap_create(&mat.textures[0], TEXTURE_DIFFUSE);
        texture_default_cubemap_create(&mat.textures[1], TEXTURE_SPECULAR);
    }

    return mat;
}

// TODO: allow multiple textures / specular textures
Material material_texture_diffuse(bool is_cubemap, const char* texture_path, vec3 specular, float shininess)
{
    Material mat;

    mat.ambient = VEC3(1.0f, 1.0f, 1.0f);
    mat.diffuse = VEC3(1.0f, 1.0f, 1.0f);
    mat.specular = specular;
    mat.shininess = shininess;

    mat.flags = 0;

    mat.textures = (Texture*)malloc(2 * sizeof(Texture));
    mat.tex_count = 2;

    if(!is_cubemap)
    {
        texture_create(&mat.textures[0], TEXTURE_DIFFUSE, texture_path, false);
        texture_default_create(&mat.textures[1], TEXTURE_SPECULAR);
    }
    else
    {
        texture_cubemap_create(&mat.textures[0], TEXTURE_DIFFUSE, texture_path);
        texture_default_cubemap_create(&mat.textures[1], TEXTURE_SPECULAR);
    }

    return mat;
}

void material_set_uniforms(Material* mat, Shader* shader)
{
    shader_uniform_vec3(shader, material_members[0], &mat->ambient);
    shader_uniform_vec3(shader, material_members[1], &mat->diffuse);
    shader_uniform_vec3(shader, material_members[2], &mat->specular);
    shader_uniform_1f(shader, material_members[3], mat->shininess);
}