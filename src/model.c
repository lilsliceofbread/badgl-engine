#include "model.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include <string.h>
#include "defines.h"
#include "arena.h"
#include "bgl_math.h"
#include "platform.h"
#include "texture.h"
#include "renderer.h"
#include "light.h"

/**
 * internal functions
 */
bool model_add_mesh(Model* self, Mesh* mesh, u32 total_meshes);
bool model_process_node(Model* self, Arena* arena, struct aiNode* node, const struct aiScene* scene);
bool model_process_mesh(Model* self, Arena* arena, struct aiMesh* mesh, const struct aiScene* scene, Mesh* mesh_out);
bool model_load_textures(Model* self, struct aiMaterial* mat, TextureType type, u32** tex_indices_out, u32* tex_count_out);

bool model_load(Model* self, Arena* scratch, const char* path, u32 shader_idx)
{
    BGL_PERFORMANCE_START();

    self->material.ambient = VEC3(1.0f, 1.0f, 1.0f);
    self->material.specular = VEC3(1.0f, 1.0f, 1.0f);
    self->material.diffuse = VEC3(1.0f, 1.0f, 1.0f);
    self->material.shininess = 32.0f;
    self->material.textures = NULL; // undefined behaviour if not set to NULL before realloc (uninit memory)
    self->material.tex_count = 0;
    self->material.flags = 0;
    self->shader_idx = shader_idx;
    transform_reset(&self->transform);
    mat4_identity(&self->model);

    char full_path[1024];
    platform_prepend_executable_directory(full_path, 1024, path);
    find_directory_from_path(self->directory, MAX_PATH_LENGTH, full_path);

    const struct aiScene* scene = aiImportFile(full_path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if(scene == NULL || scene->mRootNode == NULL || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        BGL_LOG_ERROR("loading model %s failed. assimp error:\n%s", full_path, aiGetErrorString());
        return false;
    }

    self->mesh_count = 0;
    self->meshes = (Mesh*)BGL_MALLOC(scene->mNumMeshes * sizeof(Mesh)); // allocate enough meshes

    u8* arena_init_pos = arena_alloc(scratch, 0);

    bool success = model_process_node(self, scratch, scene->mRootNode, scene);

    arena_collapse(scratch, arena_init_pos);

    aiReleaseImport(scene);

    if(!success)
    {
        BGL_LOG_ERROR("loading model %s failed\n");
        return false;
    }

    for(u32 i = 0; i < self->material.tex_count; i++)
    {
        Texture tex = self->material.textures[i];

        // TODO: make use material functions instead
        if(tex.type & BGL_TEXTURE_PHONG_DIFFUSE)
        {
            self->material.ambient = VEC3(1.0f, 1.0f, 1.0f);
            self->material.diffuse = VEC3(1.0f, 1.0f, 1.0f);
        }
        else if(tex.type & BGL_TEXTURE_PHONG_SPECULAR)
        {
            self->material.specular = VEC3(1.0f, 1.0f, 1.0f);
        }
    }

    BGL_PERFORMANCE_END("loading model");
    return true;
}

void model_update_transform(Model* self, const Transform* transform)
{
    self->transform = *transform;
    mat4_identity(&self->model);

    mat4_scale(&self->model, self->transform.scale);

    mat4_rotate_x(&self->model, RADIANS(self->transform.euler.x)); // pitch
    mat4_rotate_y(&self->model, RADIANS(self->transform.euler.y)); // yaw
    mat4_rotate_z(&self->model, RADIANS(self->transform.euler.z)); // roll

    mat4_trans(&self->model, self->transform.pos);
}

void model_draw(Model* self, Renderer* rd, Camera* cam)
{
    Shader* shader = &rd->shaders[self->shader_idx]; // TODO: move draw funcs into rendersystem to fix this

    // TODO: calculate normal matrix here instead of in shader
    mat4 mvp, model_view;
    mat4_mul(&model_view, cam->view, self->model);
    mat4_mul(&mvp, cam->projection, model_view);

    rd_use_shader(rd, self->shader_idx);
    
    material_set_uniforms(&self->material, shader);

    shader_uniform_mat4(shader, "mvp", &mvp);
    if(!(self->material.flags & (BGL_MATERIAL_NO_LIGHTING | BGL_MATERIAL_IS_LIGHT)))
    {
        shader_uniform_mat4(shader, "model_view", &model_view);
        shader_uniform_mat4(shader, "model", &self->model);
        shader_uniform_mat4(shader, "view", &cam->view);
    }

    for(u32 i = 0; i < self->mesh_count; i++)
    {
        mesh_draw(&self->meshes[i], shader, self->material.textures);
    }
}

bool model_add_mesh(Model* self, Mesh* mesh, u32 total_meshes)
{
    if(self->mesh_count >= total_meshes)
    {
        BGL_LOG_ERROR("meshes exceeded expected count");
        return false;
    }

    self->meshes[self->mesh_count] = *mesh;
    self->mesh_count++;
    return true;
}

bool model_process_node(Model* self, Arena* arena, struct aiNode* node, const struct aiScene* scene)
{
    Mesh mesh;
    for(u32 i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]]; // node meshes are indices into scene's meshes
        if(!model_process_mesh(self, arena, ai_mesh, scene, &mesh)) return false;
        if(!model_add_mesh(self, &mesh, scene->mNumMeshes)) return false;
    }

    for(u32 i = 0; i < node->mNumChildren; i++)
    {
        if(!model_process_node(self, arena, node->mChildren[i], scene)) return false;
    }

    return true;
}

bool model_process_mesh(Model* self, Arena* arena, struct aiMesh* model_mesh, const struct aiScene* scene, Mesh* mesh_out)
{
    Mesh mesh;

    u32* indices = NULL;
    u32* tex_indices = NULL; // indices into model's textures array
    const u32 total_vertices = model_mesh->mNumVertices;
    u32 total_indices = 0;
    u32 total_textures = 0;

    // loop through faces to count indices for allocation of indices array
    for(u32 i = 0; i < model_mesh->mNumFaces; i++)
    {
        struct aiFace face = model_mesh->mFaces[i];

        // prevent lines or single vertices from being added
        if(face.mNumIndices < 3) continue;
        total_indices += face.mNumIndices;
    }

    ////const u32 total_mem_size = ((u32)total_vertices * 8 * sizeof(f32)) + ((u32)total_indices * sizeof(u32));

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(arena, total_vertices * sizeof(vec3)),
        .normal = (vec3*)arena_alloc(arena, total_vertices * sizeof(vec3)),
        .uv = (vec2*)arena_alloc(arena, total_vertices * sizeof(vec2))
    };
    memset(vertex_buffer.uv, 0, total_vertices * sizeof(vec2)); // if no tex coords, then all values zeroed out

    indices = (u32*)arena_alloc(arena, total_indices * sizeof(u32));

    for(u32 i = 0; i < total_vertices; i++)
    {
        vec3 pos, normal;
        vec2 uv;

        pos.x = model_mesh->mVertices[i].x;
        pos.y = model_mesh->mVertices[i].y;
        pos.z = model_mesh->mVertices[i].z;

        normal.x = model_mesh->mNormals[i].x;
        normal.y = model_mesh->mNormals[i].y;
        normal.z = model_mesh->mNormals[i].z;

        if(model_mesh->mTextureCoords[0])
        {
            uv.u = model_mesh->mTextureCoords[0][i].x;
            uv.v = model_mesh->mTextureCoords[0][i].y;
        }
        else
        {
            uv = VEC2(0.0f, 0.0f);
        }

        vertex_buffer.pos[i] = pos;
        vertex_buffer.normal[i] = normal;
        vertex_buffer.uv[i] = uv;
    }

    // loop through faces and indices and add each to array
    u32 ind_count = 0;
    for(u32 i = 0; i < model_mesh->mNumFaces; i++)
    {
        struct aiFace face = model_mesh->mFaces[i];

        if(face.mNumIndices < 3) continue; // prevent non triangular faces being added
        for(u32 j = 0; j < face.mNumIndices; j++)
        {
            indices[ind_count] = face.mIndices[j];
            ind_count++;
        }
    }

    // aiMesh's material index indexes into scene's pool of materials
    struct aiMaterial* mat = scene->mMaterials[model_mesh->mMaterialIndex];
    u32 diffuse_count, specular_count;

    u32* diff_indices;
    u32* spec_indices;
    if(!model_load_textures(self, mat, BGL_TEXTURE_PHONG_DIFFUSE, &diff_indices, &diffuse_count)) return false;
    if(!model_load_textures(self, mat, BGL_TEXTURE_PHONG_SPECULAR, &spec_indices, &specular_count)) return false;

    total_textures = diffuse_count + specular_count;
    if(total_textures)
    {
        tex_indices = (u32*)BGL_CALLOC(total_textures, sizeof(u32));

        memcpy(tex_indices, diff_indices, diffuse_count * sizeof(u32));
        memcpy(tex_indices + diffuse_count, spec_indices, specular_count * sizeof(u32));

        BGL_FREE(diff_indices);
        BGL_FREE(spec_indices);
    }

    mesh_create(&mesh, vertex_buffer, total_vertices, indices, total_indices, tex_indices, total_textures);
    *mesh_out = mesh;
    return true;
}

bool model_load_textures(Model* self, struct aiMaterial* mat, TextureType type, u32** tex_indices_out, u32* tex_count_out)
{
    enum aiTextureType ai_type = type == BGL_TEXTURE_PHONG_DIFFUSE ? aiTextureType_DIFFUSE : aiTextureType_SPECULAR;
    u32 tex_count = aiGetMaterialTextureCount(mat, ai_type); // textures of given type
    *tex_indices_out = NULL;
    *tex_count_out = tex_count;
    if(tex_count == 0) return true;

    u32* tex_indices = (u32*)BGL_CALLOC(tex_count, sizeof(u32)); // user of function must free themselves

    char img_path[1024 + MAX_PATH_LENGTH]; // suppress warnings for snprintf (dir + '/' + str.data)
    u32 add_tex_count = 0; // since we only resize if texture doesn't already exist
    for(u32 i = 0; i < tex_count; i++)
    {
        memset(img_path, 0, sizeof(img_path)); // ensure previous string doesn't cause problems
        struct aiString str;
        aiGetMaterialTexture(mat, ai_type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL); // get material texture string
        snprintf(img_path, sizeof(img_path), "%s/%s", self->directory, str.data); // append texture string to directory

        for(u32 j = 0; j < self->material.tex_count; j++) // loop through model's textures
        {
            if(strcmp(img_path, self->material.textures[j].path) == 0) // if texture already exists
            {
                tex_indices[i] = j;
                goto next_texture;
            }
        }

        // increase amount to add to tex_count and reallocate textures
        add_tex_count++;
        self->material.textures = (Texture*)BGL_REALLOC(self->material.textures, (self->material.tex_count + add_tex_count) * sizeof(Texture));
        if(self->material.textures == NULL)
        {
            BGL_LOG_ERROR("failed to realloc texture array");
            return false;
        }

        // create new texture
        Texture texture;
        texture_create(&texture, type, img_path, true);

        // set next texture in array to current texture
        self->material.textures[self->material.tex_count + i] = texture; // first iter is tex_count + 0
        tex_indices[i] = self->material.tex_count + i; // store index into textures array for mesh to access

        next_texture: ;
    }

    // increase tex_count to correct size
    self->material.tex_count += add_tex_count;
    *tex_indices_out = tex_indices;
    return true;
}

void model_free(Model* self)
{
    for(u32 i = 0; i < self->mesh_count; i++)
    {
        mesh_free(&self->meshes[i]);
    }

    BGL_FREE(self->meshes);

    material_free(&self->material);
}
