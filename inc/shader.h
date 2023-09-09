#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

char* get_shader_source(const char* shader_filepath);

GLuint create_shader(const char* shader_filepath, GLenum shader_type, char* info_log, int log_size, int* success);