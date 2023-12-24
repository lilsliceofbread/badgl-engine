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
        self->stored_uniforms[i].location = glGetUniformLocation(self->id, self->stored_uniforms[i].name);
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

// this will repeat uniforms if in both shaders
// also doesn't allow use of uniform buffer objects
void shader_find_uniforms_in_source(Shader* self, const char* src_code)
{
    uint32_t add_uniform_count = 0;
    const char* temp = src_code;
    uniform_pair new_uniforms[MAX_UNIFORMS] = {0};

    char name[MAX_UNIF_NAME];
    while((temp = strstr(temp, "uniform"))) // strstr returns NULL when no match, ending loop
    {
        char c;
        int name_idx = 0;
        bool is_array_uniform = false;

        for(int j = 0; (c = temp[j]) != ';'; j++) // while we haven't seen ;
        {
            // can't use switch because break keyword applies to it, requiring another if anyway
            if(c == ' ') // reset name and continue (this is not the name)
            {
                name_idx = 0; // don't need to memset name since it will be null terminated later
                continue;
            }
            else if(c == '[') // because array uniform names don't fit the pattern, must deal with them in another way
            {
                is_array_uniform = true;
                break;
            }

            name[name_idx++] = c;
        }

        if(is_array_uniform) 
        {
            // should call another function to deal with this
            // and user will have to use shader_find_uniform_array
            continue;
        }

        name[name_idx] = '\0';
        strncpy(new_uniforms[add_uniform_count++].name, name, MAX_UNIF_NAME); 

        temp++; // stop strstr finding the same match again
    }

    if(self->uniform_count + add_uniform_count > MAX_UNIFORMS)
    {
        ASSERT(false, "SHADER: too many uniforms for 1 shader object");
        return;
    }

    memcpy(&self->stored_uniforms[self->uniform_count], new_uniforms, add_uniform_count * sizeof(uniform_pair));

    self->uniform_count += add_uniform_count;
}

void shader_use(Shader* self)
{
    glUseProgram(self->id);
}

GLint shader_find_uniform(Shader* self, const char* name)
{
    uniform_pair curr;
    for(uint32_t i = 0; i < self->uniform_count; i++)
    {
        curr = self->stored_uniforms[i];
        if(strcmp(name, curr.name) == 0)
        {
            if(curr.location == -1)
                fprintf(stderr, "SHADER: uniform %s does not exist. forgot to use in shader?\n", curr.name);

            return curr.location;  
        }
    }

    fprintf(stderr, "SHADER: uniform %s has not been stored\n", name);
    return -1;
}

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat)
{
    GLint location = shader_find_uniform(self, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)mat->data); // transposing matrix is false
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
    self->uniform_count = 0;
}