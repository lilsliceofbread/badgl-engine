#ifndef BGL_LIGHT_H
#define BGL_LIGHT_H

#include "bgl_math.h"
#include "shader.h"

/* define structs only once in glsl and then include */
#include "phong_types.glsl"
typedef struct Light Light;
typedef struct DirLight DirLight;

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

