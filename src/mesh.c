#include "mesh.h"

#include <string.h>
#include "vao.h"
#include "bo.h"

void mesh_init(Mesh* self, Vertex* vertices, uint32_t vertices_count, uint32_t* indices, uint32_t indices_count,
               Texture* textures, uint32_t textures_count)
{
    // preallocated
    self->vertices = vertices;
    self->indices = indices;
    self->textures = textures;
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
    bo_set_buffer(self->ebo, &indices[0], indices_count * sizeof(unsigned int), false);

    vao_attribute(0, 3, GL_FLOAT, sizeof(Vertex), 0);
    vao_attribute(1, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, normal));
    vao_attribute(2, 2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, uv));

    vao_unbind();
}

void mesh_draw(Mesh* self, Shader* shader)
{
    // current num of each texture type
    uint32_t diffuse_num = 1, specular_num = 1;
    for(uint32_t i = 0; i < self->tex_count; i++)
    {
        // activate tex unit
        glActiveTexture(GL_TEXTURE0 + i);
        const char* sampler_name;
        switch(self->textures[i].type)
        {
            case TEXTURE_DIFFUSE:
                sprintf(sampler_name, "material.texture_diffuse%d", diffuse_num);
                diffuse_num++;
                break;
            case TEXTURE_SPECULAR:
                sprintf(sampler_name, "material.texture_specular%d", specular_num);
                specular_num++;
                break;
        }

        // tell sampler which texture unit to use
        shader_uniform_1i(shader, sampler_name, i);
    }
    glActiveTexture(GL_TEXTURE0);

    vao_bind(self->vao);
    glDrawElements(GL_TRIANGLES, self->ind_count, GL_UNSIGNED_INT, 0);
    vao_unbind();
}

void mesh_free(Mesh* self)
{
    free(self->vertices);
    free(self->indices);
    free(self->textures);

    vao_free(self->vao);
    bo_free(self->vbo);
    bo_free(self->ebo);
    // free textures?
}