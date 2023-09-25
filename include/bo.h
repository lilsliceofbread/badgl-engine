#ifndef BO_H
#define BO_H

#include <glad/gl.h>
#include <stddef.h>

// forward declarations
#undef bool
#define bool _Bool

// buffer object
typedef struct BO
{
    GLuint id;
    GLenum type;
} BO;

//#define VBO BO
//#define EBO BO

BO bo_create(GLenum type);

void bo_bind(BO self);

void bo_set_buffer(BO self, void* data, size_t size, bool dynamic); // can't use dynamic rn

void bo_free(BO self); // free gpu allocation

#endif