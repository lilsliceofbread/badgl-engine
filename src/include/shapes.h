#ifndef BADGL_SPHERE_H
#define BADGL_SPHERE_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

// all shapes are centered on (0, 0, 0) or model space

void uv_sphere_gen(Model* self, float radius, uint32_t res, const Material* material, uint32_t shader_idx);

void rectangular_prism_gen(Model* self, float width, float height, float depth, const Material* material, uint32_t shader_idx);

/* will be perpendicular to x/z axis
   width along x-axis
   height along z-axis */
void rectangular_plane_gen(Model* self, float width, float height, uint32_t res, const Material* material, uint32_t shader_idx);

#endif