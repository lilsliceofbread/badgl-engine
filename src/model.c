#include "model.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <string.h>
#include "util.h"
#include "texture.h"

void model_load(Model* self, const char* path)
{
    // import scene object (entire model)
    const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    ASSERT(scene || scene->mRootNode, "ERR: model load failed\n%s", aiGetErrorString());

    self->mesh_count = 0;
    self->tex_count = 0;
    self->meshes = (Mesh*)malloc(scene->mNumMeshes * sizeof(Mesh)); // allocate enough meshes

    {
        char temp[MAX_STR_LENGTH];
        // hacky way of getting offset from start of string
        int offset = str_find_last_of(path, '/');
        ASSERT(offset != -1, "ERR: invalid path for model %s\n", path);
        strncpy(temp, path, offset); // copy up to final / into directory
        temp[offset] = '\0'; // strncpy does not null terminate strings
        self->directory = temp;
        //printf("offset: %d dir: %s", offset, self->directory);
    }

    model_process_node(self, scene->mRootNode, scene);

    aiReleaseImport(scene); // need to free scene ourselves in c
}

void model_draw(Model* self, Shader* shader)
{
    for(uint32_t i = 0; i < self->mesh_count; i++)
    {
        mesh_draw(&(self->meshes[i]), shader, self->textures);
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
    uint32_t* tex_indexes = NULL; // indexes into model textures array
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

    //ASSERT(textures != NULL, "ERR: failed to allocate textures");

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

    if(model_mesh->mMaterialIndex >= 0)
    {
        struct aiMaterial* mat = scene->mMaterials[model_mesh->mMaterialIndex];
        uint32_t diffuse_count, specular_count;
        // inefficient af, but will fix later
        uint32_t* diff_indexes = model_load_textures(self, mat, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE, &diffuse_count);
        uint32_t* spec_indexes = model_load_textures(self, mat, aiTextureType_SPECULAR, TEXTURE_SPECULAR, &specular_count);
        total_textures = diffuse_count + specular_count;
        if(total_textures)
        {
            tex_indexes = (uint32_t*)malloc(total_textures * sizeof(uint32_t));

            size_t offset = diffuse_count * sizeof(uint32_t);
            memcpy(tex_indexes, diff_indexes, offset);
            memcpy(tex_indexes + offset, spec_indexes, specular_count * sizeof(uint32_t));

            free(diff_indexes);
            free(spec_indexes);
        } // if no textures then skip
    }

    mesh_init(&mesh, vertices, model_mesh->mNumVertices, indices, total_indices, tex_indexes, total_textures);
    return mesh;
}

uint32_t* model_load_textures(Model* self, struct aiMaterial* mat, enum aiTextureType ai_type, TextureType type, uint32_t* tex_count_out)
{
    uint32_t add_tex_count = aiGetMaterialTextureCount(mat, ai_type);
    *tex_count_out = add_tex_count;
    self->textures = (Texture*)realloc(self->textures, (self->tex_count + add_tex_count) * sizeof(Texture));
    uint32_t* tex_indexes = (uint32_t*)malloc(add_tex_count * sizeof(uint32_t)); 

    char img_path[MAX_STR_LENGTH];
    uint32_t new_add_tex_count = 0;
    for(uint32_t i = 0; i < add_tex_count; i++)
    {
        memset(img_path, 0, sizeof(img_path));
        struct aiString str;
        aiGetMaterialTexture(mat, ai_type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);
        sprintf(img_path, "%s/%s", self->directory, str.data);

        bool skip = false;
        for(uint32_t j = 0; j < self->tex_count; j++)
        {
            if(strcmp(img_path, self->textures[j].path) == 0)
            {
                tex_indexes[i] = j;
                skip = true;
                break;
            }
        }

        if(!skip)
        {
            Texture texture;
            texture_create(&texture, img_path, true);
            texture.type = type;
            self->textures[self->tex_count + i] = texture;
            tex_indexes[i] = self->tex_count + i;
            new_add_tex_count++;
        }
    }

    self->tex_count += new_add_tex_count;
    return tex_indexes;
}

void model_free(Model* self)
{
    for(uint32_t i = 0; i < self->mesh_count; i++)
    {
        mesh_free(&(self->meshes[i]));
    }

    free(self->textures);
    free(self->meshes);
}