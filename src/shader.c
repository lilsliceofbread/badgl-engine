#include "shader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "defines.h"
#include "util.h"
#include "glmath.h"
#include "defines.h"

/**
 * internal functions
 */
u32 shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, i32 log_size, int* success_out);
void shader_find_uniforms_in_source(Shader* self, const char* src_code); // UNIFORMS ARE REPEATED IF IN VERT + FRAG SHADER
void shader_reallocate_uniforms(Shader* self, u32 new_count);

void shader_create(Shader* self, const char* vert_shader_src, const char* frag_shader_src)
{
    GLuint vert_shader, frag_shader, shader_program;
    i32 success;
    char info_log[512] = {0};

    self->uniform_count = 0;
    self->uniforms = NULL;

    vert_shader = shader_compile(self, vert_shader_src, GL_VERTEX_SHADER, info_log, sizeof(info_log), &success);
    BGL_ASSERT(success, "vertex shader comp failed. Info Log:\n%s\n", info_log);

    frag_shader = shader_compile(self, frag_shader_src, GL_FRAGMENT_SHADER, info_log, sizeof(info_log), &success);
    BGL_ASSERT(success, "fragment shader comp failed. info log:\n%s\n", info_log);

    shader_program = glCreateProgram();
    self->id = shader_program;

    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
    BGL_ASSERT(success, "shader program creation failed. info log:\n%s\n", info_log);

    // here because needs to be done after linking program
    for(u32 i = 0; i < self->uniform_count; i++)
    {
        i32 location = glGetUniformLocation(self->id, self->uniforms[i].name);
        if(location == -1)
        {
            BGL_LOG(LOG_WARN, "uniform %s was not given a location\n", self->uniforms[i].name);
        }
        self->uniforms[i].location = location;
    }

    // can detach shaders as we don't need them anymore once program is linked
    glDetachShader(shader_program, vert_shader);
    glDetachShader(shader_program, frag_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
}

u32 shader_compile(Shader* self, const char* shader_filepath, GLenum shader_type, char* info_log_out, i32 log_size, int* success_out)
{
    char* shader_src;
    GLuint shader;
    *success_out = false;

    shader_src = get_file_data(shader_filepath);

    BGL_ASSERT(shader_src != NULL, "failed to get shader source\n");

    shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, (const char* const*)&shader_src, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);
    shader_find_uniforms_in_source(self, shader_src);
    free(shader_src);

    i32 comp_success;
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
    u32 new_uniform_count = 0;
    const char* temp = src_code;
    Uniform new_uniforms[16] = {0}; // this definitely won't become a problem in future ;)

    char name[MAX_UNIFORM_NAME];
    while((temp = strstr(temp, "uniform"))) // strstr returns NULL when no match, ending loop
    {
        temp++; // stop strstr finding the same match again
        char c;
        i32 name_idx = 0;
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
        strncpy(new_uniforms[new_uniform_count++].name, name, MAX_UNIFORM_NAME); 
    }

    shader_reallocate_uniforms(self, self->uniform_count + new_uniform_count);

    for(u32 i = 0; i < new_uniform_count; i++)
    {
        strncpy(self->uniforms[self->uniform_count + i].name, new_uniforms[i].name, MAX_UNIFORM_NAME);
    }

    self->uniform_count += new_uniform_count;
}

void shader_reallocate_uniforms(Shader* self, u32 new_count)
{
    u32 uniform_array_size = ALIGNED_SIZE(self->uniform_count, SHADER_UNIFORM_ALLOC_SIZE);
    if(new_count > uniform_array_size)
    {
        u32 new_array_size = ALIGNED_SIZE(new_count, SHADER_UNIFORM_ALLOC_SIZE);

        self->uniforms = (Uniform*)realloc(self->uniforms, new_array_size * sizeof(Uniform));
        BGL_ASSERT(self->uniforms != NULL, "uniform cache reallocation failed");
        ////BGL_LOG(LOG_DEBUG, "uniform cache resize from %u to %u\n", uniform_array_size, new_array_size);
    }
}

void shader_use(Shader* self)
{
    glUseProgram(self->id);
}

int shader_find_uniform(Shader* self, const char* name)
{
    for(u32 i = 0; i < self->uniform_count; i++)
    {
        Uniform curr = self->uniforms[i];
        if(strcmp(name, curr.name) == 0) return curr.location;
    }

    ////BGL_LOG(LOG_DEBUG, "uniform %s not found. Caching...\n", name);
    i32 location = glGetUniformLocation(self->id, name);

    shader_reallocate_uniforms(self, self->uniform_count + 1);
    strncpy(self->uniforms[self->uniform_count].name, name, MAX_UNIFORM_NAME);
    self->uniforms[self->uniform_count].location = location;
    self->uniform_count++;

    return location;
}

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat)
{
    i32 location = shader_find_uniform(self, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)mat->data); // * transposing matrix is false
}

void shader_uniform_vec4(Shader* self, const char* name, vec4* vec)
{
    i32 location = shader_find_uniform(self, name);
    glUniform4fv(location, 1, (float*)vec->data);
}

void shader_uniform_vec3(Shader* self, const char* name, vec3* vec)
{
    i32 location = shader_find_uniform(self, name);
    glUniform3fv(location, 1, (float*)vec->data);
}

void shader_uniform_vec2(Shader* self, const char* name, vec2* vec)
{
    i32 location = shader_find_uniform(self, name);
    glUniform2fv(location, 1, (float*)vec->data);
}

void shader_uniform_float(Shader* self, const char* name, float f)
{
    i32 location = shader_find_uniform(self, name);
    glUniform1f(location, f);
}

void shader_uniform_int(Shader* self, const char* name, i32 i)
{
    i32 location = shader_find_uniform(self, name);
    glUniform1i(location, i);
}

void shader_free(Shader* self)
{
    glDeleteProgram(self->id);
    free(self->uniforms);
}