#ifndef BADGL_MESH_H
#define BADGL_MESH_H

#include <inttypes.h>
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
    // don't need to store this since it's only used in mesh_init()
    //VertexBuffer vertex_buffer;
    //uint32_t* indices;
    uint32_t* tex_indices; // indexes into parent structure's array
    uint32_t vert_count, ind_count, tex_count;

    VAO vao;
    VBO vbo;
    EBO ebo;
} Mesh;

// all vertices, indices and textures should be pre-allocated, and will be freed in mesh_free()
void mesh_init(Mesh* self, Arena arena,
               VertexBuffer vertex_buffer, uint32_t vert_count,
               uint32_t* indices, uint32_t ind_count,
               uint32_t* tex_indices, uint32_t tex_count);

void mesh_draw(Mesh* self, Shader* shader, Texture* textures);

void mesh_free(Mesh* self);

#endif