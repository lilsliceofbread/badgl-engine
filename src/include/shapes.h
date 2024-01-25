#ifndef BGL_SHAPES_H
#define BGL_SHAPES_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

// all shapes are centered on (0, 0, 0) or model space

void shapes_uv_sphere(Model* self, u32 res, const Material* material, u32 shader_idx);

void shapes_rectangular_prism(Model* self, float width, float height, float depth, const Material* material, u32 shader_idx);

/* will be perpendicular to x/z axis
   width along x-axis
   height along z-axis */
void shapes_rectangular_plane(Model* self, float width, float height, u32 res, const Material* material, u32 shader_idx);

#endif