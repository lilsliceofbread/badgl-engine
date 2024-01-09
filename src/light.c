#include "light.h"

static const char* dir_members[] = {
    "dir_light.dir",
    "dir_light.ambient",
    "dir_light.diffuse",
    "dir_light.specular"
};

Light light_create(vec3 pos, vec3 ambient, vec3 diffuse, vec3 specular, vec3 attenuation)
{
    Light light = { // need to be vec4 to align with GPU
        .pos = VEC3TOVEC4(pos, 1.0f),
        .ambient = VEC3TOVEC4(ambient, 1.0f),
        .diffuse = VEC3TOVEC4(diffuse, 1.0f),
        .specular = VEC3TOVEC4(specular, 1.0f),
        .attenuation = VEC3TOVEC4(attenuation, 1.0f)
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
    shader_uniform_vec3(shader, dir_members[0], &light->dir);
    shader_uniform_vec3(shader, dir_members[1], &light->ambient);
    shader_uniform_vec3(shader, dir_members[2], &light->diffuse);
    shader_uniform_vec3(shader, dir_members[3], &light->specular);
}