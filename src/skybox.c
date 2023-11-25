#include "skybox.h"
#include <stdlib.h>
#include <string.h>

Skybox skybox_init(const char* cubemap_path)
{
    Skybox self;

    // preset since all skyboxes should be the same
    shader_init(&self.shader, "shaders/skybox.vert", "shaders/skybox.frag");

    texture_cubemap_create(&self.texture, cubemap_path);

    // skybox has predefined sizes
    Vertex* vertices = (Vertex*)malloc(8 * sizeof(Vertex));
    uint32_t* indices = (uint32_t*)malloc(6 * 6 * sizeof(uint32_t));
    uint32_t* tex_indexes = (uint32_t*)malloc(sizeof(uint32_t));
    tex_indexes[0] = 0; // only 1 texture

    // fix later, shouldn't need to do this (mesh specify normal/uv separately)
    {
        vec3 vertex_positions[] = {
            {-1, -1,  1}, // 0
            { 1, -1,  1}, // 1
            {-1,  1,  1}, // 2
            { 1,  1,  1}, // 3
            {-1, -1, -1}, // 4
            { 1, -1, -1}, // 5
            {-1,  1, -1}, // 6
            { 1,  1, -1}  // 7
        };

        for(int i = 0; i < 8; i++)
        {
            vertices[i].pos = vertex_positions[i];

            vertices[i].normal = vec3_zero();
            vertices[i].uv = vec2_zero();
        }
    }

    {
        // clockwise indices for inside of the cube (technically anticlockwise)
        uint32_t indices_tmp[] = {
            //Top
            2, 6, 7,
            2, 7, 3,

            //Bottom
            0, 5, 4,
            0, 1, 5,

            //Left
            0, 6, 2,
            0, 4, 6,

            //Right
            1, 3, 7,
            1, 7, 5,

            //Front
            0, 2, 3,
            0, 3, 1,

            //Back
            4, 7, 6,
            4, 5, 7
        };

        // have to set it this way because it is on the heap
        memcpy(indices, indices_tmp, sizeof(indices_tmp));
    }

    mesh_init(&self.mesh, vertices, 8, indices, 6 * 6, tex_indexes, 1);

    return self;
}

void skybox_draw(Skybox* self, Camera* cam)
{
    mat4 vp; // no translation
    mat4 corrected_view = cam->view;
    corrected_view.m14 = corrected_view.m24 = corrected_view.m34 = 0.0f; // remove translation
    mat4_mul(&vp, cam->proj, corrected_view);

    glDepthFunc(GL_LEQUAL);

    shader_use(&self->shader);
    shader_uniform_mat4(&self->shader, "vp", vp);
    mesh_draw(&self->mesh, &self->shader, &self->texture);

    glDepthFunc(GL_LESS);
}

void skybox_free(Skybox* self)
{
    mesh_free(&self->mesh);
    shader_free(&self->shader);
    texture_free(&self->texture);
}