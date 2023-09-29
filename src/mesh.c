#include "mesh.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vao.h"
#include "bo.h"

void mesh_init(Mesh* self, Vertex* vertices, uint32_t vertices_count, uint32_t* indices, uint32_t indices_count,
               uint32_t* tex_indexes, uint32_t textures_count)
{
    // preallocated
    self->vertices = vertices;
    self->indices = indices;
    self->tex_indexes = tex_indexes;
    self->vert_count = vertices_count;
    self->ind_count = indices_count;
    self->tex_count = textures_count;

    self->vao = vao_create();
    self->vbo = bo_create(GL_ARRAY_BUFFER);
    self->ebo = bo_create(GL_ELEMENT_ARRAY_BUFFER);

    vao_bind(self->vao);

    bo_bind(self->vbo);
    bo_set_buffer(self->vbo, &vertices[0], vertices_count * sizeof(Vertex), false);

    bo_bind(self->ebo);
    bo_set_buffer(self->ebo, &indices[0], indices_count * sizeof(uint32_t), false);

    vao_attribute(0, 3, GL_FLOAT, sizeof(Vertex), 0);
    vao_attribute(1, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, normal));
    vao_attribute(2, 2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, uv));

    vao_unbind();
}

void mesh_draw(Mesh* self, Shader* shader, Texture* textures)
{
    // current num of each texture type
    uint32_t diffuse_num = 1, specular_num = 1;
    char sampler_name[MAX_STR_LENGTH];
    for(uint32_t i = 0; i < self->tex_count; i++)
    {
        uint32_t curr_tex = self->tex_indexes[i];

        // activate next tex unit
        texture_unit_active(i);
        // fix for texture types other than diffuse/spec later
        uint32_t num = textures[curr_tex].type == TEXTURE_DIFFUSE ? diffuse_num : specular_num;
        sprintf(sampler_name, "%s%d", texture_type_get_str(textures[curr_tex].type), num);

        // tell sampler which texture unit to use
        //printf("LOG: texture name %s\n", sampler_name);
        shader_uniform_1i(shader, sampler_name, i);
        texture_bind(textures[curr_tex]);
    }

    vao_bind(self->vao);
    glDrawElements(GL_TRIANGLES, self->ind_count, GL_UNSIGNED_INT, 0);
    vao_unbind();

    // good practice to reset
    texture_unit_active(0);
}

void mesh_free(Mesh* self)
{
    free(self->vertices);
    free(self->indices);
    if(self->tex_count > 0)
    {
        free(self->tex_indexes);
    }

    /*// temp
    for(int i = 0; i < self->tex_count; i++)
    {
        printf("MESH: idx - %d\n", self->tex_indexes[i]);
    }
    */

    vao_free(self->vao);
    bo_free(self->vbo);
    bo_free(self->ebo);
    // free textures?
}