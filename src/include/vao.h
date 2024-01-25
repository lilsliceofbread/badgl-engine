#ifndef BGL_VAO_H
#define BGL_VAO_H

#include <glad/gl.h>
#include <stddef.h>
#include "defines.h"

typedef struct VAO
{
    u32 id;
} VAO;

VAO vao_create(void);

void vao_bind(VAO self);

void vao_unbind(void);

void vao_attribute(GLuint index, GLint size, GLenum type, GLsizei stride, size_t offset);

void vao_free(VAO self); // free gpu allocation

#endif