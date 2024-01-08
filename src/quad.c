#include "quad.h"

Quad quad_init(vec2 pos, vec2 size, const char* texture_path)
{
    Quad self;

    self.vao = vao_create();
    self.vbo = vbo_create();
    self.ebo = ebo_create();

    // pos is bottom left
    float vertices[] = {
        pos.x,          pos.y,          0.0f,   0.0f, 0.0f,
        pos.x,          pos.y + size.y, 0.0f,   0.0f, 1.0f,
        pos.x + size.x, pos.y + size.y, 0.0f,   1.0f, 1.0f,
        pos.x + size.x, pos.y,          0.0f,   1.0f, 0.0f,
    };

    uint32_t indices[] = {
        2, 1, 0,
        3, 2, 0 
    };

    vao_bind(self.vao);

    vbo_bind(self.vbo);
    vbo_set_buffer(self.vbo, vertices, (size_t)4 * 5 * sizeof(float), false);

    ebo_bind(self.ebo);
    ebo_set_buffer(self.ebo, indices, (size_t)6 * sizeof(uint32_t), false);

    vao_attribute(0, 3, GL_FLOAT, 5 * sizeof(float), 0); // vertex position
    vao_attribute(1, 2, GL_FLOAT, 5 * sizeof(float), 3 * sizeof(float)); // vertex uv

    vao_unbind();

    if(texture_path != NULL) texture_create(&self.texture, TEXTURE_DIFFUSE, texture_path, false); // type not really necessary

    return self;
}

void quad_draw(Quad* self, Renderer* rd)
{
    shader_use(&rd->shaders[rd->quad_shader]);

    texture_bind(&self->texture);

    glDisable(GL_DEPTH_TEST);
    vao_bind(self->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    vao_unbind();
    glEnable(GL_DEPTH_TEST);
}

void quad_free(Quad* self)
{
    vao_free(self->vao);
    vbo_free(self->vbo);
    ebo_free(self->ebo);
    texture_free(&self->texture);
}