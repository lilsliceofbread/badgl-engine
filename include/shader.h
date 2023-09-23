#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

int shader_create(int* shader, const char* shader_filepath, GLenum shader_type, char* info_log, int log_size);

void shader_program_create(int* shader_program_ptr, const char* vert_shader_src, const char* frag_shader_src);

#endif