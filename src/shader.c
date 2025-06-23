#include "shader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "defines.h"
#include "util.h"
#include "glmath.h"
#include "arena.h"

// TODO:
// check #include
// move into separate file
// think of more tests
// replace assert with BGL_LOG_ERROR -> return bool to allow program to continue running

/* for include directive */
#define MAX_SHADER_FILENAME 128 

typedef struct ShaderParser
{
    const char* code;
    const char* version_str;
    /* token span */
    u32 first;
    u32 last;
    enum 
    {
        TYPE_VERTEX = 0,
        TYPE_FRAGMENT = 1,
        TYPE_GEOMETRY = 2,
        TYPE_NONE, // shaders are in separate files
    } shader_type;

    u8* prev_ptr;
    u32 prev_alloc_size;
    u8* ptr;
    u32 prev_edit;
} ShaderParser;

/**
 * internal functions
 */
char* shader_process(Shader* self, ShaderParser* parser, Arena* scratch);
void shader_parser_skip_whitespace(ShaderParser* parser);
void shader_next_token(ShaderParser* parser);
bool shader_token_strequal(ShaderParser* parser, const char* string);
void shader_parser_alloc(ShaderParser* parser, Arena* scratch);

void shader_process_uniform(Shader* self, ShaderParser* parser);
void shader_process_type_directive(ShaderParser* parser);
void shader_process_version_directive(ShaderParser* parser);
void shader_process_include_directive(ShaderParser* parser, Arena* scratch);
void shader_add_version_directive(ShaderParser* parser, Arena* scratch);

u32 shader_compile(const char* shader_code, GLenum shader_type);

void shader_create(Shader* self, const char** shader_filepaths, u32 shader_count, const char* version_str)
{
    Arena scratch;
    ShaderParser parser;
    char* shader_code[3];
    char* processed_shader_code[3] = {0}; // must be set to NULL
    GLuint vert_shader, frag_shader, geom_shader, shader_program;

    BGL_ASSERT(shader_filepaths != NULL, "shader_filepaths is NULL");
    BGL_ASSERT(0 < shader_count && shader_count <= 3, "invalid amount of shaders %lu", shader_count);

    self->uniform_count = 0;
    self->uniforms = NULL;

    arena_create_sized(&scratch, MEGABYTES(1)); // surely big enough

    for(u32 i = 0; i < shader_count; i++)
    {
        const char* path = shader_filepaths[i];
        BGL_ASSERT(path != NULL, "shader filepath %lu is NULL", i + 1);
        const char* extension = str_find_last_of(path, '.');
        BGL_ASSERT(extension != NULL, "shader file has no extension");

        /* if extension is .glsl (all shaders in one file) also place in first index */
        if(strcmp(extension, ".vert") == 0 || strcmp(extension, ".glsl") == 0)
        {
            shader_code[0] = arena_read_file(&scratch, path, NULL);
        }
        else if(strcmp(extension, ".frag") == 0)
        {
            shader_code[1] = arena_read_file(&scratch, path, NULL);
        }
        else if(strcmp(extension, ".geom") == 0)
        {
            shader_code[2] = arena_read_file(&scratch, path, NULL);
        }
        else
        {
            BGL_ASSERT(0, "invalid shader file extension %s", extension);
        }
    }

    BGL_ASSERT(shader_code[0] != NULL, "missing shaders to create shader program");
    
    parser.first = parser.last = 0;
    parser.version_str = version_str;
    for(u32 i = 0; i < shader_count; i++)
    {
        parser.code = shader_code[i];
        char* code = shader_process(self, &parser, &scratch);

        /* deal with #type directives */
        if(parser.shader_type != TYPE_NONE)
        {
            BGL_ASSERT(shader_count == 1, "#type directive used even though multiple shader paths were input. #type can only be used if one shader file is used");

            BGL_ASSERT(processed_shader_code[parser.shader_type] == NULL, "second #type directive of same shader type");
            processed_shader_code[parser.shader_type] = code;

            if(parser.code[parser.last] != '\0') i--; // prevent from ending the loop if more of the file to come
            continue;
        }
    
        BGL_ASSERT(shader_count != 1, "one shader file provided, but no #type directive for multiple shaders in file");
        processed_shader_code[i] = code;
        parser.first = parser.last = 0;
    }

    BGL_ASSERT(processed_shader_code[0] != NULL, "missing vertex shader");
    BGL_ASSERT(processed_shader_code[1] != NULL, "missing fragment shader");

    ////BGL_LOG_INFO("vertex:\n%s", processed_shader_code[0]);
    ////BGL_LOG_INFO("fragment:\n%s", processed_shader_code[1]);

    vert_shader = shader_compile(processed_shader_code[0], GL_VERTEX_SHADER);

    frag_shader = shader_compile(processed_shader_code[1], GL_FRAGMENT_SHADER);

    shader_program = glCreateProgram();
    self->id = shader_program;

    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);

    if(shader_count == 3)
    {
        BGL_ASSERT(processed_shader_code[2] != NULL, "missing geometry shader");

        geom_shader = shader_compile(processed_shader_code[2], GL_GEOMETRY_SHADER);
        glAttachShader(shader_program, geom_shader);
    }
    else
    {
        if(processed_shader_code[2] != NULL) BGL_LOG_WARN("shader has geometry shader but shader count is <3, ignoring geometry shader");
    }

    glLinkProgram(shader_program);

    i32 linked;
    char info_log[512] = {0};
    glGetProgramiv(shader_program, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        BGL_ASSERT(false, "shader program creation failed. info log:\n%s", info_log);
    }

    /* get locations here because needs to be done after linking program */
    for(u32 i = 0; i < self->uniform_count; i++)
    {
        i32 location = glGetUniformLocation(self->id, self->uniforms[i].name);
        if(location == -1)
        {
            BGL_LOG_WARN("uniform %s was not given a location in program. name of one of the shader sources: %s", self->uniforms[i].name, shader_filepaths[0]);
        }
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

    arena_free(&scratch);
}

u32 shader_compile(const char* shader_code, GLenum shader_type)
{
    GLuint shader;
    char info_log[512] = {0};

    shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, (const char* const*)&shader_code, NULL); // must cast to pointer to const pointer to const char
    glCompileShader(shader);

    i32 compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        BGL_ASSERT(false, "shader code compilation failed. info log:\n%s", info_log);
    }

    return shader;
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

// TODO: rewrite to go line by line, simpler and same speed because arenas?
char* shader_process(Shader* self, ShaderParser* parser, Arena* scratch)
{
    char* processed_code = (char*)arena_alloc_unaligned(scratch, 0);
    bool processed_type_directive = false;
    parser->shader_type = TYPE_NONE;
    parser->prev_ptr = (u8*)processed_code;
    parser->prev_alloc_size = 0;
    parser->ptr = NULL;
    parser->prev_edit = parser->first;

    shader_add_version_directive(parser, scratch);

    shader_next_token(parser);
    parser->first--; // hack to prevent first character from being skipped
    while(parser->code[parser->last] != '\0')
    {
        if(shader_token_strequal(parser, "uniform"))
        {
            shader_process_uniform(self, parser);
        }
        else if(shader_token_strequal(parser, "#type"))
        {
            if(processed_type_directive)
            {
                /* hacky way of allocating 1 more character for \0 */
                parser->first++;
                shader_parser_alloc(parser, scratch);
                parser->first--;

                parser->ptr[parser->prev_alloc_size - 1] = '\0';

                parser->last = parser->first; // go to start of #type directive for next call
                return processed_code;
            }

            shader_parser_alloc(parser, scratch);
            shader_process_type_directive(parser);
            processed_type_directive = true;
        }
        else if(shader_token_strequal(parser, "#version"))
        {
            BGL_LOG_WARN("don't set version, the code will set it itself, deleting the line");
            shader_parser_alloc(parser, scratch);
            shader_process_version_directive(parser);
        }
        else if(shader_token_strequal(parser, "#include"))
        {
            // TODO: recursive include parsing
            shader_parser_alloc(parser, scratch);
            shader_process_include_directive(parser, scratch);
        }

        shader_next_token(parser);
    }

    /* since we alloc from prev_edit to first - 1, go one past \0 to include the \0 in processed_code */
    parser->first = parser->last + 1;
    shader_parser_alloc(parser, scratch);
    
    return processed_code;
}

void shader_parser_skip_whitespace(ShaderParser* parser)
{
    const char* p = parser->code;
    while(p[parser->first] == ' ' || p[parser->first] == '\t'
       || p[parser->first] == '\r' || p[parser->first] == '\n')
    {
        parser->first++;
    }
}

/* assumes tokens are separated by whitespace, which works for our purposes */
void shader_next_token(ShaderParser* parser)
{
    const char* p = parser->code;

    parser->first = parser->last + 1;

    shader_parser_skip_whitespace(parser);
    parser->last = parser->first;

    if(p[parser->first] == '\0')
    {
        return;
    }

    while(p[parser->last] != ' '  && p[parser->last] != '\t'
       && p[parser->last] != '\r' && p[parser->last] != '\n'
       )
    {
        parser->last++;
        if(p[parser->last] == '\0') return; // avoid last-- to keep it on the \0
    }
    parser->last--; // last increment was whitespace so go back
}

bool shader_token_strequal(ShaderParser* parser, const char* string)
{
    if(parser->first == parser->last)
        return false;

    for(u32 i = parser->first; i <= parser->last; i++)
    {
        if(parser->code[i] != string[i - parser->first]) // also handles if string is too short
            return false;
    }

    if(string[parser->last - parser->first + 1] != '\0') // string is too long
        return false;

    return true;
}

/* slightly hacky way of increasing size of string without reallocating using the arena */
void shader_parser_alloc(ShaderParser* parser, Arena* scratch)
{
    if(parser->first == 0) return; // no need to alloc since allocing all from before first

    u32 alloc_size = parser->first - parser->prev_edit; // alloc from prev_edit to before first char of token (-1 + 1)
    parser->ptr = arena_alloc_unaligned(scratch, alloc_size);
    BGL_ASSERT(parser->ptr == parser->prev_ptr + parser->prev_alloc_size, "new allocation for shader code is not contiguous with previous allocation");
    parser->prev_ptr = parser->ptr;
    parser->prev_alloc_size = alloc_size;

    memcpy(parser->ptr, parser->code + parser->prev_edit, alloc_size);
}

void shader_add_version_directive(ShaderParser* parser, Arena* scratch)
{
    u32 length = (u32)strlen(parser->version_str);

    parser->ptr = arena_alloc_unaligned(scratch, length + 1); 
    BGL_ASSERT(parser->ptr == parser->prev_ptr + parser->prev_alloc_size, "new allocation for shader code is not contiguous with previous allocation");
    parser->prev_ptr = parser->ptr;
    parser->prev_alloc_size = length + 1;

    memcpy(parser->ptr, parser->version_str, length);
    parser->ptr[length] = '\n';
}

void shader_process_uniform(Shader* self, ShaderParser* parser)
{
    Uniform uniform;

    shader_next_token(parser);
    // TODO: store type, use 32KB arena in Shader for allocation
    shader_next_token(parser);

    /* TODO: next part assumes immediate whitespace after semicolon, it may have a comment
     * after or another uniform with no newline in between which messes this up */
    u32 length = parser->last - parser->first;
    if(length >= MAX_UNIFORM_NAME - 1) return; // consider \0

    memcpy(uniform.name, parser->code + parser->first, length);
    uniform.name[length] = '\0';

    /* ignore duplicates from previously processed shaders */
    for(u32 i = 0; i < self->uniform_count; i++)
    {
        if(strcmp(uniform.name, self->uniforms[i].name) == 0)
            return;
    }

    BLOCK_RESIZE_ARRAY(&self->uniforms, Uniform, self->uniform_count, 1);
    self->uniforms[self->uniform_count++] = uniform;
}

void shader_process_type_directive(ShaderParser* parser)
{
    shader_next_token(parser);

    if(shader_token_strequal(parser, "vertex"))
    {
        parser->shader_type = TYPE_VERTEX;
    }
    else if(shader_token_strequal(parser, "fragment"))
    {
        parser->shader_type = TYPE_FRAGMENT;
    }
    else if(shader_token_strequal(parser, "geometry"))
    {
        parser->shader_type = TYPE_GEOMETRY;
    }
    else
    {
        BGL_ASSERT(false, "invalid #type directive");
    }

    parser->prev_edit = parser->last + 1; // skip this line
    parser->first = parser->last;
}

void shader_process_version_directive(ShaderParser* parser)
{
    /* assume directive ends at newline and nothing after, reasonable assumption */
    const char* p = parser->code;
    while(p[parser->last] != '\n') // skip to newline
    {
        BGL_ASSERT(p[parser->last] != '\0', "#version directive at end of file, why?");
        parser->last++;
    }

    parser->prev_edit = parser->last; // include newline
    parser->first = parser->last; // handle case where #version is at start of file (make first != 0 for shader_next_token)
}

void shader_process_include_directive(ShaderParser* parser, Arena* scratch)
{
    // TODO: recursive include parsing
    char filename[MAX_SHADER_FILENAME] = {0};
    u32 file_size;
    shader_next_token(parser);

    char first = parser->code[parser->first];
    char last = parser->code[parser->last];
    u32 length = parser->last - parser->first - 1; // - 2 + 1

    BGL_ASSERT(first == '\"' || first == '<', "#include directive missing first \" or <");
    BGL_ASSERT(last == '\"' || last == '>', "#include directive missing last \" or <");
    BGL_ASSERT(length <= MAX_SHADER_FILENAME - 1,
               "#include directive filename is too long: %lu > ", length, MAX_SHADER_FILENAME - 1); // consider \0

    memcpy(filename, parser->code + parser->first + 1, length);
    filename[length] = '\0';

    /* alloc file contiguous with previous stuff */
    parser->ptr = (u8*)arena_read_file(scratch, filename, &file_size);
    BGL_ASSERT(parser->ptr == parser->prev_ptr + parser->prev_alloc_size, "new allocation for shader code is not contiguous with previous allocation");
    parser->prev_ptr = parser->ptr;
    parser->prev_alloc_size = file_size; // NOTE: POTENTIAL POINT OF ERROR

    parser->prev_edit = parser->last + 1;
    parser->first = parser->last; // handle case where #include is at start of file (make first != 0 for shader_next_token)
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
