#include "model.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <string.h>
#include "util.h"

void model_load(Model* self, const char* path)
{
    // import scene object (entire model)
    const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    ASSERT(scene || scene->mRootNode, "ERR: model load failed\n%s", aiGetErrorString());

    self->mesh_count = 0;
    self->meshes = (Mesh*)malloc(scene->mNumMeshes * sizeof(Mesh)); // allocate enough meshes

    {
        char temp[strlen(path)];
        // hacky way of getting offset from start of string
        size_t offset = (size_t)strrchr(path, '/') - (size_t)path; 
        strncpy(temp, path, offset); // copy up to final / into directory
        temp[offset] = '\0'; // strncpy does not null terminate strings
        self->directory = temp;
    }

    model_process_node(self, scene->mRootNode, scene);

    aiReleaseImport(scene); // need to free scene ourselves in c
}

void model_draw(Model* self, Shader* shader)
{
    for(uint32_t i = 0; i < self->mesh_count; i++)
    {
        mesh_draw(&(self->meshes[i]), shader);
    }
}

void model_add_mesh(Model* self, Mesh mesh, uint32_t total_meshes)
{
    ASSERT(self->mesh_count <= total_meshes, "ERR: meshes exceeded allocated memory");
    self->meshes[self->mesh_count] = mesh;
    self->mesh_count++;
}

void model_process_node(Model* self, struct aiNode* node, const struct aiScene* scene)
{
    // process each mesh in current node
    for(uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; // node meshes are indexes into scene's meshes
        model_add_mesh(self, model_process_mesh(self, mesh, scene), scene->mNumMeshes);
    }
    // recurse for child nodes
    for(uint32_t i = 0; i < node->mNumChildren; i++)
    {
        model_process_node(self, node->mChildren[i], scene);
    }
}

Mesh model_process_mesh(Model* self, struct aiMesh* model_mesh, const struct aiScene* scene)
{
    Mesh mesh;

    Vertex* vertices = NULL;
    uint32_t* indices = NULL;
    Texture* textures = NULL;
    uint32_t total_indices = 0;
    uint32_t total_textures = 0;

    // allocation (ownership goes to mesh which is freed in mesh_free())

    vertices = (Vertex*)malloc(model_mesh->mNumVertices * sizeof(Vertex));
    ASSERT(vertices != NULL, "ERR: failed to allocate vertices");

    for(uint32_t i = 0; i < model_mesh->mNumFaces; i++)
    {
        total_indices += model_mesh->mFaces[i].mNumIndices;
    }
    indices = (uint32_t*)malloc(total_indices * sizeof(uint32_t));
    ASSERT(indices != NULL, "ERR: failed to allocate indices");

    textures = (Texture*)malloc(sizeof(Texture));
    ASSERT(textures != NULL, "ERR: failed to allocate textures");

    for(uint32_t i = 0; i < model_mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vec3 vec;
        vec[0] = model_mesh->mVertices[i].x;
        vec[1] = model_mesh->mVertices[i].y;
        vec[2] = model_mesh->mVertices[i].z;
        glm_vec3_copy(vec, vertex.pos);

        vec[0] = model_mesh->mNormals[i].x;
        vec[1] = model_mesh->mNormals[i].y;
        vec[2] = model_mesh->mNormals[i].z;
        glm_vec3_copy(vec, vertex.normal);

        if(model_mesh->mTextureCoords[0])
        {
            vec2 uv_vec;
            uv_vec[0] = model_mesh->mTextureCoords[0][i].x;
            uv_vec[1] = model_mesh->mTextureCoords[0][i].y;
            glm_vec2_copy(uv_vec, vertex.uv);
        }
        else
        {
            glm_vec2_copy((vec2){0.0f, 0.0f}, vertex.uv);
        }

        vertices[i] = vertex;
    }

    uint32_t ind_count = 0;
    for(uint32_t i = 0; i < model_mesh->mNumFaces; i++)
    {
        struct aiFace face = model_mesh->mFaces[i];
        for(uint32_t j = 0; j < face.mNumIndices; j++)
        {
            indices[ind_count] = face.mIndices[j];
            ind_count++;
        }
    }

    /*
    if(model_mesh->mMaterialIndex >= 0)
    {
        struct aiMaterial* mat = scene->mMaterials[model_mesh->mMaterialIndex];

    }
    */

    mesh_init(&mesh, vertices, model_mesh->mNumVertices, indices, total_indices, textures, total_textures);
    return mesh;
}

Texture* model_load_textures(Model* self, struct aiMaterial* mat, enum aiTextureType ai_type, TextureType type)
{

}

void model_free(Model* self)
{
    for(uint32_t i = 0; i < self->mesh_count; i++)
    {
        mesh_free(&(self->meshes[i]));
    }

    free(self->meshes);
}