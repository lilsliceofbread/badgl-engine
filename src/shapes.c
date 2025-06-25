#include "shapes.h"

#include <stdlib.h>
#include "defines.h"
#include "util.h"
#include "arena.h"

extern inline u32* shape_setup(Model* model, const Material* material, u32 shader_idx);

void shapes_uv_sphere(Model* self, Arena* scratch, u32 res, const Material* material, u32 shader_idx)
{
    BGL_ASSERT(res >= 2, "%u too small of a resolution for uv sphere", res);

    u32* tex_indices = shape_setup(self, material, shader_idx);

    u32 vert_count = 0, ind_count = 0;

    const u32 horizontals = res, verticals = 2 * res;
    const u32 total_vertices = horizontals * verticals + 2;
    const u32 total_indices = 6 * verticals * (horizontals - 1); // 6 indices per square, but top and bottom rings are triangles (3 per), so h - 2 + 1

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(scratch, total_vertices * sizeof(vec3)),
        .normal = NULL,
        .uv = NULL
    };
    u32* indices = (u32*)arena_alloc(scratch, total_indices * sizeof(u32));

    // top vertex
    vertex_buffer.pos[0].x = 0.0f;
    vertex_buffer.pos[0].y = 1.0f;
    vertex_buffer.pos[0].z = 0.0f;

    vert_count++;

    const f32 inv_hori = 1.0f / (f32)horizontals;
    const f32 inv_vert = 1.0f / (f32)verticals;
    const f32 pi2 = 2.0f * GL_PI; // multiply 2 * PI before hand
    for(u32 i = 0; i < horizontals; i++) // latitudes (start after top vertex (i+1) and stop before bottom vertex (< not <=))
    {
        // only 1PI because horizontal stacks will only need to sweep half the circle (other half is just the reverse)
        const f32 horizontal_angle = (f32)(i + 1) * GL_PI * inv_hori;

        const f32 radius = sinf(horizontal_angle);
        const f32 y = cosf(horizontal_angle); // same for whole horizontal

        for(u32 j = 0; j < verticals; j++) // longitudes (< to stop before 2PI which is a repeat of 0)
        {
            /**
             * 2PI because it's going around the full circumference.
             * since opengl is right-handed this creates vertices clockwise
             * as the unit circle is reversed with +z going "down"
             */
            f32 vertical_angle = (f32)j * pi2 * inv_vert;

            vec3 pos;
            // calculate cartesian coords from spherical
            pos.x = radius * cosf(vertical_angle);
            pos.y = y;
            pos.z = radius * sinf(vertical_angle);

            vertex_buffer.pos[vert_count] = pos;
            vert_count++;
        }
    }
    // this creates (horizontals - 1) horizontals as the loop goes from 1 to horizontals

    // bottom vertex
    vertex_buffer.pos[vert_count].x = 0.0f;
    vertex_buffer.pos[vert_count].y = -1.0f;
    vertex_buffer.pos[vert_count].z = 0.0f;

    vert_count++;

    // top triangle's indices
    for(u32 i = 0; i < verticals; i++)
    {
        u32 p1 = (i) + 1; // add 1 because 0 is top vertex
        u32 p2 = ((i + 1) % verticals) + 1; // i + 2 but remember to wrap around back to 0 instead of going above verticals

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
    for(u32 i = 0; i < horizontals - 2; i++)
    {
        u32 layer1 = i * verticals + 1; // add 1 because of top vertex and then go down horizontals by multiplying by num verticals
        u32 layer2 = (i + 1) * verticals + 1; // i + 1 gives next layer down
        
        for(u32 j = 0; j < verticals; j++)
        {
            u32 tr = layer1 + j; // j: from 0 to verticals - 1
            u32 tl = layer1 + (j + 1) % verticals; // j + 1 but remembering to wrap back to 0
            u32 br = layer2 + j;
            u32 bl = layer2 + (j + 1) % verticals;

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
    const u32 final_layer_index = verticals * (horizontals - 2) + 1; // add 1 because of top vertex
    for(u32 i = 0; i < verticals; i++)
    {
        u32 p1 = final_layer_index + i; // first vertex
        u32 p2 = final_layer_index + ((i + 1) % verticals); // next vertex, remembering to wrap back to 0 on last

        indices[ind_count] = vert_count - 1; // first vertex is bottom vertex
        indices[ind_count + 1] = p1;
        indices[ind_count + 2] = p2;

        ind_count += 3;
    }

    mesh_create(&self->meshes[0], vertex_buffer, vert_count, indices, ind_count,
                tex_indices, material == NULL ? 0 : self->material.tex_count);

    arena_collapse(scratch, (u8*)vertex_buffer.pos); // reset arena to before allocation
}

void shapes_box(Model* self, Arena* scratch, f32 width, f32 height, f32 depth, const Material* material, u32 shader_idx)
{
    u32* tex_indices = shape_setup(self, material, shader_idx);
    
    const u32 vert_count = 6 * 4;
    const u32 ind_count = 6 * 6;

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(scratch, vert_count * sizeof(vec3)),
        .normal = (vec3*)arena_alloc(scratch, vert_count * sizeof(vec3)),
        .uv = NULL
    };

    {
        // half width, height, depth
        const f32 hw = 0.5f * width;
        const f32 hh = 0.5f * height;
        const f32 hd = 0.5f * depth;

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

            {0.0f, -1.0f, 0.0f}, // bottom
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

        memcpy(vertex_buffer.pos, vertex_positions, vert_count * sizeof(vec3));
        memcpy(vertex_buffer.normal, vertex_normals, vert_count * sizeof(vec3));
    }

    const u32 indices[] = {
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

    mesh_create(&self->meshes[0], vertex_buffer, vert_count, indices, ind_count,
                tex_indices, material == NULL ? 0 : self->material.tex_count);

    arena_collapse(scratch, (u8*)vertex_buffer.pos);
}

void shapes_plane(Model* self, Arena* scratch, f32 width, f32 height, u32 res, const Material* material, u32 shader_idx)
{
    BGL_ASSERT(res >= 2, "%u too small of a resolution for rectangular plane", res);

    u32* tex_indices = shape_setup(self, material, shader_idx);

    const u32 vert_count = res * res;
    const u32 vert_size = vert_count * sizeof(vec3);
    const u32 ind_count = 6 * (res - 1) * (res - 1);
    const u32 ind_size = ind_count * sizeof(u32);

    VertexBuffer vertex_buffer = {
        .pos = (vec3*)arena_alloc(scratch, vert_size),
        .normal = (vec3*)arena_alloc(scratch, vert_size),
        .uv = NULL
    };
    u32* indices = (u32*)arena_alloc(scratch, ind_size);

    vec3 normal;
    normal.x = normal.z = 0.0f;
    normal.y = 1.0f;

    f32 inv_res = 1.0f / (f32)(res - 1);
    f32 step_x = width * inv_res;
    f32 step_z = height * inv_res;

    f32 top_left_x = -0.5f * width;
    f32 top_left_z = -0.5f * height;
    for(u32 z = 0; z < res; z++)
    {
        f32 curr_z = top_left_z + ((f32)z * step_z);

        for(u32 x = 0; x < res; x++)
        {
            u32 idx = z * res + x;

            vec3 pos = VEC3(
                top_left_x + ((f32)x * step_x),
                0.0f,
                curr_z
            );
            vertex_buffer.pos[idx] = pos;

            vertex_buffer.normal[idx] = normal;
        }
    }

    u32 idx = 0;
    for(u32 y = 0; y < res - 1; y++) // y as in the distance down
    {
        for(u32 x = 0; x < res - 1; x++)
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

    mesh_create(&self->meshes[0], vertex_buffer, vert_count, indices, ind_count,
                tex_indices, material == NULL ? 0 : self->material.tex_count);

    arena_collapse(scratch, (u8*)vertex_buffer.pos);
}

inline u32* shape_setup(Model* model, const Material* material, u32 shader_idx)
{
    model->meshes = (Mesh*)BGL_MALLOC(sizeof(Mesh));
    model->mesh_count = 1;
    model->shader_idx = shader_idx;
    if(material == NULL)
    {
        memset(&model->material, 0, sizeof(Material));
    }
    else
    {
        model->material = *material;
    }

    transform_reset(&model->transform);
    mat4_identity(&model->model);
    memset(model->directory, 0, MAX_PATH_LENGTH); // not necessary but feels good

    // allow for variable amount of textures
    // these will always be contiguous for shapes so can use loop
    u32* tex_indices = (u32*)BGL_MALLOC(model->material.tex_count * sizeof(u32));
    for(u32 i = 0; i < model->material.tex_count; i++)
    {
        tex_indices[i] = i;
    }

    return tex_indices;
}
