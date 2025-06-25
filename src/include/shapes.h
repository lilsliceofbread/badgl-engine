#ifndef BGL_SHAPES_H
#define BGL_SHAPES_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

/* all models centred on (0, 0, 0) or model space */

/**
 * @brief  create uv sphere
 * @param  scratch:  arena for doing temp work in. arena is reset back to its initial position before returning
 * @param  res:  resolution
 * @param  shader_idx:  index to shader in rd->shaders
 */
void shapes_uv_sphere(Model* self, Arena* scratch, u32 res, const Material* material, u32 shader_idx);

/**
 * @brief  create axis-aligned box
 * @param  scratch:  arena for doing temp work in. arena is reset back to its initial position before returning
 * @param  shader_idx:  index to shader in rd->shaders
 */
void shapes_box(Model* self, Arena* scratch, f32 width, f32 height, f32 depth, const Material* material, u32 shader_idx);

/* will be parallel to x/z axis plane
   width along x-axis
   height along z-axis */
/**
 * @brief  create rectangular plane perpendicular to y axis
 * @param  scratch:  arena for doing temp work in. arena is reset back to its initial position before returning
 * @param  width:  distance along x-axis
 * @param  height:  distance along z-axis
 * @param  res:  resolution
 * @param  shader_idx:  index to shader in rd->shaders
 */
void shapes_plane(Model* self, Arena* scratch, f32 width, f32 height, u32 res, const Material* material, u32 shader_idx);

#endif
