#include "shader.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

int shader_compile(GLuint* shader_id, const char* shader_filepath, GLenum shader_type, char* info_log, int log_size)
{
    char* shader_src;
    GLuint shader;

    shader_src = get_file_data(shader_filepath);

    ASSERT(shader_src != NULL, "ERR: failed to get shader source");

    shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, (const char* const*)&shader_src, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);
    free(shader_src);

    int comp_success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &comp_success);
    if(!comp_success)
    {
        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        return false;
    }

    *shader_id = shader;
}

void shader_init(Shader* self, const char* vert_shader_src, const char* frag_shader_src)
{
    self->uniform_count = 0;
    GLuint vert_shader, frag_shader, shader_program;
    int success;
    char info_log[512];
    info_log[0] = '\0'; // if shader creation failed but no info log

    success = shader_compile(&vert_shader, vert_shader_src, GL_VERTEX_SHADER, info_log, sizeof(info_log));
    ASSERT(success, "ERR: vertex shader comp failed. Info Log:\n%s", info_log);

    success = shader_compile(&frag_shader, frag_shader_src, GL_FRAGMENT_SHADER, info_log, sizeof(info_log));
    ASSERT(success, "ERR: fragment shader comp failed. info log:\n%s", info_log);

    shader_program = glCreateProgram();

    self->id = shader_program;

    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
    ASSERT(success, "ERR: shader program creation failed. info log:\n%s", info_log);

    glDetachShader(shader_program, vert_shader);
    glDetachShader(shader_program, frag_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
}

void shader_use(Shader* self)
{
    glUseProgram(self->id);
}

GLuint shader_find_uniform(Shader* self, const char* name)
{
    // find uniform if already used before
    int unif_count = self->uniform_count;
    uniform_pair curr;
    for(int i = 0; i < unif_count; i++)
    {
        curr = self->stored_uniforms[i];
        if(strcmp(name, curr.name) == 0) // if uniform used before
        {
            return curr.location;  
        }
    }

    // if haven't used uniform before store
    GLuint location = glGetUniformLocation(self->id, name);
    if(unif_count < MAX_UNIFORMS) // don't add past array end
    {
        self->stored_uniforms[unif_count].name = name;
        self->stored_uniforms[unif_count].location = location;
        unif_count++;
    }
    return location;
}

void shader_uniform_mat4(Shader* self, const char* name, mat4 mat)
{
    GLuint location = shader_find_uniform(self, name);
    glUniformMatrix4fv(location, 1, false, (float*)mat);
}

void shader_uniform_1f(Shader* self, const char* name, float f)
{
    GLuint location = shader_find_uniform(self, name);
    glUniform1f(location, f);
}

void shader_uniform_1i(Shader* self, const char* name, int i)
{
    GLuint location = shader_find_uniform(self, name);
    glUniform1i(location, i);
}

void shader_free(Shader* self)
{
    glDeleteProgram(self->id);
    memset(self->stored_uniforms, 0, self->uniform_count); //reset values so they can't be used
    self->uniform_count = 0;
}