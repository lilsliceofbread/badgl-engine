#include "light.h"

static const char* dir_members[] = {
    "dir_light.dir",
    "dir_light.ambient",
    "dir_light.diffuse",
    "dir_light.specular"
};

void light_create(Light* light, vec3 pos, vec3 ambient, vec3 diffuse, vec3 specular, vec3 attenuation)
{
    light->pos = VEC3TOVEC4(pos, 1.0f);
    light->ambient = VEC3TOVEC4(ambient, 1.0f);
    light->diffuse = VEC3TOVEC4(diffuse, 1.0f);
    light->specular = VEC3TOVEC4(specular, 1.0f);
    light->attenuation = VEC3TOVEC4(attenuation, 1.0f);
}

void dir_light_create(DirLight* light, vec3 dir, vec3 ambient, vec3 diffuse, vec3 specular)
{
    light->dir = dir;
    light->ambient = ambient;
    light->diffuse = diffuse;
    light->specular = specular;
}

void dir_light_set_uniforms(DirLight* light, Shader* shader)
{
    shader_uniform_vec3(shader, dir_members[0], &light->dir);
    shader_uniform_vec3(shader, dir_members[1], &light->ambient);
    shader_uniform_vec3(shader, dir_members[2], &light->diffuse);
    shader_uniform_vec3(shader, dir_members[3], &light->specular);
}