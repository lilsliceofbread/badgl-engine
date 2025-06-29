#ifndef BGL_BO_H
#define BGL_BO_H

#include <glad/glad.h>
#include <stddef.h>
#include "defines.h"

typedef struct BO
{
    u32 id;
    GLenum type;
} BO;

#define VBO BO
#define vbo_create() bo_create(GL_ARRAY_BUFFER)
#define vbo_bind bo_bind
#define vbo_unbind bo_unbind
#define vbo_set_buffer bo_set_buffer
#define vbo_set_buffer_region bo_set_buffer_region
#define vbo_free bo_free

#define EBO BO
#define ebo_create() bo_create(GL_ELEMENT_ARRAY_BUFFER)
#define ebo_bind bo_bind
#define ebo_unbind bo_unbind
#define ebo_set_buffer bo_set_buffer
#define ebo_set_buffer_region bo_set_buffer_region
#define ebo_free bo_free

#define UBO BO
#define ubo_create() bo_create(GL_UNIFORM_BUFFER)
#define ubo_bind bo_bind
#define ubo_unbind bo_unbind
#define ubo_set_buffer bo_set_buffer
#define ubo_set_buffer_region bo_set_buffer_region
#define ubo_free bo_free


BO bo_create(GLenum type);

void bo_bind(BO self);

void bo_unbind(BO self);

void bo_set_buffer(BO self, const void* data, size_t size, bool dynamic);

void bo_set_buffer_region(BO self, const void* data, i32 offset, size_t size);

void bo_free(BO self); // free gpu allocation

/* UBO functions */
void ubo_bind_buffer_base(UBO self, u32 index);

void ubo_bind_buffer_range(UBO self, u32 index, i32 offset, size_t size);

#endif
