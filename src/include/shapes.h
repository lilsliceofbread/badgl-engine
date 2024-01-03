#ifndef BADGL_SPHERE_H
#define BADGL_SPHERE_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

// all shapes are centered on (0, 0, 0) or model space

Model uv_sphere_gen(float radius, uint32_t res,
                    const char* cubemap_path, Material* material, uint32_t shader_index);

Model rectangular_prism_gen(float width, float height, float depth,
                            const char* cubemap_path, Material* material, uint32_t shader_index);

/* will be perpendicular to x/z axis
   width along x-axis
   height along z-axis */
Model rectangular_plane_gen(float width, float height, uint32_t res,
                            const char* texture_path, Material* material, uint32_t shader_index);

#endif