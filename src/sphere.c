#include "sphere.h"
#include "util.h"
#include <stdbool.h>
#include <stdlib.h>

Sphere uv_sphere_gen(vec3 pos, float radius, uint32_t resolution, const char* cubemap_path, uint32_t shader_index)
{
    Sphere self;
    self.pos = pos;
    self.radius = radius;
    self.shader_index = shader_index;
    uint32_t* indices = NULL;
    uint32_t* tex_indexes = NULL;
    uint32_t vert_count = 0, ind_count = 0;
    uint32_t tex_count = cubemap_path == NULL ? 0 : 1; // only 1 if cubemap path has been given

    const uint32_t horizontals = resolution, verticals = 2 * resolution;
    const size_t total_vertices = horizontals * verticals + 2;
    const size_t total_indices = 6 * verticals * (horizontals - 1); // 6 indices per square, but top and bottom rings are triangles (3 per), so h - 2 + 1

    Arena arena = arena_create((total_vertices * 2 * sizeof(vec3)) + (total_indices * sizeof(uint32_t)));
    ASSERT(arena.raw_memory != NULL, "SPHERE: failed to allocate arena for buffer\n");

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(&arena, total_vertices * sizeof(vec3)),
        .normal = (vec3*)arena_alloc(&arena, total_vertices * sizeof(vec3)),
        .uv = NULL
    };
    ASSERT(vertex_buffer.pos != NULL || vertex_buffer.normal != NULL, "SPHERE: failed to allocate vertices\n");

    indices = (uint32_t*)arena_alloc(&arena, total_indices * sizeof(uint32_t));
    ASSERT(indices != NULL, "SPHERE: failed to allocate indices\n");

    if(cubemap_path != NULL) // for now
    {
        texture_cubemap_create(&self.texture, cubemap_path);
        tex_indexes = (uint32_t*)malloc(sizeof(uint32_t));
        tex_indexes[0] = 0; // the only index
    }

    // top vertex
    vertex_buffer.pos[0].x = 0.0f;
    vertex_buffer.pos[0].y = radius;
    vertex_buffer.pos[0].z = 0.0f;

    vertex_buffer.normal[0].x = 0.0f;
    vertex_buffer.normal[0].y = 1.0f;
    vertex_buffer.normal[0].z = 0.0f;

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
               since opengl is right-handed this creates vertices clockwise
               as the unit circle is reversed with +z going "down" */
            float vertical_angle = (float)j * pi2 * inv_vert;

            vec3 pos, normal;
            // calculate cartesian coords from spherical
            pos.x = radius * sh * cosf(vertical_angle);
            pos.y = y;
            pos.z = radius * sh * sinf(vertical_angle);

            normal.x = pos.x * inv_radius;
            normal.y = pos.y * inv_radius;
            normal.z = pos.z * inv_radius;

            vertex_buffer.pos[vert_count] = pos;
            vertex_buffer.normal[vert_count] = normal;
            vert_count++;
        }
    }
    // this creates (horizontals - 1) horizontals as the loop goes from 1 to horizontals

    // bottom vertex
    vertex_buffer.pos[vert_count].x = 0.0f;
    vertex_buffer.pos[vert_count].y = -radius;
    vertex_buffer.pos[vert_count].z = 0.0f;

    vertex_buffer.normal[vert_count].x = 0.0f;
    vertex_buffer.normal[vert_count].y = -1.0f;
    vertex_buffer.normal[vert_count].z = 0.0f;

    vert_count++;

    // top triangle's indices
    for(uint32_t i = 0; i < verticals; i++)
    {
        uint32_t p1 = (i) + 1; // add 1 because 0 is top vertex
        uint32_t p2 = ((i + 1) % verticals) + 1; // i + 2 but remember to wrap around back to 0 instead of going above verticals

        indices[ind_count] = 0; // first vertex is top vertex
        indices[ind_count + 1] = p2; // p2 first in clockwise order
        indices[ind_count + 2] = p1;

        ind_count += 3;
    }

    /* loop through quads
       horizontals - 2 represents stopping before final bottom triangles,
       as each iteration involves 1 layer and another below, and the top
       and bottom layers only have 1 set of triangle indices as they are
       triangles but the quads have 2, so 1 less set on top */
    for(uint32_t i = 0; i < horizontals - 2; i++)
    {
        uint32_t layer1 = i * verticals + 1; // add 1 because of top vertex and then go down horizontals by multiplying by num verticals
        uint32_t layer2 = (i + 1) * verticals + 1; // i + 1 gives next layer down
        
        for(uint32_t j = 0; j < verticals; j++)
        {
            // top right and clockwise
            uint32_t tl = layer1 + j; // j: from 0 to verticals - 1
            uint32_t tr = layer1 + (j + 1) % verticals; // j + 1 but remembering to wrap back to 0
            uint32_t bl = layer2 + j;
            uint32_t br = layer2 + (j + 1) % verticals;

            // clockwise quad indices
            indices[ind_count] = tl; // tri 1
            indices[ind_count + 1] = tr;
            indices[ind_count + 2] = br;

            indices[ind_count + 3] = tl; // tri 2
            indices[ind_count + 4] = br;
            indices[ind_count + 5] = bl;

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

    mesh_init(&self.mesh, arena, vertex_buffer, vert_count, indices, ind_count, tex_indexes, tex_count);

    return self;
}

void sphere_draw(Sphere* self, Renderer* rd, mat4* vp)
{
    Shader* shader_ptr = &rd->shaders[self->shader_index];
    shader_use(shader_ptr);

    shader_uniform_mat4(shader_ptr, "vp", vp);

    mat4 model = mat4_identity();
    mat4_trans(&model, self->pos);
    shader_uniform_mat4(shader_ptr, "model", &model);

    mesh_draw(&self->mesh, shader_ptr, &self->texture);
}

void sphere_free(Sphere* self)
{
    mesh_free(&self->mesh);
    texture_free(&self->texture);
}