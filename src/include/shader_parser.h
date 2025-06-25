#ifndef BGL_SHADER_PARSER_H
#define BGL_SHADER_PARSER_H

#include "defines.h"
#include "shader.h"
#include "arena.h"

/* shader processer
 * this is complicated because c does not have a string object which can be manipulated
 * the string must be copied and the edits added in contiguously
 * this parser/processor makes one pass through the file token by token
 * each 'edit' that needs to be made incurs that the previous code is allocated and copied, and then the edit
 * this repeats until the end of the file is reached and the rest of the code is copied */

typedef struct ShaderParser
{
    const char* code;
    const char* path; // for resolving includes
    const char* version_str;

    /* for allocating sections of the shader as we pass through the file */
    u8* ptr;
    u8* prev_ptr;
    u64 prev_alloc_size;
    u64 prev_edit;

    /* token span */
    u64 first;
    u64 last;
    enum 
    {
        TYPE_VERTEX = 0,
        TYPE_FRAGMENT = 1,
        TYPE_GEOMETRY = 2,
        TYPE_NONE, // shaders are in separate files
    } shader_type;
} ShaderParser;

char* shader_process(ShaderParser* parser, Shader* shader, Arena* scratch);

#endif
