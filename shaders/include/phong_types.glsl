#ifndef BGL_PHONG_TYPES_GLSL
#define BGL_PHONG_TYPES_GLSL

/* material struct in engine is different so don't mix them up when including */
#ifdef GL_core_profile
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
#endif

struct Light {
    vec4 pos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation; // attenuation constants - quadratic, linear, constant
};

struct DirLight {
    vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#endif
