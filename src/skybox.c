#include "skybox.h"
#include "shapes.h"
#include <stdlib.h>
#include <string.h>

Model skybox_init(const char* cubemap_path)
{
    Model self = rectangular_prism_gen(1.0f, 1.0f, 1.0f, cubemap_path, 0); // since not using shader_index, put any value

    return self;
}

void skybox_draw(Model* self, Renderer* rd, Camera* cam)
{
    Shader* shader = &rd->skybox_shader;

    mat4 vp; // no translation allowed to keep skybox at consistent distance
    mat4 corrected_view = cam->view;
    corrected_view.m14 = corrected_view.m24 = corrected_view.m34 = 0.0f; // remove translation
    mat4_mul(&vp, cam->proj, corrected_view);

    shader_use(shader);
    shader_uniform_mat4(shader, "vp", &vp);
    mesh_draw(&self->meshes[0], shader, self->textures);
}

void skybox_free(Model* self)
{
    model_free(self);
}