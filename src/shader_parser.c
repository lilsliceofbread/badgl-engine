#include "shader_parser.h"
#include "util.h"
#include "defines.glsl"

// TODO: test geometry shader

/* for #include directive */
#define PARSER_ALLOC_SIZE(parser) ((parser)->first - (parser)->prev_edit) // alloc from prev_edit to before first char of token (-1 + 1)

#define PARSER_ASSERT(cond, msg, ...) \
{                                          \
    if(!(cond))                            \
    {                                      \
        BGL_LOG_ERROR(msg, ##__VA_ARGS__); \
        return false;                      \
    }                                      \
}

#define PARSER_CHECK(cond) if(!(cond)) return NULL

/**
 * internal functions
 * naming conventions have left the building
 */
void skip_whitespace(ShaderParser* parser);
void next_token(ShaderParser* parser);
bool token_strequal(ShaderParser* parser, const char* string);
bool token_add_uniform_name(ShaderParser* parser, Uniform* uniform);

bool parser_alloc(ShaderParser* parser, Arena* scratch, u64 size);
bool add_version_directive(ShaderParser* parser, Arena* scratch);
bool process_binding(ShaderParser* parser);
void process_uniform(ShaderParser* parser, Shader* shader);
bool process_type_directive(ShaderParser* parser);
bool process_version_directive(ShaderParser* parser);
bool process_include_directive(ShaderParser* parser, Arena* scratch);

char* shader_process(ShaderParser* parser, Shader* shader, Arena* scratch)
{
    char* processed_code = (char*)arena_alloc_unaligned(scratch, 0);
    bool processed_type_directive = false;
    parser->shader_type = TYPE_NONE;
    parser->ptr = NULL;
    parser->prev_ptr = (u8*)processed_code;
    parser->prev_alloc_size = 0;
    parser->prev_edit = parser->first;

    PARSER_CHECK(add_version_directive(parser, scratch));

    next_token(parser);
    parser->first--; // hack to prevent first character from being skipped
    while(parser->code[parser->last] != '\0')
    {
        if(token_strequal(parser, "uniform"))
        {
            process_uniform(parser, shader);
        }
        else if(parser->no_uniform_bindings && token_strequal(parser, "(binding"))
        {
            PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser)));
            PARSER_CHECK(process_binding(parser));
        }
        else if(parser->no_uniform_bindings && token_strequal(parser, "binding")) // handle when layout specified before binding
        {
            const char* p = parser->code;
            /* make sure not to alloc the comma */
            while(p[parser->first] != ',')
            {
                PARSER_CHECK(p[parser->first] != '\n' && parser->first > 1); // missing comma and bracket
                parser->first--;
                if(p[parser->first] == '(')
                {
                    parser->first++;
                    break;
                }
            }
            PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser)));
            PARSER_CHECK(process_binding(parser));
        }
        else if(token_strequal(parser, "#type"))
        {
            if(processed_type_directive)
            {
                PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser) + 1)); // +1 for null terminator
                parser->ptr[parser->prev_alloc_size - 1] = '\0';

                parser->last = parser->first; // go to start of #type directive for next call
                return processed_code;
            }

            PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser)));
            PARSER_CHECK(process_type_directive(parser));
            processed_type_directive = true;
        }
        else if(token_strequal(parser, "#version"))
        {
            BGL_LOG_WARN("don't set #version, the code will set it itself, deleting the line");
            PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser)));
            PARSER_CHECK(process_version_directive(parser));
        }
        else if(token_strequal(parser, "#include"))
        {
            PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser)));
            PARSER_CHECK(process_include_directive(parser, scratch));
        }

        next_token(parser);
    }

    parser->first = parser->last;
    PARSER_CHECK(parser_alloc(parser, scratch, PARSER_ALLOC_SIZE(parser) + 1)); // +1 for null terminator
    
    return processed_code;
}

void skip_whitespace(ShaderParser* parser)
{
    const char* p = parser->code;
    while(p[parser->first] == ' ' || p[parser->first] == '\t'
       || p[parser->first] == '\r' || p[parser->first] == '\n')
    {
        parser->first++;
    }
}

/* assumes tokens are separated by whitespace, which works for our purposes */
void next_token(ShaderParser* parser)
{
    const char* p = parser->code;

    parser->first = parser->last + 1;

    skip_whitespace(parser);
    parser->last = parser->first;

    if(p[parser->first] == '\0') return;

    while(p[parser->last] != ' '  && p[parser->last] != '\t'
       && p[parser->last] != '\r' && p[parser->last] != '\n'
       )
    {
        parser->last++;
        if(p[parser->last] == '\0') return; // avoid last-- to keep it on the \0
    }
    parser->last--; // last increment was whitespace so go back
}

bool token_strequal(ShaderParser* parser, const char* string)
{
    if(parser->first == parser->last)
        return false;

    for(u64 i = parser->first; i <= parser->last; i++)
    {
        if(parser->code[i] != string[i - parser->first]) // also handles if string is too short
            return false;
    }

    if(string[parser->last - parser->first + 1] != '\0') // string is too long
        return false;

    return true;
}

/* slightly hacky way of increasing size of string without reallocating using the arena */
bool parser_alloc(ShaderParser* parser, Arena* scratch, u64 size)
{
    if(parser->first == 0) return true; // no need to alloc since allocing all from before first

    parser->ptr = arena_alloc_unaligned(scratch, size);
    PARSER_ASSERT(parser->ptr == parser->prev_ptr + parser->prev_alloc_size, "new allocation for shader code is not contiguous with previous allocation");
    parser->prev_ptr = parser->ptr;
    parser->prev_alloc_size = size;

    memcpy(parser->ptr, parser->code + parser->prev_edit, size);
    return true;
}

bool add_version_directive(ShaderParser* parser, Arena* scratch)
{
    u64 length = (u64)strlen(parser->version_str);

    parser->ptr = arena_alloc_unaligned(scratch, length + 1); 
    PARSER_ASSERT(parser->ptr == parser->prev_ptr + parser->prev_alloc_size, "new allocation for shader code is not contiguous with previous allocation");
    parser->prev_ptr = parser->ptr;
    parser->prev_alloc_size = length + 1;

    memcpy(parser->ptr, parser->version_str, length);
    parser->ptr[length] = '\n';
    return true;
}

bool token_add_uniform_name(ShaderParser* parser, Uniform* uniform)
{
    u64 length = parser->last - parser->first;
    if(length >= MAX_UNIFORM_NAME - 1) // consider \0
    {
        BGL_LOG_INFO("length of uniform name too long to add to cache");
        return false;
    }

    memcpy(uniform->name, parser->code + parser->first, length);
    uniform->name[length] = '\0';

    return true;
}

bool process_binding(ShaderParser* parser)
{
    const char* p = parser->code;
    parser->first = parser->last;

    while(!CHAR_IS_NUMBER(p[parser->first]))
    {
        parser->first++;
        PARSER_ASSERT(p[parser->first] != '\0', "reached end of file while processing binding. missing binding number");
    }
    PARSER_ASSERT(!CHAR_IS_NUMBER(p[parser->first + 1]), "this engine does not support more than 10 uniform block bindings");
    parser->ubos[parser->ubo_count].binding = (u32)CHAR_TO_INT(p[parser->first]);

    while(p[parser->first] != ')')
    {
        parser->first++;
        PARSER_ASSERT(p[parser->first] != '\n' && p[parser->first] != '\0', "uniform block binding does not have closing bracket");
    }
    parser->prev_edit = parser->first; // start allocation after "binding = x"

    do
    {
        next_token(parser);
        PARSER_ASSERT(p[parser->last] != '\0', "block binding does not have keyword uniform after");
    }
    while(!token_strequal(parser, "uniform"));
    next_token(parser);

    u64 length = parser->last - parser->first + 1;
    PARSER_ASSERT(length < MAX_UBO_NAME - 1, "length of uniform name exceeded MAX_UBO_NAME in shader_parser.h, increase value to 64?"); // consider \0
                                                                                                                                
    memcpy(parser->ubos[parser->ubo_count].name, parser->code + parser->first, length);
    parser->ubos[parser->ubo_count].name[length] = '\0';
    parser->ubo_count++;

    return true;
}

/* assumes immediate whitespace after semicolon, it may have a comment
 * after or another uniform with no newline in between which messes this up */
void process_uniform(ShaderParser* parser, Shader* shader)
{
    Uniform uniform;

    next_token(parser);
    if(token_strequal(parser, "sampler2D") || token_strequal(parser, "samplerCube")) // TODO: add more 
    {
        next_token(parser);
        // TODO: remove texture macro system?
        parser->last--; // remove semicolon
        if(token_strequal(parser, MACRO_TO_MACRO_NAME(BGL_GLSL_TEXTURE_DIFFUSE)))
        {
            strncpy(uniform.name, MACRO_TO_STR(BGL_GLSL_TEXTURE_DIFFUSE), MAX_UNIFORM_NAME);
        }
        else if(token_strequal(parser, MACRO_TO_MACRO_NAME(BGL_GLSL_TEXTURE_SPECULAR)))
        {
            strncpy(uniform.name, MACRO_TO_STR(BGL_GLSL_TEXTURE_SPECULAR), MAX_UNIFORM_NAME);
        }
        else
        {
            parser->last++;
            if(!token_add_uniform_name(parser, &uniform)) return; // parser name too long - don't fail
            BGL_LOG_INFO("non-standard name for sampler uniform: %s", uniform.name);
            parser->last--;
        }
        parser->last++; // add back semicolon
    }
    else if(token_strequal(parser, "Material") || token_strequal(parser, "DirLight")) // struct uniforms are too complicated
    {
        parser->last++;
        return;
    }
    else
    {
        next_token(parser);
        if(parser->code[parser->first] == '{') return; // ubo - don't bother with it now will be cached later
        if(!token_add_uniform_name(parser, &uniform)) return; // parser name too long - don't fail
    }

    /* ignore duplicates from previously processed shaders */
    for(u32 i = 0; i < shader->uniform_count; i++)
    {
        if(strcmp(uniform.name, shader->uniforms[i].name) == 0) return;
    }
    if(strcmp(uniform.name, "") == 0) BGL_LOG_ERROR("%s", parser->code + parser->first);

    BLOCK_RESIZE_ARRAY(&shader->uniforms, Uniform, shader->uniform_count, 1);
    shader->uniforms[shader->uniform_count++] = uniform;
}

bool process_type_directive(ShaderParser* parser)
{
    next_token(parser);

    if(token_strequal(parser, "vertex"))
    {
        parser->shader_type = TYPE_VERTEX;
    }
    else if(token_strequal(parser, "fragment"))
    {
        parser->shader_type = TYPE_FRAGMENT;
    }
    else if(token_strequal(parser, "geometry"))
    {
        parser->shader_type = TYPE_GEOMETRY;
    }
    else
    {
        PARSER_ASSERT(false, "invalid #type directive");
    }

    parser->prev_edit = parser->last + 1; // skip this line
    parser->first = parser->last;
    return true;
}

bool process_version_directive(ShaderParser* parser)
{
    /* assume directive ends at newline and nothing after, reasonable assumption */
    const char* p = parser->code;
    while(p[parser->last] != '\n') // skip to newline
    {
        PARSER_ASSERT(p[parser->last] != '\0', "#version directive at end of file, why?");
        parser->last++;
    }

    parser->prev_edit = parser->last; // include newline
    parser->first = parser->last; // handle case where #version is at start of file (make first != 0 for next_token)
    return true;
}

bool process_include_directive(ShaderParser* parser, Arena* scratch)
{
    // TODO: recursive include parsing
    char filepath[MAX_SHADER_FILEPATH] = {0};
    find_directory_from_path(filepath, MAX_SHADER_FILEPATH, parser->path); // specify includes relative to path
    u64 dir_length = (u64)strlen(filepath);
    PARSER_ASSERT(dir_length < MAX_SHADER_FILEPATH - 1, // consider \0
               "directory for shader files in  #include directive is too long. to fix, increase MAX_SHADER_FILEPATH in shader_parser.c");
    filepath[dir_length] = PLATFORM_FILE_SEPARATOR;

    next_token(parser);

    char first = parser->code[parser->first];
    char last = parser->code[parser->last];
    u64 name_length = parser->last - parser->first - 1; // - 2 + 1
    u64 length = dir_length + name_length + 1; // added slash

    PARSER_ASSERT((first == '\"' && last == '\"') || (first == '<' && last == '>'),
               "#include directive missing one or both quotes or braces (\"\" or <>)");
    PARSER_ASSERT(length <= MAX_SHADER_FILEPATH - 1, // consider \0
               "#include directive filepath is too long. to fix, increase MAX_SHADER_FILEPATH in shader_parser.c");

    memcpy(filepath + dir_length + 1, parser->code + parser->first + 1, name_length);
    filepath[length] = '\0';

    /* alloc file contiguous with previous stuff */
    u64 file_size;
    parser->ptr = (u8*)arena_read_file_unterminated_unaligned(scratch, filepath, &file_size);

    PARSER_ASSERT(parser->ptr != NULL, "could not open #include file %s. is the path correct?", filepath);

    PARSER_ASSERT(parser->ptr == parser->prev_ptr + parser->prev_alloc_size, "new allocation for shader code is not contiguous with previous allocation");
    
    parser->prev_ptr = parser->ptr;
    parser->prev_alloc_size = file_size;
    parser->prev_edit = ++parser->last; // past last > or "
    parser->first = parser->last; // handle case where #include is at start of file (make first != 0 for next_token)
    return true;
}
