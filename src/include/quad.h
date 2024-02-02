#ifndef BGL_QUAD_H
#define BGL_QUAD_H

#include "shader.h"
#include "vao.h"
#include "bo.h"
#include "texture.h"
#include "renderer.h"

// flat 2d quads
// ! hacky system, will improve at some point
typedef struct Quad {
    VAO vao;
    VBO vbo;
    EBO ebo;
    Texture texture;
} Quad;

/**
 * @note coordinates within (-1, 1) screen space
 * @param  pos: position of top left point
 * @param  size: width and height
 * @param  texture_path: path to texture
 * @returns Quad
 */
Quad quad_create(vec2 pos, vec2 size, const char* texture_path);

void quad_draw(Quad* self, Renderer* rd);

void quad_free(Quad* self);

#endif