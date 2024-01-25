#include "bo.h"

#include <stdio.h>
#include "defines.h"

BO bo_create(GLenum type)
{
    BO self;
    self.type = type;
    glGenBuffers(1, &self.id);
    return self;
}

void bo_bind(BO self)
{
    glBindBuffer(self.type, self.id);
}

void bo_unbind(BO self)
{
    glBindBuffer(self.type, 0);
}

void bo_set_buffer(BO self, const void* data, size_t size, bool dynamic)
{
    GLenum usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(self.type, (GLsizeiptr)size, data, usage);
}

void bo_set_buffer_region(BO self, const void* data, i32 offset, size_t size)
{
    glBufferSubData(self.type, (GLintptr)offset, (GLsizeiptr)size, data);
}

void bo_free(BO self)
{
    glDeleteBuffers(1, &self.id);
}

void ubo_bind_buffer_base(UBO self, u32 index)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, self.id);
}

void ubo_bind_buffer_range(UBO self, u32 index, i32 offset, size_t size)
{
    glBindBufferRange(GL_UNIFORM_BUFFER, index, self.id, offset, (GLsizeiptr)size);
}