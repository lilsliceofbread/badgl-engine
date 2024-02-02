#ifndef BGL_SHAPES_H
#define BGL_SHAPES_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

// * all models centred on (0, 0, 0) or model space

/**
 * @param  res:  resolution
 * @param  shader_idx:  index to shader in rd->shaders
 */
void shapes_uv_sphere(Model* self, u32 res, const Material* material, u32 shader_idx);

/**
 * @brief  create axis-aligned box
 * @param  shader_idx:  index to shader in rd->shaders
 */
void shapes_box(Model* self, float width, float height, float depth, const Material* material, u32 shader_idx);

/* will be perpendicular to x/z axis
   width along x-axis
   height along z-axis */
/**
 * @brief  create rectangular plane perpendicular to x/z axis
 * @param  width:  distance along x-axis
 * @param  height:  distance along z-axis
 * @param  res:  resolution
 * @param  shader_idx:  index to shader in rd->shaders
 */
void shapes_plane(Model* self, float width, float height, u32 res, const Material* material, u32 shader_idx);

#endif