#include "light.h"

static const char* dir_members[] = {
    "dir",
    "ambient",
    "diffuse",
    "specular"
};

Light light_create(vec3 pos, vec3 ambient, vec3 diffuse, vec3 specular, vec3 attenuation)
{
    Light light = { // need to be vec4 to align with GPU
        .pos = {pos.x, pos.y, pos.z, 1.0f},
        .ambient = {ambient.x, ambient.y, ambient.z, 1.0f},
        .diffuse = {diffuse.x, diffuse.y, diffuse.z, 1.0f},
        .specular = {specular.x, specular.y, specular.z, 1.0f},
        .attenuation = {attenuation.x, attenuation.y, attenuation.z, 1.0f}
    };

    return light;
}

DirLight dir_light_create(vec3 dir, vec3 ambient, vec3 diffuse, vec3 specular)
{
    DirLight light = {
        .dir = dir,
        .ambient = ambient,
        .diffuse = diffuse,
        .specular = specular
    };

    return light;
}

void dir_light_set_uniforms(DirLight* light, Shader* shader)
{
    shader_uniform_vec3(shader, "dir_light", &light->dir,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)dir_members[0]);
    shader_uniform_vec3(shader, "dir_light", &light->ambient,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)dir_members[1]);
    shader_uniform_vec3(shader, "dir_light", &light->diffuse,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)dir_members[2]);
    shader_uniform_vec3(shader, "dir_light", &light->specular,
                        (FindUniformFunc)shader_find_uniform_struct, (void*)dir_members[3]);
}