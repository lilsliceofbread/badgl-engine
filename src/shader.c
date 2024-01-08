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
    GLuint vert_shader, frag_shader, shader_program;
    int success;
    char info_log[512] = {0};

    self->uniform_count = 0;

    memset(self->stored_uniforms, 0, sizeof(self->stored_uniforms));

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
        int location = glGetUniformLocation(self->id, self->stored_uniforms[i].name);
        if(location == -1) fprintf(stderr, "SHADER: uniform %s was not given a location\n", self->stored_uniforms[i].name);
        self->stored_uniforms[i].location = location;
    }

    // can detach shaders as we don't need them anymore once program is linked
    glDetachShader(shader_program, vert_shader);
    glDetachShader(shader_program, frag_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
}

uint32_t shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, int log_size, int* success_out)
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
// unable to use uniform structs, arrays, or uniform buffers
void shader_find_uniforms_in_source(Shader* self, const char* src_code)
{
    uint32_t add_uniform_count = 0;
    const char* temp = src_code;
    Uniform new_uniforms[MAX_UNIFORMS] = {0};

    char name[MAX_UNIF_NAME];
    while((temp = strstr(temp, "uniform"))) // strstr returns NULL when no match, ending loop
    {
        temp++; // stop strstr finding the same match again
        char c;
        int name_idx = 0;
        bool skip_uniform = false;

        for(int i = 0; (c = temp[i]) != ';'; i++) // while we haven't seen ;
        {
            // can't use switch because break keyword applies to it, requiring another if anyway
            if(c == ' ') // reset name and continue (this is not the name)
            {
                name_idx = 0; // don't need to memset name since it will be null terminated later
                continue;
            }
            else if(c == '[' || c == '{') // ignore ubos or uniform arrays (hopefully)
            {
                skip_uniform = true;
                break;
            }

            name[name_idx++] = c;
        }
        
        if(skip_uniform) 
        {
            continue;
        }

        name[name_idx] = '\0';
        strncpy(new_uniforms[add_uniform_count++].name, name, MAX_UNIF_NAME); 
    }

    ASSERT(self->uniform_count + add_uniform_count <= MAX_UNIFORMS, "SHADER: too many uniforms for 1 shader object\n");

    //memcpy(&self->stored_uniforms[self->uniform_count], new_uniforms, add_uniform_count * sizeof(Uniform));
    for(uint32_t i = 0; i < add_uniform_count; i++)
    {
        strncpy(self->stored_uniforms[self->uniform_count + i].name, new_uniforms[i].name, MAX_UNIF_NAME);
    }

    self->uniform_count += add_uniform_count;
}


void shader_use(Shader* self)
{
    glUseProgram(self->id);
}

int shader_find_uniform(Shader* self, const char* name, FindUniformFunc func, void* arg)
{
    // temp system until i create a better way to detect uniforms
    if(func != NULL)
    {
        if(arg != NULL) return func(self, name, arg);

        return func(self, name); // if arg is null then the function does not take an extra argument
    }

    for(uint32_t i = 0; i < self->uniform_count; i++)
    {
        Uniform curr = self->stored_uniforms[i];
        if(strcmp(name, curr.name) == 0) return curr.location;
    }

    fprintf(stderr, "SHADER: uniform %s does not exist\n", name);
    return -1;
}

int shader_find_uniform_array(Shader* self, const char* name, void* arg)
{
    uint32_t index = *(uint32_t*)arg;

    size_t max_len = MAX_UNIF_NAME + 5; // allow for square brackets and 3 digits appended
    char indexed_name[max_len];

    snprintf(indexed_name, max_len, "%s[%u]", name, index);

    return glGetUniformLocation(self->id, indexed_name);
}

int shader_find_uniform_struct(Shader* self, const char* name, void* arg)
{
    const char* member = (const char*)arg;

    size_t max_len = 2 * MAX_UNIF_NAME + 1; // name + . + member
    char concat_name[max_len];

    snprintf(concat_name, max_len, "%s.%s", name, member);

    return glGetUniformLocation(self->id, concat_name);
}

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat, FindUniformFunc func, void* arg)
{
    GLint location = shader_find_uniform(self, name, func, arg);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)mat->data); // transposing matrix is false
}

void shader_uniform_vec4(Shader* self, const char* name, vec4* vec, FindUniformFunc func, void* arg)
{
    GLint location = shader_find_uniform(self, name, func, arg);
    glUniform4fv(location, 1, (float*)vec->data);
}

void shader_uniform_vec3(Shader* self, const char* name, vec3* vec, FindUniformFunc func, void* arg)
{
    GLint location = shader_find_uniform(self, name, func, arg);
    glUniform3fv(location, 1, (float*)vec->data);
}

void shader_uniform_1f(Shader* self, const char* name, float f, FindUniformFunc func, void* arg)
{
    GLint location = shader_find_uniform(self, name, func, arg);
    glUniform1f(location, f);
}

void shader_uniform_1i(Shader* self, const char* name, int i, FindUniformFunc func, void* arg)
{
    GLint location = shader_find_uniform(self, name, func, arg);
    glUniform1i(location, i);
}

void shader_free(Shader* self)
{
    glDeleteProgram(self->id);
}