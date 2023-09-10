#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

int create_shader(GLuint* shader, const char* shader_filepath, GLenum shader_type, char* info_log, int log_size);

int create_shader_program(GLuint* shader_program_ptr, const char* vert_shader_src, const char* frag_shader_src);