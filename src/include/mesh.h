#ifndef BADGL_MESH_H
#define BADGL_MESH_H

#include <inttypes.h>
#include "glmath.h"
#include "texture.h"
#include "vao.h"
#include "bo.h"
#include "shader.h"

typedef struct Vertex
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} Vertex;

typedef struct Mesh
{
    Vertex* vertices;
    uint32_t* indices;
    uint32_t* tex_indexes; // indexes into parent structure's array
    uint32_t vert_count, ind_count, tex_count;

    VAO vao;
    VBO vbo;
    EBO ebo;
} Mesh;

// all vertices, indices and textures should be pre-allocated, and will be freed in mesh_free()
void mesh_init(Mesh* self, Vertex* vertices, uint32_t vertices_count, uint32_t* indices, uint32_t indices_count,
               uint32_t* tex_indexes, uint32_t textures_count);

void mesh_draw(Mesh* self, Shader* shader, Texture* textures);

void mesh_free(Mesh* self);

#endif