#ifndef BADGL_BO_H
#define BADGL_BO_H

#include <glad/gl.h>
#include <stddef.h>
#include <stdbool.h>

// buffer object
typedef struct BO
{
    GLuint id;
    GLenum type;
} BO;

// this might be bad code practice but it looks nice
#define VBO BO
#define vbo_create() bo_create(GL_ARRAY_BUFFER)
#define vbo_bind bo_bind
#define vbo_set_buffer bo_set_buffer
#define vbo_set_buffer_region bo_set_buffer_region
#define vbo_free bo_free

#define EBO BO
#define ebo_create() bo_create(GL_ELEMENT_ARRAY_BUFFER)
#define ebo_bind bo_bind
#define ebo_set_buffer bo_set_buffer
#define ebo_set_buffer_region bo_set_buffer_region
#define ebo_free bo_free


BO bo_create(GLenum type);

void bo_bind(BO self);

void bo_set_buffer(BO self, const void* data, size_t size, bool dynamic); // can't use dynamic rn

void bo_set_buffer_region(BO self, const void* data, int offset, size_t size);

void bo_free(BO self); // free gpu allocation

#endif