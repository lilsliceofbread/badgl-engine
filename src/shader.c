#include "shader.h"

#include <glad/glad.h>
#include <string.h>
#include "defines.h"
#include "shader_parser.h"
#include "util.h"
#include "glmath.h"
#include "arena.h"

#define INFO_LOG_SIZE 512 

#define CREATION_ASSERT(cond, msg, ...) \
{                                                \
    if(!(cond))                                  \
    {                                            \
        BGL_LOG_ERROR(msg, ##__VA_ARGS__);       \
        arena_collapse(scratch, arena_init_pos); \
        return false;                            \
    }                                            \
}

/**
 * internal functions
 */
bool shader_compile(const char* shader_code, GLenum shader_type, u32* shader_out);

bool shader_create(Shader* self, Arena* scratch, const char** shader_filepaths, u32 shader_count, const char* version_str)
{
    BGL_PERFORMANCE_START();

    ShaderParser parser;
    char* shader_code[3];
    char* processed_shader_code[3] = {0}; // must be set to NULL
    GLuint vert_shader, frag_shader, geom_shader, shader_program;
    u8* arena_init_pos = arena_alloc(scratch, 0);

    CREATION_ASSERT(shader_filepaths != NULL, "shader_filepaths is NULL");
    CREATION_ASSERT(0 < shader_count && shader_count <= 3, "invalid amount of shaders %lu", shader_count);

    self->uniform_count = 0;
    self->uniforms = NULL;

    for(u32 i = 0; i < shader_count; i++)
    {
        const char* path = shader_filepaths[i];
        CREATION_ASSERT(path != NULL, "shader filepath %lu is NULL", i + 1);
        const char* extension = str_find_last_of(path, '.');
        CREATION_ASSERT(extension != NULL, "shader file has no extension");

        /* if extension is .glsl (all shaders in one file) also place in first index */
        if(strcmp(extension, ".vert") == 0 || strcmp(extension, ".glsl") == 0)
        {
            shader_code[0] = arena_read_file(scratch, path, NULL);
        }
        else if(strcmp(extension, ".frag") == 0)
        {
            shader_code[1] = arena_read_file(scratch, path, NULL);
        }
        else if(strcmp(extension, ".geom") == 0)
        {
            shader_code[2] = arena_read_file(scratch, path, NULL);
        }
        else
        {
            CREATION_ASSERT(0, "invalid shader file extension %s", extension);
        }
    }

    CREATION_ASSERT(shader_code[0] != NULL, "missing shaders to create shader program");
    
    parser.first = parser.last = 0;
    parser.version_str = version_str;
    for(u32 i = 0; i < shader_count; i++)
    {
        parser.path = shader_filepaths[i];
        parser.code = shader_code[i];
        char* code = shader_process(&parser, self, scratch);
        CREATION_ASSERT(code != NULL, "shader program not created");

        /* deal with #type directives */
        if(parser.shader_type != TYPE_NONE)
        {
            CREATION_ASSERT(shader_count == 1, "#type directive used even though multiple shader paths were input. #type can only be used if one shader file is used");

            CREATION_ASSERT(processed_shader_code[parser.shader_type] == NULL, "second #type directive of same shader type");
            processed_shader_code[parser.shader_type] = code;

            if(parser.code[parser.last] != '\0') i--; // prevent from ending the loop if more of the file to come
            continue;
        }
    
        CREATION_ASSERT(shader_count != 1, "one shader file provided, but no #type directive for multiple shaders in file");
        processed_shader_code[i] = code;
        parser.first = parser.last = 0;
    }

    CREATION_ASSERT(processed_shader_code[0] != NULL, "missing vertex shader");
    CREATION_ASSERT(processed_shader_code[1] != NULL, "missing fragment shader");
    CREATION_ASSERT(shader_compile(processed_shader_code[0], GL_VERTEX_SHADER, &vert_shader), "shader program not created");
    CREATION_ASSERT(shader_compile(processed_shader_code[1], GL_FRAGMENT_SHADER, &frag_shader), "shader program not created");

    shader_program = glCreateProgram();
    self->id = shader_program;
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);

    if(shader_count == 3)
    {
        CREATION_ASSERT(processed_shader_code[2] != NULL, "missing geometry shader");

        CREATION_ASSERT(shader_compile(processed_shader_code[2], GL_GEOMETRY_SHADER, &geom_shader), "shader program not created");
        glAttachShader(shader_program, geom_shader);
    }
    else
    {
        if(processed_shader_code[2] != NULL) BGL_LOG_WARN("geometry shader given but shader count is <3, ignoring geometry shader");
    }

    glLinkProgram(shader_program);

    i32 linked;
    char info_log[INFO_LOG_SIZE] = {0};
    glGetProgramiv(shader_program, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        glDeleteProgram(shader_program);
        CREATION_ASSERT(false, "shader program creation failed. info log:\n%s", info_log);
    }

    /* get locations here because needs to be done after linking program */
    for(u32 i = 0; i < self->uniform_count; i++)
    {
        i32 location = glGetUniformLocation(self->id, self->uniforms[i].name);
        /*if(location == -1)
        {
            BGL_LOG_WARN("uniform %s was not given a location in program. name of one of the shader sources: %s", self->uniforms[i].name, shader_filepaths[0]);
        }*/
        self->uniforms[i].location = location;
    }

    glDetachShader(shader_program, vert_shader);
    glDetachShader(shader_program, frag_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    if(shader_count == 3)
    {
        glDetachShader(shader_program, geom_shader);
        glDeleteShader(geom_shader);
    }

    arena_collapse(scratch, arena_init_pos);
    BGL_PERFORMANCE_END("shader program creation");
    return true;
}

bool shader_compile(const char* shader_code, GLenum shader_type, u32* shader_out)
{
    GLuint shader;

    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char* const*)&shader_code, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);

    i32 compiled;
    char info_log[INFO_LOG_SIZE] = {0};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
        glDeleteShader(shader);
        BGL_LOG_ERROR("shader code compilation failed. info log:\n%s", info_log);
        return false;
    }

    *shader_out = (u32)shader;
    return true;
}

i32 shader_find_uniform(Shader* self, const char* name)
{
    BGL_ASSERT(name != NULL, "uniform name cannot be NULL");

    for(u32 i = 0; i < self->uniform_count; i++)
    {
        Uniform curr = self->uniforms[i];
        if(strcmp(name, curr.name) == 0) return curr.location;
    }

    BGL_LOG_INFO("uniform %s not found. Caching...", name);
    i32 location = glGetUniformLocation(self->id, name);

    /* just in case there is a name longer, still allow to work but don't cache */
    if(strlen(name) <= MAX_UNIFORM_NAME)
    {
        BLOCK_RESIZE_ARRAY(&self->uniforms, Uniform, self->uniform_count, 1);
        strncpy(self->uniforms[self->uniform_count].name, name, MAX_UNIFORM_NAME);
        self->uniforms[self->uniform_count].location = location;
        self->uniform_count++;
    }

    return location;
}

void shader_use(Shader* self)
{
    glUseProgram(self->id);
}

void shader_free(Shader* self)
{
    glDeleteProgram(self->id);
    BGL_FREE(self->uniforms);
}

void shader_uniform_mat4(Shader* self, const char* name, mat4* mat)
{
    i32 location = shader_find_uniform(self, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (f32*)mat->data); // transposing matrix is false
}

void shader_uniform_vec4(Shader* self, const char* name, vec4* vec)
{
    i32 location = shader_find_uniform(self, name);
    glUniform4fv(location, 1, (f32*)vec->data);
}

void shader_uniform_vec3(Shader* self, const char* name, vec3* vec)
{
    i32 location = shader_find_uniform(self, name);
    glUniform3fv(location, 1, (f32*)vec->data);
}

void shader_uniform_vec2(Shader* self, const char* name, vec2* vec)
{
    i32 location = shader_find_uniform(self, name);
    glUniform2fv(location, 1, (f32*)vec->data);
}

void shader_uniform_f32(Shader* self, const char* name, f32 f)
{
    i32 location = shader_find_uniform(self, name);
    glUniform1f(location, f);
}

void shader_uniform_int(Shader* self, const char* name, i32 i)
{
    i32 location = shader_find_uniform(self, name);
    glUniform1i(location, i);
}
