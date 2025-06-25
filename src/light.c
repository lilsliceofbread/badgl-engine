#include "light.h"

static const char* dir_members[] = { // TODO: fix somehow?
    "dir_light.dir",
    "dir_light.ambient",
    "dir_light.diffuse",
    "dir_light.specular",
};

void dir_light_set_uniforms(DirLight* light, Shader* shader)
{
    shader_uniform_vec3(shader, dir_members[0], &light->dir);
    shader_uniform_vec3(shader, dir_members[1], &light->ambient);
    shader_uniform_vec3(shader, dir_members[2], &light->diffuse);
    shader_uniform_vec3(shader, dir_members[3], &light->specular);
}
