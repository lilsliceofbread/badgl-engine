#ifndef BGL_MODEL_H
#define BGL_MODEL_H

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "renderer.h"
#include "bgl_math.h"
#include "util.h"
#include "camera.h"
#include "material.h"
#include "transform.h"

#define MAX_PATH_LENGTH 128

typedef struct Model
{
    Mesh* meshes;
    u32 mesh_count;

    char directory[MAX_PATH_LENGTH];
    // IN ECS ENDS HERE
    // remove the following
    u32 shader_idx;
    Transform transform;
    mat4 model;

    Material material;
} Model;

/**
 * @brief  load model from file (.obj)
 * @param  scratch:  arena for doing temp work in. arena is reset back to its initial position before returning
 * @param  path:  path relative to executable
 * @param  shader_idx:  index to shader in rd->shaders
 * @returns bool denoting if model load was successful
 */
bool model_load(Model* self, Arena* scratch, const char* path, u32 shader_idx);

/**
 * @brief  update model transform and matrix
 * @param  transform: position, rotation and scale
 */
void model_update_transform(Model* self, const Transform* transform);

void model_draw(Model* self, Renderer* rd, Camera* cam);

void model_free(Model* self);

#endif
