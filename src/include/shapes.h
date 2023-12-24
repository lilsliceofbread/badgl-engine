#ifndef BADGL_SPHERE_H
#define BADGL_SPHERE_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

// sphere just uses Model struct

Model uv_sphere_gen(float radius, uint32_t resolution, const char* cubemap_path, uint32_t shader_index);

Model rectangular_prism_gen(float width, float height, float depth, const char* cubemap_path, uint32_t shader_index);

#endif