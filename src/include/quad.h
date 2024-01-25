#ifndef BGL_QUAD_H
#define BGL_QUAD_H

#include "shader.h"
#include "vao.h"
#include "bo.h"
#include "texture.h"
#include "renderer.h"

// flat 2d quads
typedef struct Quad {
    VAO vao;
    VBO vbo;
    EBO ebo;
    Texture texture;
} Quad;

// pos is bottom left of quad
Quad quad_create(vec2 pos, vec2 size, const char* texture_path);

void quad_draw(Quad* self, Renderer* rd);

void quad_free(Quad* self);

#endif