#include "mesh.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vao.h"
#include "bo.h"

void mesh_init(Mesh* self, VertexBuffer vertex_buffer, uint32_t vert_count, uint32_t* indices, uint32_t ind_count,
               uint32_t* tex_indexes, uint32_t tex_count)
{
    // preallocated
    self->vertex_buffer = vertex_buffer;
    self->indices = indices;
    self->tex_indexes = tex_indexes;
    self->vert_count = vert_count;
    self->ind_count = ind_count;
    self->tex_count = tex_count;

    size_t vertex_size = 3;
    bool use_normals = false, use_UVs = false;
    if(self->vertex_buffer.normal != NULL)
    {
        use_normals = true;
        vertex_size += 3;
    }
    if(self->vertex_buffer.uv != NULL)
    {
        use_UVs = true;
        vertex_size += (size_t)(2 + 3 * (!use_normals)); // too complicated to figure out how to do vertex attributes if uvs but not normals so do this instead
        // adds 3 byte padding between in this case
    }

    self->vao = vao_create();
    self->vbo = vbo_create();
    self->ebo = ebo_create();

    vao_bind(self->vao);

    vbo_bind(self->vbo);

    size_t single_size = self->vert_count * sizeof(float);
    vbo_set_buffer(self->vbo, NULL, single_size * vertex_size, false);

    vbo_set_buffer_region(self->vbo, self->vertex_buffer.pos, 0, 3 * single_size);
    if(use_normals) vbo_set_buffer_region(self->vbo, self->vertex_buffer.normal, 3 * (int)single_size, 3 * single_size);
    if(use_UVs)     vbo_set_buffer_region(self->vbo, self->vertex_buffer.uv, 6 * (int)single_size, 2 * single_size);

    ebo_bind(self->ebo);
    ebo_set_buffer(self->ebo, &indices[0], ind_count * sizeof(uint32_t), false);

    vao_attribute(0, 3, GL_FLOAT, 3 * sizeof(float), 0);
    if(use_normals) vao_attribute(1, 3, GL_FLOAT, 3 * sizeof(float), 3 * single_size);
    if(use_UVs)     vao_attribute(2, 2, GL_FLOAT, 2 * sizeof(float), 6 * single_size);

    vao_unbind();
}

void mesh_draw(Mesh* self, Shader* shader, Texture* textures)
{
    // current num of each texture type
    uint32_t diffuse_num = 1, specular_num = 1;
    char sampler_name[100];

    for(uint32_t i = 0; i < self->tex_count; i++)
    {
        uint32_t curr_tex = self->tex_indexes[i];

        switch(textures[curr_tex].type)
        {
            case TEXTURE_CUBEMAP:
                break;
            case TEXTURE_DIFFUSE: case TEXTURE_SPECULAR: case TEXTURE_NORMAL:
                // activate next tex unit
                texture_unit_active(i);
                // fix for texture types other than diffuse/spec later
                uint32_t num = textures[curr_tex].type == TEXTURE_DIFFUSE ? diffuse_num : specular_num;
                snprintf(sampler_name, 100, "%s%d", texture_type_get_str(textures[curr_tex].type), num);
                shader_uniform_1i(shader, sampler_name, (int)i);
                break;
        }
        // tell sampler which texture unit to use
        //printf("LOG: texture name %s\n", sampler_name);
        texture_bind(&textures[curr_tex]);
    }

    vao_bind(self->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)self->ind_count, GL_UNSIGNED_INT, 0);
    vao_unbind();

    // good practice to reset
    texture_unit_active(0);
}

void mesh_free(Mesh* self)
{
    free(self->vertex_buffer.pos);
    if(self->vertex_buffer.normal != NULL)
    {
        free(self->vertex_buffer.normal);
    }
    if(self->vertex_buffer.uv != NULL)
    {
        free(self->vertex_buffer.uv);
    }

    free(self->indices);
    if(self->tex_count > 0 && self->tex_indexes != NULL)
    {
        free(self->tex_indexes);
    }
    
    //printf("MESH: vertices: %d indices %d\n", self->vert_count, self->ind_count);

    vao_free(self->vao);
    bo_free(self->vbo);
    bo_free(self->ebo);
}