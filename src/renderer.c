#include "renderer.h"
#include <stdio.h>
#include <stdbool.h>

int renderer_init(Renderer* renderer)
{

}

int renderer_add_texture(Renderer* renderer)
{
    if((renderer->tex_count) >= MAX_TEXTURES)
    {
        fprintf(stderr, "ERR: could not add texture, max textures reached");
        return false;
    }
}

void renderer_free(Renderer* renderer)
{
    glDeleteVertexArrays(1, &(renderer->vao));
    glDeleteBuffers(1, &(renderer->vbo));
    glDeleteBuffers(1, &(renderer->ebo));
    glDeleteProgram(renderer->shader_program);
}