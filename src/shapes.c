#include "shapes.h"

#include <stdbool.h>
#include <stdlib.h>
#include "defines.h"
#include "util.h"

extern inline uint32_t* shape_setup(Model* model, const Material* material, uint32_t shader_idx);

void uv_sphere_gen(Model* self, float radius, uint32_t res, const Material* material, uint32_t shader_idx)
{
    uint32_t* tex_indices = shape_setup(self, material, shader_idx);

    uint32_t vert_count = 0, ind_count = 0;

    const uint32_t horizontals = res, verticals = 2 * res;
    const size_t total_vertices = horizontals * verticals + 2;
    const size_t total_indices = 6 * verticals * (horizontals - 1); // 6 indices per square, but top and bottom rings are triangles (3 per), so h - 2 + 1

    Arena mesh_arena = arena_create((total_vertices * 2 * sizeof(vec3)) + (total_indices * sizeof(uint32_t)));

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(&mesh_arena, total_vertices * sizeof(vec3)),
        .normal = NULL,
        .uv = NULL
    };
    ASSERT(vertex_buffer.pos != NULL, "failed to allocate vertices\n");

    uint32_t* indices = (uint32_t*)arena_alloc(&mesh_arena, total_indices * sizeof(uint32_t));
    ASSERT(indices != NULL, "failed to allocate indices\n");


    // top vertex
    vertex_buffer.pos[0].x = 0.0f;
    vertex_buffer.pos[0].y = radius;
    vertex_buffer.pos[0].z = 0.0f;

    vert_count++;

    const float inv_hori = 1.0f / (float)horizontals;
    const float inv_vert = 1.0f / (float)verticals;
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

            vec3 pos;
            // calculate cartesian coords from spherical
            pos.x = radius * sh * cosf(vertical_angle);
            pos.y = y;
            pos.z = radius * sh * sinf(vertical_angle);

            vertex_buffer.pos[vert_count] = pos;
            vert_count++;
        }
    }
    // this creates (horizontals - 1) horizontals as the loop goes from 1 to horizontals

    // bottom vertex
    vertex_buffer.pos[vert_count].x = 0.0f;
    vertex_buffer.pos[vert_count].y = -radius;
    vertex_buffer.pos[vert_count].z = 0.0f;

    vert_count++;

    // top triangle's indices
    for(uint32_t i = 0; i < verticals; i++)
    {
        uint32_t p1 = (i) + 1; // add 1 because 0 is top vertex
        uint32_t p2 = ((i + 1) % verticals) + 1; // i + 2 but remember to wrap around back to 0 instead of going above verticals

        indices[ind_count] = 0; // first vertex is top vertex
        indices[ind_count + 1] = p2; // p2 first in counter-clockwise order
        indices[ind_count + 2] = p1;

        ind_count += 3;
    }

    /**
     * loop through quads
     * horizontals - 2 represents stopping before final bottom triangles,
     * as each iteration involves 1 layer and another below, and the top
     * and bottom layers only have 1 set of triangle indices as they are
     * triangles but the quads have 2, so 1 less set on top 
    */
    for(uint32_t i = 0; i < horizontals - 2; i++)
    {
        uint32_t layer1 = i * verticals + 1; // add 1 because of top vertex and then go down horizontals by multiplying by num verticals
        uint32_t layer2 = (i + 1) * verticals + 1; // i + 1 gives next layer down
        
        for(uint32_t j = 0; j < verticals; j++)
        {
            uint32_t tr = layer1 + j; // j: from 0 to verticals - 1
            uint32_t tl = layer1 + (j + 1) % verticals; // j + 1 but remembering to wrap back to 0
            uint32_t br = layer2 + j;
            uint32_t bl = layer2 + (j + 1) % verticals;

            // counter-clockwise quad indices
            indices[ind_count] = tr;
            indices[ind_count + 1] = tl;
            indices[ind_count + 2] = bl;

            indices[ind_count + 3] = tr;
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

    mesh_init(&self->meshes[0], mesh_arena, vertex_buffer, vert_count, indices, ind_count, tex_indices, self->material.tex_count);
}

void rectangular_prism_gen(Model* self, float width, float height, float depth, const Material* material, uint32_t shader_idx)
{
    uint32_t* tex_indices = shape_setup(self, material, shader_idx);
    
    const size_t vert_size = 24 * sizeof(vec3);
    const size_t ind_size = 6 * 6 * sizeof(uint32_t);
    Arena arena = arena_create((2 * vert_size) + ind_size); // 2 vertex attributes

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(&arena, vert_size),
        .normal = (vec3*)arena_alloc(&arena, vert_size),
        .uv = NULL
    };
    ASSERT(vertex_buffer.pos != NULL && vertex_buffer.normal != NULL, "failed to allocate vertices\n");

    uint32_t* indices = (uint32_t*)arena_alloc(&arena, ind_size);
    ASSERT(indices != NULL, "failed to allocate indices\n");

    {
        // half width, height, depth
        const float hw = 0.5f * width;
        const float hh = 0.5f * height;
        const float hd = 0.5f * depth;

        // need to specify 24 vertices to allow normals to be flat
        const vec3 vertex_positions[] = {
            {-hw, -hh, hd}, // front
            { hw, -hh, hd},
            { hw,  hh, hd},
            {-hw,  hh, hd},

            {-hw, hh,  hd}, // top
            { hw, hh,  hd},
            { hw, hh, -hd},
            {-hw, hh, -hd},

            { hw, -hh, -hd}, // back
            {-hw, -hh, -hd},
            {-hw,  hh, -hd},
            { hw,  hh, -hd},

            {-hw, -hh, -hd}, // bottom
            { hw, -hh, -hd},
            { hw, -hh,  hd},
            {-hw, -hh,  hd},

            {hw, -hh,  hd}, // right
            {hw, -hh, -hd},
            {hw,  hh, -hd},
            {hw,  hh,  hd},

            {-hw, -hh, -hd}, // left
            {-hw, -hh,  hd},
            {-hw,  hh,  hd},
            {-hw,  hh, -hd}
        };

        const vec3 vertex_normals[] = {
            {0.0f, 0.0f, 1.0f}, // front
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},

            {0.0f, 1.0f, 0.0f}, // top
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},

            {0.0f, 0.0f, -1.0f}, // back
            {0.0f, 0.0f, -1.0f},
            {0.0f, 0.0f, -1.0f},
            {0.0f, 0.0f, -1.0f},

            {0.0f, -1.0f, 0.0f}, // top
            {0.0f, -1.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},

            {1.0f, 0.0f, 0.0f}, // right
            {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},

            {-1.0f, 0.0f, 0.0f}, // left
            {-1.0f, 0.0f, 0.0f},
            {-1.0f, 0.0f, 0.0f},
            {-1.0f, 0.0f, 0.0f},
        };

        memcpy(vertex_buffer.pos, vertex_positions, vert_size);
        memcpy(vertex_buffer.normal, vertex_normals, vert_size);
    }

    {
        const uint32_t indices_tmp[] = {
            0, 1, 2, // front
            0, 2, 3,

            4, 5, 6, // top
            4, 6, 7,

            8, 9, 10, // back
            8, 10, 11,

            12, 13, 14, // bottom
            12, 14, 15,

            16, 17, 18, // right
            16, 18, 19,
            
            20, 21, 22, // left
            20, 22, 23
        };

        memcpy(indices, indices_tmp, ind_size);
    }

    mesh_init(&self->meshes[0], arena, vertex_buffer, 24, indices, 6 * 6, tex_indices, self->material.tex_count);
}

void rectangular_plane_gen(Model* self, float width, float height, uint32_t res, const Material* material, uint32_t shader_idx)
{
    uint32_t* tex_indices = shape_setup(self, material, shader_idx);

    const size_t vert_size = res * res * sizeof(vec3);
    const size_t ind_size = 6 * (res - 1) * (res - 1) * sizeof(uint32_t);
    Arena arena = arena_create((2 * vert_size) + ind_size); // 2 vertex attributes

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(&arena, vert_size),
        .normal = (vec3*)arena_alloc(&arena, vert_size),
        .uv = NULL
    };
    ASSERT(vertex_buffer.pos != NULL && vertex_buffer.normal != NULL, "failed to allocate vertices\n");

    uint32_t* indices = (uint32_t*)arena_alloc(&arena, ind_size);
    ASSERT(indices != NULL, "failed to allocate indices\n");

    vec3 normal;
    normal.x = normal.z = 0.0f;
    normal.y = 1.0f;

    float inv_res = 1.0f / (float)(res - 1);
    float step_x = width * inv_res;
    float step_z = height * inv_res;

    float top_left_x = -0.5f * width;
    float top_left_z = -0.5f * height;
    for(uint32_t z = 0; z < res; z++)
    {
        float curr_z = top_left_z + ((float)z * step_z);

        for(uint32_t x = 0; x < res; x++)
        {
            uint32_t idx = z * res + x;

            vec3 curr;
            curr.x = top_left_x + ((float)x * step_x);
            curr.y = 0.0f;
            curr.z = curr_z;
            vertex_buffer.pos[idx] = curr;

            vertex_buffer.normal[idx] = normal;
        }
    }

    uint32_t idx = 0;
    for(uint32_t y = 0; y < res - 1; y++) // y as in the distance down
    {
        for(uint32_t x = 0; x < res - 1; x++)
        {
            // anti-clockwise indices
            indices[idx]     = (res * y)       + x + 1; // top right
            indices[idx + 1] = (res * y)       + x;     // top left
            indices[idx + 2] = (res * (y + 1)) + x;     // bottom left
            indices[idx + 3] = (res * y)       + x + 1; // top right
            indices[idx + 4] = (res * (y + 1)) + x;     // bottom left
            indices[idx + 5] = (res * (y + 1)) + x + 1; // bottom right

            idx += 6;
        }
    }

    mesh_init(&self->meshes[0], arena, vertex_buffer, res * res, indices, 6 * (res - 1) * (res - 1), tex_indices, self->material.tex_count);
}

inline uint32_t* shape_setup(Model* model, const Material* material, uint32_t shader_idx)
{
    model->meshes = (Mesh*)malloc(sizeof(Mesh));
    model->mesh_count = 1;
    model->shader_idx = shader_idx;
    model->material = *material;

    transform_reset(&model->transform);
    mat4_identity(&model->model);
    memset(model->directory, 0, MAX_PATH_LENGTH); // not necessary but feels good

    // allow for variable amount of textures
    // these will always be contiguous for shapes so can use loop
    uint32_t* tex_indices = (uint32_t*)malloc(material->tex_count * sizeof(uint32_t));
    for(uint32_t i = 0; i < material->tex_count; i++)
    {
        tex_indices[i] = i;
    }

    return tex_indices;
}