#include "model.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <string.h>
#include <stdbool.h>
#include "glmath.h"
#include "util.h"
#include "texture.h"
#include "renderer.h"
#include "light.h"

Model model_load(const char* path, const Material* material, uint32_t shader_idx)
{
    PERF_TIMER_START();

    Model self;

    if(material != NULL) self.material = *material; // use aiMat to set?
    self.material.textures = NULL; // undefined behaviour if not set to NULL before realloc (uninit memory)
    self.material.tex_count = 0;
    self.material.flags = 0;
    self.shader_idx = shader_idx;
    transform_reset(&self.transform);
    mat4_identity(&self.model);
    find_directory_from_path(self.directory, path);

    const struct aiScene* scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    ASSERT(scene && scene->mRootNode && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "MODEL: loading %s failed\n%s\n", path, aiGetErrorString());

    self.mesh_count = 0;
    self.meshes = (Mesh*)malloc(scene->mNumMeshes * sizeof(Mesh)); // allocate enough meshes

    model_process_node(&self, scene->mRootNode, scene);

    aiReleaseImport(scene);



    for(uint32_t i = 0; i < self.material.tex_count; i++)
    {
        Texture curr_tex = self.material.textures[i];

        if(curr_tex.type & TEXTURE_DIFFUSE)
        {
            self.material.flags |= HAS_DIFFUSE_TEXTURE;

            self.material.ambient = VEC3(1.0f, 1.0f, 1.0f);
            self.material.diffuse = VEC3(1.0f, 1.0f, 1.0f);
        }
        else if(curr_tex.type & TEXTURE_SPECULAR)
        {
            self.material.flags |= HAS_SPECULAR_TEXTURE;

            self.material.specular = VEC3(1.0f, 1.0f, 1.0f);
        }
        else if(curr_tex.type & TEXTURE_NORMAL)
        {
            self.material.flags |= HAS_NORMAL_TEXTURE;
        }
    }

    PERF_TIMER_END("MODEL: loading model");

    return self;
}

void model_update_transform(Model* self, Transform* transform)
{
    self->transform = *transform;
    mat4_identity(&self->model);

    mat4_rotate_x(&self->model, self->transform.euler.x); // pitch
    mat4_rotate_y(&self->model, self->transform.euler.y); // yaw
    mat4_rotate_z(&self->model, self->transform.euler.z); // roll

    mat4_scale(&self->model, self->transform.scale);

    mat4_trans(&self->model, self->transform.pos);
}

void model_draw(Model* self, Renderer* rd, Camera* cam)
{
    Shader* shader = rd_get_shader(rd, self->shader_idx);

    mat4 mvp, model_view;
    mat4_mul(&model_view, cam->view, self->model);
    mat4_mul(&mvp, cam->proj, model_view);

    shader_use(shader);
    shader_uniform_mat4(shader, "mvp", &mvp);
    
    MaterialFlags flags = self->material.flags;
    if(!(flags & NO_LIGHTING))
    {
        material_set_uniforms(&self->material, shader);
    }
    if(!(flags & (NO_LIGHTING | IS_LIGHT)))
    {
        shader_uniform_mat4(shader, "model_view", &model_view);
        shader_uniform_mat4(shader, "model", &self->model);
        shader_uniform_mat4(shader, "view", &cam->view);
    }

    for(uint32_t i = 0; i < self->mesh_count; i++)
    {
        mesh_draw(&self->meshes[i], shader, self->material.textures);
    }
}

void model_add_mesh(Model* self, Mesh mesh, uint32_t total_meshes)
{
    ASSERT(self->mesh_count <= total_meshes, "MODEL: meshes exceeded expected count\n");
    self->meshes[self->mesh_count] = mesh;
    self->mesh_count++;
}

void model_process_node(Model* self, struct aiNode* node, const struct aiScene* scene)
{
    for(uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; // node meshes are indexes into scene's meshes
        model_add_mesh(self, model_process_mesh(self, mesh, scene), scene->mNumMeshes);
    }

    for(uint32_t i = 0; i < node->mNumChildren; i++)
    {
        model_process_node(self, node->mChildren[i], scene);
    }
}

Mesh model_process_mesh(Model* self, struct aiMesh* model_mesh, const struct aiScene* scene)
{
    Mesh mesh;

    uint32_t* indices = NULL;
    uint32_t* tex_indices = NULL; // indexes into model's textures array
    const uint32_t total_vertices = model_mesh->mNumVertices;
    uint32_t total_indices = 0;
    uint32_t total_textures = 0;

    // allocation (arena freed by mesh)
    Arena arena;
    // loop through faces to count indices for allocation of indices array
    for(uint32_t i = 0; i < model_mesh->mNumFaces; i++)
    {
        struct aiFace face = model_mesh->mFaces[i];

        // prevent lines or single vertices from being added
        if(face.mNumIndices < 3) continue;
        total_indices += face.mNumIndices;
    }

    const size_t total_mem_size = ((size_t)total_vertices * 8 * sizeof(float)) + ((size_t)total_indices * sizeof(uint32_t));
    arena = arena_create(total_mem_size);

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(&arena, total_vertices * sizeof(vec3)),
        .normal = (vec3*)arena_alloc(&arena, total_vertices * sizeof(vec3)),
        .uv = (vec2*)arena_alloc(&arena, total_vertices * sizeof(vec2))
    };
    memset(vertex_buffer.uv, 0, total_vertices * sizeof(vec2)); // if no tex coords, then all values zeroed out
    ASSERT(vertex_buffer.pos != NULL && vertex_buffer.normal != NULL && vertex_buffer.uv != NULL, "MODEL: failed to allocate vertices\n");

    indices = (uint32_t*)arena_alloc(&arena, total_indices * sizeof(uint32_t));
    ASSERT(indices != NULL, "MODEL: failed to allocate indices\n");

    for(uint32_t i = 0; i < total_vertices; i++)
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
            uv = vec2_zero();
        }

        vertex_buffer.pos[i] = pos;
        vertex_buffer.normal[i] = normal;
        vertex_buffer.uv[i] = uv;
    }

    // loop through faces and indices and add each to array
    uint32_t ind_count = 0;
    for(uint32_t i = 0; i < model_mesh->mNumFaces; i++)
    {
        struct aiFace face = model_mesh->mFaces[i];

        if(face.mNumIndices < 3) continue; // prevent non triangular faces being added
        for(uint32_t j = 0; j < face.mNumIndices; j++)
        {
            indices[ind_count] = face.mIndices[j];
            ind_count++;
        }
    }

    // aiMesh's material index indexes into scene's pool of materials
    struct aiMaterial* mat = scene->mMaterials[model_mesh->mMaterialIndex];
    uint32_t diffuse_count, specular_count;

    // not the best since allocs multiple temp variables, but it works
    uint32_t* diff_indexes = model_load_textures(self, mat, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE, &diffuse_count);
    uint32_t* spec_indexes = model_load_textures(self, mat, aiTextureType_SPECULAR, TEXTURE_SPECULAR, &specular_count);

    total_textures = diffuse_count + specular_count;
    if(total_textures) // if no textures, don't allocate/memcpy/free
    {
        tex_indices = (uint32_t*)calloc(total_textures, sizeof(uint32_t));

        // copy separate indexes into tex_indices
        memcpy(tex_indices, diff_indexes, diffuse_count * sizeof(uint32_t));
        memcpy(tex_indices + diffuse_count, spec_indexes, specular_count * sizeof(uint32_t));

        free(diff_indexes);
        free(spec_indexes);
    }

    mesh_init(&mesh, arena, vertex_buffer, total_vertices, indices, total_indices, tex_indices, total_textures);
    return mesh;
}

uint32_t* model_load_textures(Model* self, struct aiMaterial* mat, enum aiTextureType ai_type, TextureType type, uint32_t* tex_count_out)
{
    uint32_t add_tex_count = aiGetMaterialTextureCount(mat, ai_type); // textures of given type
    *tex_count_out = add_tex_count;
    if(add_tex_count == 0) return NULL;
    uint32_t* tex_indices = (uint32_t*)calloc(add_tex_count, sizeof(uint32_t)); // user of function must free themselves

    char img_path[1024 + MAX_PATH_LENGTH]; // suppress warnings for snprintf (dir + '/' + str.data)
    uint32_t new_add_tex_count = 0; // since we only resize if texture doesn't already exist
    for(uint32_t i = 0; i < add_tex_count; i++)
    {
        memset(img_path, 0, sizeof(img_path)); // ensure previous string doesn't cause problems
        struct aiString str;
        aiGetMaterialTexture(mat, ai_type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL); // get material texture string
        snprintf(img_path, sizeof(img_path), "%s/%s", self->directory, str.data); // append texture string to directory

        bool skip = false;
        for(uint32_t j = 0; j < self->material.tex_count; j++) // loop through model's textures
        {
            if(strcmp(img_path, self->material.textures[j].path) == 0) // if texture already exists
            {
                tex_indices[i] = j;
                skip = true;
                break;
            }
        }

        if(!skip) // texture doesn't already exist
        {
            // increase amount to add to tex_count and reallocate textures
            new_add_tex_count++;
            self->material.textures = (Texture*)realloc(self->material.textures, (self->material.tex_count + new_add_tex_count) * sizeof(Texture));
            ASSERT(self->material.textures != NULL, "MODEL: failed to realloc texture array\n");

            // create new texture
            Texture texture;
            texture_create(&texture, type, img_path, true);

            // set next texture in array to current texture
            self->material.textures[self->material.tex_count + i] = texture; // first iter is tex_count + 0
            tex_indices[i] = self->material.tex_count + i; // store index into textures array for mesh to access
        }
    }

    // increase tex_count to correct size
    self->material.tex_count += new_add_tex_count;
    return tex_indices;
}

void model_free(Model* self)
{
    for(uint32_t i = 0; i < self->mesh_count; i++)
    {
        mesh_free(&self->meshes[i]);
    }

    free(self->meshes);

    if(self->material.textures == NULL && self->material.tex_count == 0) return; // if only 1 of these conditions occurs smth is wrong so continue and give error

    for(uint32_t i = 0; i < self->material.tex_count; i++)
    {
        texture_free(&self->material.textures[i]);
    }
    free(self->material.textures);
}