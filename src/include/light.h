#ifndef BADGL_LIGHT_H
#define BADGL_LIGHT_H

#include "glmath.h"

typedef struct Light {
    vec4 pos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation; // attenuation constants: (x, y, z) -> (quadratic, linear, constant)
} Light;

Light light_create(vec3 pos, vec3 ambient, vec3 diffuse, vec3 specular, vec3 attenuation);

#endif
