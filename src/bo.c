#include "bo.h"
#include <stdbool.h>

BO bo_create(GLenum type)
{
    BO self;
    self.type = type;
    glGenBuffers(1, &(self.id));
    return self;
}

void bo_bind(BO self)
{
    glBindBuffer(self.type, self.id);
}

void bo_set_buffer(BO self, void* data, size_t size, bool dynamic)
{
    GLenum usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW; // if dynamic glBufferSubData?
    glBufferData(self.type, size, data, usage);
}

void bo_free(BO self)
{
    glDeleteBuffers(1, &(self.id));
}