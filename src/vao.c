#include "vao.h"

VAO vao_create(void)
{
    VAO self;
    glGenVertexArrays(1, &self.id);
    return self;
}

void vao_bind(VAO self)
{
    glBindVertexArray(self.id);
}

void vao_unbind(void)
{
    glBindVertexArray(0);
}

void vao_attribute(GLuint index, GLint size, GLenum type, GLsizei stride, size_t offset)
{
    // fix when integer attribute is integer (glVertexAttribIPointer)
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset);
    glEnableVertexAttribArray(index);
}

void vao_free(VAO self)
{
    glDeleteVertexArrays(1, &self.id);
}