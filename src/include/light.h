#ifndef BGL_LIGHT_H
#define BGL_LIGHT_H

#include "glmath.h"
#include "shader.h"

typedef struct Light {
    vec4 pos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
} Light;

typedef struct DirLight {
    vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} DirLight;

/**
 * @param  attenuation: attenuation constants: (x, y, z) -> (quadratic, linear, constant)
 */
static inline void light_create(Light* light, vec3 pos, vec3 ambient, vec3 diffuse, vec3 specular, vec3 attenuation)
{
    light->pos = VEC3TOVEC4(pos, 1.0f);
    light->ambient = VEC3TOVEC4(ambient, 1.0f);
    light->diffuse = VEC3TOVEC4(diffuse, 1.0f);
    light->specular = VEC3TOVEC4(specular, 1.0f);
    light->attenuation = VEC3TOVEC4(attenuation, 1.0f);
}

/**
 * @param  dir: direction light is coming from
 */
static inline void dir_light_create(DirLight* light, vec3 dir, vec3 ambient, vec3 diffuse, vec3 specular)
{
    light->dir = dir;
    light->ambient = ambient;
    light->diffuse = diffuse;
    light->specular = specular;
}

/**
 * engine/internal functions
 */
void dir_light_set_uniforms(DirLight* light, Shader* shader);

#endif
