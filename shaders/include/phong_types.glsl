#ifndef BGL_PHONG_TYPES_GLSL
#define BGL_PHONG_TYPES_GLSL

/* if included in glsl, not c */
#ifdef GL_core_profile

/* allow cubemap shaders to have vec3 texture coords */
#ifndef tex_coord_t
#define tex_coord_t vec2
#endif

struct VSOut {
    vec3 frag_pos;
    vec3 world_pos;
    vec3 normal;
    tex_coord_t tex_coord;
};

/* material struct in engine is different so don't mix them up when including */
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
