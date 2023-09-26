#ifndef MODEL_H
#define MODEL_H

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include "mesh.h"
#include "texture.h"
#include "shader.h"

typedef struct Model
{
    Mesh* meshes;
    uint32_t mesh_count;
    const char* directory;
    Texture* loaded_textures;
} Model;

void model_load(Model* self, const char* path);

void model_draw(Model* self, Shader* shader);

void model_add_mesh(Model* self, Mesh mesh, uint32_t total_meshes);

void model_process_node(Model* self, struct aiNode* node, const struct aiScene* scene);

Mesh model_process_mesh(Model* self, struct aiMesh* mesh, const struct aiScene* scene);

Texture* model_load_textures(Model* self, struct aiMaterial* mat, enum aiTextureType ai_type, TextureType type);

void model_free(Model* self);

#endif