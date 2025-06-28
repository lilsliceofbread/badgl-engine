#ifndef BGL_MESH_H
#define BGL_MESH_H

#include "bgl_math.h"
#include "texture.h"
#include "vao.h"
#include "bo.h"
#include "shader.h"
#include "arena.h"

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

/* vertices and indices are freed by caller as they are not stored but tex_indices are stored by mesh and so freed by mesh */
void mesh_create(Mesh* self,
                 const VertexBuffer vertex_buffer, u32 vert_count,
                 const u32* indices, u32 ind_count,
                 u32* tex_indices, u32 tex_count);

void mesh_draw(Mesh* self, Shader* shader, Texture* textures);

void mesh_free(Mesh* self);

#endif
