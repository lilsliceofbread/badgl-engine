#ifndef BADGL_MODEL_H
#define BADGL_MODEL_H

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


typedef struct Model
{
    Mesh* meshes;
    uint32_t mesh_count;
    const char* directory;
    Texture* textures;
    uint32_t tex_count;
    uint32_t shader_index; // index into renderer's shader array

    Transform transform;
    mat4 model;
} Model;

void model_load(Model* self, const char* path, uint32_t shader_index);

void model_update_transform(Model* self, Transform* transform);

void model_draw(Model* self, Renderer* rd, mat4* vp);

void model_add_mesh(Model* self, Mesh mesh, uint32_t total_meshes);

void model_process_node(Model* self, struct aiNode* node, const struct aiScene* scene);

Mesh model_process_mesh(Model* self, struct aiMesh* mesh, const struct aiScene* scene);

uint32_t* model_load_textures(Model* self, struct aiMaterial* mat, enum aiTextureType ai_type, TextureType type, uint32_t* tex_count_out);

void model_free(Model* self);

#endif