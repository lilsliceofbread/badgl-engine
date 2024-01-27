#ifndef BGL_MODEL_H
#define BGL_MODEL_H

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "renderer.h"
#include "glmath.h"
#include "util.h"
#include "camera.h"
#include "material.h"

#define MAX_PATH_LENGTH 128

typedef struct Model
{
    Mesh* meshes;
    u32 mesh_count;
    u32 shader_idx; // index into renderer's shader array

    char directory[MAX_PATH_LENGTH];

    Transform transform;
    mat4 model;

    Material material;
} Model;

void model_load(Model* self, const char* path, u32 shader_idx);

void model_update_transform(Model* self, const Transform* transform);

void model_draw(Model* self, Renderer* rd, Camera* cam);

void model_free(Model* self);

#endif