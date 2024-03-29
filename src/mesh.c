#include "mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "vao.h"
#include "renderer.h"
#include "bo.h"
#include "arena_alloc.h"

void mesh_create(Mesh* self, Arena arena,
               VertexBuffer vertex_buffer, u32 vert_count,
               u32* indices, u32 ind_count,
               u32* tex_indices, u32 tex_count)
{
    // preallocated
    self->tex_indices = tex_indices;
    self->vert_count = vert_count;
    self->ind_count = ind_count;
    self->tex_count = tex_count;

    size_t vertex_size = 3;
    bool use_normals = false, use_UVs = false;
    if(vertex_buffer.normal != NULL)
    {
        use_normals = true;
        vertex_size += 3;
    }
    if(vertex_buffer.uv != NULL)
    {
        use_UVs = true;
        vertex_size += 2;
    }

    self->vao = vao_create();
    self->vbo = vbo_create();
    self->ebo = ebo_create();

    vao_bind(self->vao);

    vbo_bind(self->vbo);

    size_t single_size = self->vert_count * sizeof(float);
    vbo_set_buffer(self->vbo, NULL, single_size * vertex_size, false);

    vbo_set_buffer_region(self->vbo, vertex_buffer.pos, 0, 3 * single_size);
    if(use_normals) vbo_set_buffer_region(self->vbo, vertex_buffer.normal, 3 * (i32)single_size, 3 * single_size);
    if(use_UVs)
    {
        i32 offset = use_normals ? 6 * (i32)single_size : 3 * (i32)single_size; // different offset depending on if normals are used

        vbo_set_buffer_region(self->vbo, vertex_buffer.uv, offset, 2 * single_size);
    }

    ebo_bind(self->ebo);
    ebo_set_buffer(self->ebo, &indices[0], ind_count * sizeof(u32), false);

    vao_attribute(0, 3, GL_FLOAT, 3 * sizeof(float), 0);
    if(use_normals) vao_attribute(1, 3, GL_FLOAT, 3 * sizeof(float), 3 * single_size);

    if(use_UVs) 
    {
        u32 index = 1 + (u32)use_normals; // if normals used then will be 2, otherwise 1
        size_t offset = use_normals ? 6 * single_size : 3 * single_size;

        vao_attribute(index, 2, GL_FLOAT, 2 * sizeof(float), offset);
    }

    #ifndef BGL_NO_DEBUG
        vao_unbind(); // shouldn't need this if code works properly lol
    #endif

    // don't need this data anymore
    arena_free(&arena);
}

void mesh_draw(Mesh* self, Shader* shader, Texture* textures)
{
    for(u32 i = 0; i < self->tex_count; i++)
    {
        Texture curr_tex = textures[self->tex_indices[i]];

        texture_unit_active(i); // activate next tex unit

        // tell sampler which unit is associated with it
        shader_uniform_int(shader, texture_type_get_str(curr_tex.type), (i32)i);

        texture_bind(&curr_tex); // associate texture with current bound unit
    }

    vao_bind(self->vao);
    rd_draw_triangles(self->ind_count);
    vao_unbind();

    texture_unit_active(0);
}

void mesh_free(Mesh* self)
{
    if(self->tex_indices != NULL)
    {
        free(self->tex_indices);
    }
    
    vao_free(self->vao);
    vbo_free(self->vbo);
    ebo_free(self->ebo);
}