#include "skybox.h"
#include "shapes.h"
#include <stdlib.h>
#include <string.h>

Model skybox_init(Renderer* rd, const char* cubemap_path)
{
    Material material = material_texture_diffuse(true, cubemap_path, (vec3){1.0f, 1.0f, 1.0f}, 1.0f); // dummy values
    material.flags |= NO_LIGHTING;

    Model self = rectangular_prism_gen(2.0f, 2.0f, 2.0f, &material, rd->skybox_shader);
    return self;
}

void skybox_draw(Model* self, Renderer* rd, Camera* cam)
{
    Shader* shader = &rd->shaders[self->shader_idx];

    mat4 vp; // no translation allowed to keep skybox at consistent distance
    mat4 corrected_view = cam->view;
    corrected_view.m14 = corrected_view.m24 = corrected_view.m34 = 0.0f; // remove translation
    mat4_mul(&vp, cam->proj, corrected_view);

    glCullFace(GL_FRONT); // since we are inside the box
    shader_use(shader);
    shader_uniform_mat4(shader, "mvp", &vp, NULL, NULL);
    mesh_draw(&self->meshes[0], shader, self->material.textures);
    glCullFace(GL_BACK);
}

void skybox_free(Model* self)
{
    model_free(self);
}