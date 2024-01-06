#include "material.h"

#include <stdlib.h>

static const char* material_members[4] = {
    "ambient",
    "diffuse",
    "specular",
    "shininess"
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
    mat.textures[0].type = TEXTURE_DIFFUSE;
    mat.textures[1].type = TEXTURE_SPECULAR;

    if(!is_cubemap_shader) // create default white texture
    {

        texture_default_create(&mat.textures[0]);
        texture_default_create(&mat.textures[1]);
    }
    else
    {
        texture_default_cubemap_create(&mat.textures[0]);
        texture_default_cubemap_create(&mat.textures[1]);
    }

    return mat;
}

// TODO: allow multiple textures / specular textures
Material material_texture_diffuse(bool is_cubemap, const char* texture_path, vec3 specular, float shininess)
{
    Material mat;

    mat.ambient = (vec3){1.0f, 1.0f, 1.0f};
    mat.diffuse = (vec3){1.0f, 1.0f, 1.0f};
    mat.specular = specular;
    mat.shininess = shininess;

    mat.flags = 0;

    mat.textures = (Texture*)malloc(2 * sizeof(Texture));
    mat.tex_count = 2;
    mat.textures[0].type = TEXTURE_DIFFUSE;
    mat.textures[1].type = TEXTURE_SPECULAR;

    if(!is_cubemap)
    {
        texture_create(&mat.textures[0], texture_path, false);
        texture_default_create(&mat.textures[1]);
    }
    else
    {
        texture_cubemap_create(&mat.textures[0], texture_path);
        texture_default_cubemap_create(&mat.textures[1]);
    }

    return mat;
}

void material_set_uniforms(Material* mat, Shader* shader)
{
    shader_uniform_vec3(shader, "material", &mat->ambient,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)material_members[0]);
    shader_uniform_vec3(shader, "material", &mat->diffuse,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)material_members[1]);
    shader_uniform_vec3(shader, "material", &mat->specular,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)material_members[2]);
    shader_uniform_1f(shader, "material", mat->shininess,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)material_members[3]);
}