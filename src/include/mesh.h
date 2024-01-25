#ifndef BGL_MESH_H
#define BGL_MESH_H

#include "glmath.h"
#include "texture.h"
#include "vao.h"
#include "bo.h"
#include "shader.h"
#include "arena_alloc.h"

typedef struct VertexBuffer
{
    vec3* pos;
    vec3* normal;
    vec2* uv;
} VertexBuffer;

typedef struct Mesh
{
    u32* tex_indices; // indexes into parent structure's array
    u32 vert_count, ind_count, tex_count;

    VAO vao;
    VBO vbo;
    EBO ebo;
} Mesh;

// all vertices, indices and textures should be pre-allocated, and will be freed in mesh_free()
void mesh_create(Mesh* self, Arena arena,
               VertexBuffer vertex_buffer, u32 vert_count,
               u32* indices, u32 ind_count,
               u32* tex_indices, u32 tex_count);

void mesh_draw(Mesh* self, Shader* shader, Texture* textures);

void mesh_free(Mesh* self);

#endif