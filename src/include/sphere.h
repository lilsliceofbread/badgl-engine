#ifndef BADGL_SPHERE_H
#define BADGL_SPHERE_H

#include "glmath.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "model.h"

// sphere just uses Model struct

void uv_sphere_gen(Model* self, float radius, uint32_t resolution, const char* cubemap_path, uint32_t shader_index);

#endif