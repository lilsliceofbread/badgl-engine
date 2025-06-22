#include "material.h"

#include "defines.h"
#include <stdlib.h>
#include <string.h>

static const char* material_members[] = { // ? mildly scuffed
    "material.ambient",
    "material.diffuse",
    "material.specular",
    "material.shininess",
};

void material_create(Material* mat, bool is_cubemap_shader, vec3 ambient, vec3 diffuse, vec3 specular, f32 shininess)
{
    mat->ambient = ambient;
    mat->diffuse = diffuse;
    mat->specular = specular;
    mat->shininess = shininess;

    mat->flags = is_cubemap_shader ? BGL_MATERIAL_USE_CUBEMAP_TEXTURES : 0;

    mat->textures = (Texture*)BGL_MALLOC(2 * sizeof(Texture)); // diffuse and specular
    mat->tex_count = 2;

    texture_global_default_create(&mat->textures[0], TEXTURE_DIFFUSE, is_cubemap_shader);
    texture_global_default_create(&mat->textures[1], TEXTURE_SPECULAR, is_cubemap_shader);
}

void material_add_texture(Material* mat, TextureType type, const char* texture_path)
{
    if(mat->textures == NULL || mat->tex_count < 2)
    {
        BGL_LOG_ERROR("textures have not been allocated");
        return;
    }

    type &= (TextureType)~(TEXTURE_CUBEMAP | TEXTURE_DEFAULT);

    Texture* tex_to_replace = NULL;
    for(u32 i = 0; i < mat->tex_count; i++)
    {
        if((mat->textures[i].type & (TextureType)~(TEXTURE_CUBEMAP | TEXTURE_DEFAULT)) != type) continue;

        if(!(mat->textures[i].type & TEXTURE_DEFAULT))
        {
            texture_free(&mat->textures[i]);
        }

        tex_to_replace = &mat->textures[i];
        memset(tex_to_replace, 0, sizeof(Texture)); // just in case to prevent old values being used
        break;
    }

    if(tex_to_replace == NULL) // not already created e.g. normal textures
    {
        mat->textures = (Texture*)BGL_REALLOC(mat->textures, (mat->tex_count + 1) * sizeof(Texture));
        tex_to_replace = &mat->textures[mat->tex_count++];
    }

    if(mat->flags & BGL_MATERIAL_USE_CUBEMAP_TEXTURES)
    {
        texture_cubemap_create(tex_to_replace, type, texture_path);
    }
    else
    {
        texture_create(tex_to_replace, type, texture_path, false);
    }

    if(type & TEXTURE_DIFFUSE)
    {
        mat->ambient = VEC3(1.0f, 1.0f, 1.0f);
        mat->diffuse = VEC3(1.0f, 1.0f, 1.0f);
    }
    else if(type & TEXTURE_SPECULAR)
    {
        mat->specular = VEC3(1.0f, 1.0f, 1.0f);
    }
}

void material_set_uniforms(Material* mat, Shader* shader)
{
    if(mat->flags & BGL_MATERIAL_NO_LIGHTING) return;

    shader_uniform_vec3(shader, material_members[0], &mat->ambient);
    shader_uniform_vec3(shader, material_members[1], &mat->diffuse);
    shader_uniform_vec3(shader, material_members[2], &mat->specular);
    shader_uniform_f32(shader, material_members[3], mat->shininess);
}

void material_free(Material* mat)
{
    if(mat->textures == NULL) 
    {
        BGL_LOG_INFO("no texture data on free");
        return;
    }

    for(u32 i = 0; i < mat->tex_count; i++)
    {
        texture_free(&mat->textures[i]);
    }
    BGL_FREE(mat->textures);
}
