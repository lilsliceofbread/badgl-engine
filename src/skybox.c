#include "skybox.h"

#include <stdlib.h>
#include <string.h>
#include "shapes.h"

void skybox_create(Model* self, Arena* scratch, Renderer* rd, const char* cubemap_path)
{
    Material material;
    vec3 white = VEC3(1.0f, 1.0f, 1.0f); // don't care about colour values
    material_create(&material, true, white, white, white, 1.0f);
    material_add_texture(&material, TEXTURE_DIFFUSE, cubemap_path);
    material.flags |= BGL_MATERIAL_NO_LIGHTING;

    shapes_box(self, scratch, 2.0f, 2.0f, 2.0f, &material, rd->skybox_shader);
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
    shader_uniform_mat4(shader, "mvp", &vp);

    mesh_draw(&self->meshes[0], shader, self->material.textures);
    glCullFace(GL_BACK);
}

void skybox_free(Model* self)
{
    model_free(self);
}
