#include "light.h"

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