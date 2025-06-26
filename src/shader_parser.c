#include "shader_parser.h"
#include "util.h"

// TODO: test geometry shader

/* for #include directive */
#define MAX_SHADER_FILEPATH 512 

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

bool parser_alloc(ShaderParser* parser, Arena* scratch, u64 size);
bool add_version_directive(ShaderParser* parser, Arena* scratch);
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

void process_uniform(ShaderParser* parser, Shader* shader)
{
    Uniform uniform;

    next_token(parser);
    // TODO: store type, use 32KB arena in Shader for allocation?
    next_token(parser);

    /* next part assumes immediate whitespace after semicolon, it may have a comment
     * after or another uniform with no newline in between which messes this up */
    u64 length = parser->last - parser->first;
    if(length >= MAX_UNIFORM_NAME - 1) return; // consider \0

    memcpy(uniform.name, parser->code + parser->first, length);
    uniform.name[length] = '\0';

    /* ignore duplicates from previously processed shaders */
    for(u32 i = 0; i < shader->uniform_count; i++)
    {
        if(strcmp(uniform.name, shader->uniforms[i].name) == 0)
            return;
    }

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
