#ifndef BADGL_SPHERE_H
#define BADGL_SPHERE_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"

// can be used to hold any type of sphere e.g. uv, icosphere
typedef struct Sphere
{
    vec3 pos;
    float radius;
    Mesh mesh;
    Texture texture; // only 1 texture, a cubemap
    uint32_t shader_index; // index into renderer's shaders
} Sphere;

Sphere uv_sphere_gen(vec3 pos, float radius, uint32_t resolution, const char* cubemap_path, uint32_t shader_index);

void sphere_draw(Sphere* self, Renderer* rd, mat4* vp);

void sphere_free(Sphere* self);

#endif