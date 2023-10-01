#include "sphere.h"
#include "util.h"
#include <stdbool.h>
#include <stdlib.h>

Sphere uv_sphere_gen(vec3 pos, float radius, uint32_t resolution, const char* cubemap_path)
{
    Sphere self;
    self.pos = pos;
    self.radius = radius;
    Vertex* vertices = NULL;
    uint32_t* indices = NULL;
    uint32_t* tex_indexes = NULL;
    uint32_t vert_count = 0, ind_count = 0;
    uint32_t tex_count = cubemap_path == NULL ? 0 : 1; // only 1 if cubemap path has been given

    const uint32_t horizontals = resolution, verticals = 2 * resolution;

    // calloc to initialise all uvs to 0 since we won't be using them
    vertices = (Vertex*)calloc((horizontals * verticals + 2), sizeof(Vertex)); // (h * v) + 2 is the amt of vertices
    ASSERT(vertices != NULL, "SPHERE: failed to allocate vertices");
    indices = (uint32_t*)malloc((6 * verticals * (horizontals - 1)) * sizeof(uint32_t)); // 6 * v * (h - 1) is the amt of indices
    ASSERT(indices != NULL, "SPHERE: failed to allocate indices");
    if(cubemap_path != NULL) // for now
    {
        texture_cubemap_create(&self.texture, cubemap_path);
        tex_indexes = (uint32_t*)malloc(sizeof(uint32_t));
        tex_indexes[0] = 0; // the only index
    }

    // top vertex
    vertices[0].pos.x = 0.0f;
    vertices[0].pos.y = radius;
    vertices[0].pos.z = 0.0f;

    vertices[0].normal.x = 0.0f;
    vertices[0].normal.y = 1.0f;
    vertices[0].normal.z = 0.0f;

    vert_count++;

    const float inv_hori = 1.0f / (float)horizontals;
    const float inv_vert = 1.0f / (float)verticals;
    const float inv_radius = 1.0f / radius;
    const float pi2 = 2.0f * GL_PI; // multiply 2 * PI before hand
    for(uint32_t i = 0; i < horizontals; i++) // latitudes (start after top vertex (i+1) and stop before bottom vertex (< not <=))
    {
        // only 1PI because horizontal stacks will only need to sweep half the circle (other half is just the reverse)
        const float horizontal_angle = (float)(i + 1) * GL_PI * inv_hori;

        const float sh = sinf(horizontal_angle);
        const float y = radius * cosf(horizontal_angle); // same for whole horizontal

        for(uint32_t j = 0; j < verticals; j++) // longitudes (< to stop before 2PI which is a repeat of 0)
        {
            /* 2PI because it's going around the full circumference.
               because opengl is right-handed, sin/cos of vertical angle
               rotates opposite to normal unit circle (clockwise instead of anti) */
            float vertical_angle = (float)j * pi2 * inv_vert;

            Vertex vertex;

            // calculate cartesian coords from spherical
            vertex.pos.x = radius * sh * cosf(vertical_angle);
            vertex.pos.y = y;
            vertex.pos.z = radius * sh * sinf(vertical_angle);

            vertex.normal.x = vertex.pos.x * inv_radius;
            vertex.normal.y = vertex.pos.y * inv_radius;
            vertex.normal.z = vertex.pos.z * inv_radius;

            vertices[vert_count] = vertex;
            vert_count++;
        }
    }
    // this creates (horizontals - 1) horizontals as the loop goes from 1 to horizontals

    // bottom vertex
    vertices[vert_count].pos.x = 0.0f;
    vertices[vert_count].pos.y = -radius;
    vertices[vert_count].pos.z = 0.0f;

    vertices[vert_count].normal.x = 0.0f;
    vertices[vert_count].normal.y = -1.0f;
    vertices[vert_count].normal.z = 0.0f;

    vert_count++;

    // top triangle's indices
    for(uint32_t i = 0; i < verticals; i++)
    {
        uint32_t p1 = i + 1; // add 1 because 0 is top vertex
        uint32_t p2 = ((i + 1) % verticals) + 1; // i + 2 but remember to wrap around back to 0 instead of going above verticals

        indices[ind_count] = 0; // first vertex is top vertex
        indices[ind_count + 1] = p2; // p2 first in anticlockwise order
        indices[ind_count + 2] = p1;

        ind_count += 3;
    }

    /* loop through quads
       horizontals - 2 represents stopping before final bottom triangles,
       as each iteration involves 1 layer and another below, so another
       iteration would go past that and there are h-1 horizontals as i said before */
    for(uint32_t i = 0; i < horizontals - 2; i++)
    {
        uint32_t layer1 = i * verticals + 1; // add 1 because of top vertex and then go down horizontals by multiplying by num verticals
        uint32_t layer2 = (i + 1) * verticals + 1; // i + 1 gives next layer down
        
        for(uint32_t j = 0; j < verticals; j++)
        {
            // top right and anticlockwise
            uint32_t tr = layer1 + j; // j: from 0 to verticals - 1
            uint32_t tl = layer1 + (j + 1) % verticals; // j + 1 but remembering to wrap back to 0
            uint32_t br = layer2 + j;
            uint32_t bl = layer2 + (j + 1) % verticals;

            // anticlockwise quad indices
            indices[ind_count] = tr; // tri 1
            indices[ind_count + 1] = tl;
            indices[ind_count + 2] = bl;

            indices[ind_count + 3] = tr; // tri 2
            indices[ind_count + 4] = bl;
            indices[ind_count + 5] = br;

            ind_count += 6;
        }

    }

    // bottom triangle's indices
    const uint32_t final_layer_index = verticals * (horizontals - 2) + 1; // add 1 because of top vertex
    for(uint32_t i = 0; i < verticals; i++)
    {
        uint32_t p1 = final_layer_index + i; // first vertex
        uint32_t p2 = final_layer_index + ((i + 1) % verticals); // next vertex, remembering to wrap back to 0 on last

        indices[ind_count] = vert_count - 1; // first vertex is bottom vertex
        indices[ind_count + 1] = p1;
        indices[ind_count + 2] = p2;

        ind_count += 3;
    }

    mesh_init(&self.mesh, vertices, vert_count, indices, ind_count, tex_indexes, tex_count);

    return self;
}

void sphere_draw(Sphere* self, Shader* shader)
{
    mat4 model = mat4_identity();
    mat4_trans(&model, self->pos);
    shader_uniform_mat4(shader, "model", model);
    mesh_draw(&self->mesh, shader, &self->texture);
}

void sphere_free(Sphere* self)
{
    mesh_free(&self->mesh);
}