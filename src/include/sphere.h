#ifndef BADGL_SPHERE_H
#define BADGL_SPHERE_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"

// can be used to hold any type of sphere e.g. uv, icosphere
typedef struct Sphere
{
    vec3 pos;
    float radius;
    Mesh mesh;
    Texture texture; // only 1 texture, a cubemap
    Shader shader;
} Sphere;

Sphere uv_sphere_gen(vec3 pos, float radius, uint32_t resolution, const char* cubemap_path);

void sphere_draw(Sphere* self, mat4* vp);

void sphere_free(Sphere* self);

#endif