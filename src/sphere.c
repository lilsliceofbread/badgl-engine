#include "sphere.h"
#include "util.h"
#include <stdbool.h>
#include <stdlib.h>

Sphere gen_uv_sphere(vec3 pos, float radius, int horizontals, int verticals, const char* img_path)
{
    Sphere self;
    self.pos = pos;
    self.radius = radius;
    Vertex* vertices = NULL;
    uint32_t* indices = NULL;
    uint32_t* tex_indexes = NULL;
    uint32_t vert_count = 0, ind_count = 0;
    uint32_t tex_count = img_path == NULL ? 0 : 1; // 1 or 0 for now

    vertices = (Vertex*)malloc((horizontals * verticals + 2) * sizeof(Vertex)); // might be extra
    ASSERT(vertices != NULL, "SPHERE: failed to allocate vertices");
    indices = (uint32_t*)malloc((6 * verticals * (horizontals - 1)) * sizeof(uint32_t)); // FIX SIZE LATER
    ASSERT(indices != NULL, "SPHERE: failed to allocate indices");
    if(img_path != NULL) // for now
    {
        texture_create(&self.texture, img_path, true);
        self.texture.type = TEXTURE_DIFFUSE;
        tex_indexes = (uint32_t*)malloc(tex_count * sizeof(uint32_t));
        tex_indexes[0] = 0;
    }

    // top vertex
    vertices[0].pos.x = 0.0f;
    vertices[0].pos.y = radius;
    vertices[0].pos.z = 0.0f;

    vertices[0].normal.x = 0.0f;
    vertices[0].normal.y = 1.0f;
    vertices[0].normal.z = 0.0f;

    vertices[0].uv = vec2_zero(); // fix later
    vert_count++;

    float inv_hori = 1.0f / (float)horizontals;
    float inv_vert = 1.0f / (float)verticals;
    float inv_radius = 1.0f / radius;
    for(int i = 0; i < horizontals; i++) // latitudes (start after top vertex and stop before bottom vertex)
    {
        // only 1 PI because parallels are half the circumference
        float horizontal_angle = GL_PI * (i + 1) * inv_hori;
        float ch = cosf(horizontal_angle);
        float sh = sinf(horizontal_angle);

        for(int j = 0; j < verticals; j++) // longitudes
        {
            // 2 PI because it's the full circumference
            float meridian_angle = 2 * GL_PI * j * inv_vert;

            Vertex vertex;

            // calculate cartesian coords from spherical
            vertex.pos.x = radius * sh * cosf(meridian_angle);
            vertex.pos.y = radius * ch;
            vertex.pos.z = radius * sh * sinf(meridian_angle);

            vertex.normal.x = vertex.pos.x * inv_radius;
            vertex.normal.y = vertex.pos.y * inv_radius;
            vertex.normal.z = vertex.pos.z * inv_radius;

            vertex.uv.u = (j + 1) * inv_vert;
            vertex.uv.v = (i + 1) * inv_hori;

            vertices[vert_count] = vertex;
            vert_count++;
        }
    }

    // bottom vertex
    vertices[vert_count].pos.x = 0.0f;
    vertices[vert_count].pos.y = -radius;
    vertices[vert_count].pos.z = 0.0f;

    vertices[vert_count].normal.x = 0.0f;
    vertices[vert_count].normal.y = -1.0f;
    vertices[vert_count].normal.z = 0.0f;

    vertices[vert_count].uv = vec2_zero(); // fix later
    vert_count++;

    // top triangle indices
    for(uint32_t i = 0; i < verticals; i++)
    {
        uint32_t p1 = (i + 1) % verticals + 1;
        uint32_t p2 = i + 1;

        indices[ind_count] = 0;
        indices[ind_count + 1] = p1;
        indices[ind_count + 2] = p2;

        ind_count += 3;
    }

    // don't know how this works
    for(uint32_t i = 0; i < horizontals - 2; i++)
    {
        uint32_t a_start = i * verticals + 1;
        uint32_t b_start = (i + 1) * verticals + 1;
        
        for(uint32_t j = 0; j < verticals; j++)
        {
            uint32_t a1 = a_start + j;
            uint32_t a2 = a_start + (j + 1) % verticals;
            uint32_t b1 = b_start + j;
            uint32_t b2 = b_start + (j + 1) % verticals;

            indices[ind_count] = a1;
            indices[ind_count + 1] = a2;
            indices[ind_count + 2] = b2;
            indices[ind_count + 3] = a1;
            indices[ind_count + 4] = b2;
            indices[ind_count + 5] = b1;

            ind_count += 6;
        }

    }

    // bottom triangle indices
    for(uint32_t i = 0; i < verticals; i++)
    {
        uint32_t p1 = i + verticals * (horizontals - 2) + 1;
        uint32_t p2 = (i + 1) % verticals + verticals * (horizontals - 2) + 1;

        indices[ind_count] = vert_count - 1;
        indices[ind_count + 1] = p1;
        indices[ind_count + 2] = p2;

        ind_count += 3;
    }

    // WHEN ADDING MORE TEXTURES REMEMBER TO CHANGE 0 TEX_INDEX
    mesh_init(&self.mesh, vertices, vert_count, indices, ind_count, tex_indexes, tex_count);

    return self;
}

void sphere_draw(Sphere* self, Shader* shader)
{
    //mat4 model = mat4_identity();
    //model = mat4_trans(model, self->pos);
    //shader_uniform_mat4(shader, "model", model); // should use this or no?
    mesh_draw(&self->mesh, shader, &self->texture);
}

void sphere_free(Sphere* self)
{
    mesh_free(&self->mesh);
}