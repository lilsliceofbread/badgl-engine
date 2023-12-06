#include "shader.h"
#include "util.h"
#include "glmath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

void shader_init(Shader* self, const char* vert_shader_src, const char* frag_shader_src)
{
    self->uniform_count = 0;
    self->stored_uniforms = NULL;
    GLuint vert_shader, frag_shader, shader_program;
    int success;
    char info_log[512];
    info_log[0] = '\0'; // if shader creation failed but no info log

    vert_shader = shader_compile(self, vert_shader_src, GL_VERTEX_SHADER, info_log, sizeof(info_log), &success);
    ASSERT(success, "SHADER: vertex shader comp failed. Info Log:\n%s\n", info_log);

    frag_shader = shader_compile(self, frag_shader_src, GL_FRAGMENT_SHADER, info_log, sizeof(info_log), &success);
    ASSERT(success, "SHADER: fragment shader comp failed. info log:\n%s\n", info_log);

    shader_program = glCreateProgram();

    self->id = shader_program;

    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
    ASSERT(success, "SHADER: shader program creation failed. info log:\n%s\n", info_log);

    // here because needs to be done after linking program
    for(uint32_t i = 0; i < self->uniform_count; i++)
    {
        // glGetUniformLocation returns -1 if no uniform
        self->stored_uniforms[i].location = glGetUniformLocation(self->id, self->stored_uniforms[i].name);
        //printf("LOG: uniform %d: %s\n", self->stored_uniforms[i].location, self->stored_uniforms[i].name);
    }

    // can detach shaders as we don't need them anymore once program is linked
    glDetachShader(shader_program, vert_shader);
    glDetachShader(shader_program, frag_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
}

GLuint shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, int log_size, int* success_out)
{
    char* shader_src;
    GLuint shader;
    *success_out = false;

    shader_src = get_file_data(shader_filepath);

    ASSERT(shader_src != NULL, "SHADER: failed to get shader source\n");

    shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, (const char* const*)&shader_src, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);
    shader_find_uniforms_in_source(self, shader_src);
    free(shader_src);

    int comp_success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &comp_success);
    if(!comp_success)
    {
        glGetShaderInfoLog(shader, log_size, NULL, info_log_out);
        return 0;
    }

    *success_out = true;
    return shader;
}

// THIS WILL REPEAT UNIFORMS IF IN BOTH SHADERS
void shader_find_uniforms_in_source(Shader* self, const char* src_code)
{
    uint32_t add_uniform_count = 0;
    uint32_t uniform_pos[100]; // 100 is probably enough
    const char* temp = src_code;

    // find location of word uniform in shader code
    while((temp = strstr(temp, "uniform"))) // strstr returns NULL when no match, ending loop
    {
        uniform_pos[add_uniform_count] = (uint32_t)(temp - src_code);
        temp++; // to prevent strstr from finding the same match again
        add_uniform_count++;
    }

    self->stored_uniforms = realloc(self->stored_uniforms, (self->uniform_count + add_uniform_count) * sizeof(uniform_pair));
    ASSERT(self->stored_uniforms != NULL, "SHADER: failed to reallocate uniforms array\n");

    // get name of uniform from it 
    // (find ; and the space behind it)
    for(uint32_t i = 0; i < add_uniform_count; i++)
    {
        char line[100]; // 100 should be fine

        char c;
        int line_idx = 0;
        uint32_t j = uniform_pos[i];
        while((c = src_code[j]) != ';' && line_idx < 300) // while we haven't seen ; and less than 300 for sanity check (line shouldn't be that long)
        {
            if(c == ' ') // if space reset line and continue
            {
                line_idx = 0;
                j++;
            }
            else // else continue adding to line
            {
                line[line_idx] = c;
                line_idx++;
                j++;
            }
        }
        ASSERT(line_idx < 300, "SHADER: failed to find uniform name. Missing semicolon in shader?\n");
        line[line_idx] = '\0'; // have to manually null terminate string

        strncpy(self->stored_uniforms[self->uniform_count + i].name, line, MAX_UNIF_NAME); 
    }

    self->uniform_count += add_uniform_count;
}

void shader_use(Shader* self)
{
    glUseProgram(self->id);
}

GLint shader_find_uniform(Shader* self, const char* name)
{
    // find uniforms that have been pre stored by analysing shader src
    uniform_pair curr;
    for(uint32_t i = 0; i < self->uniform_count; i++)
    {
        curr = self->stored_uniforms[i];
        if(strcmp(name, curr.name) == 0)
        {
            /*if(curr.location == -1)
                fprintf(stderr, "SHADER: uniform %s does not exist. forgot to use in shader?\n", curr.name);*/
            return curr.location;  
        }
    }

    fprintf(stderr, "SHADER: uniform %s has not been stored\n", name);
    return -1;
}

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat)
{
    GLint location = shader_find_uniform(self, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)mat->data); // false to transpose matrix
}

void shader_uniform_1f(Shader* self, const char* name, float f)
{
    GLint location = shader_find_uniform(self, name);
    glUniform1f(location, f);
}

void shader_uniform_1i(Shader* self, const char* name, int i)
{
    GLint location = shader_find_uniform(self, name);
    glUniform1i(location, i);
}

void shader_free(Shader* self)
{
    glDeleteProgram(self->id);
    free(self->stored_uniforms);
    self->uniform_count = 0;
}