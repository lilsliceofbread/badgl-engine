#ifndef BADGL_VAO_H
#define BADGL_VAO_H

#include <glad/gl.h>
#include <stddef.h>

typedef struct VAO
{
    GLuint id;
} VAO;

VAO vao_create();

void vao_bind(VAO self);

void vao_unbind();

void vao_attribute(GLuint index, GLint size, GLenum type, GLsizei stride, size_t offset);

void vao_free(VAO self); // free gpu allocation

#endif