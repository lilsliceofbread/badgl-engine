#ifndef BADGL_LIGHT_H
#define BADGL_LIGHT_H

#include "glmath.h"

typedef struct Light {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation; // attenuation constants: (x, y, z) -> (quadratic, linear, constant)
} Light;

#endif
