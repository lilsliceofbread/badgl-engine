#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* get_file_data(const char* filepath)
{
    FILE* file;
    file = fopen(filepath, "r");
    if(file == NULL || fseek(file, 0, SEEK_END)) // file open or fseek failed
    {
        return NULL;
    }

    // get file size
    long file_size = ftell(file);
    if(file_size == -1)
    {
        return NULL;
    }
    char* file_data = (char*)malloc((size_t)file_size + 1); // should be big enough
    
    fseek(file, 0, SEEK_SET); // i think since already checked fseek before should be fine
    if(file_data == NULL || fread(file_data, 1, (size_t)file_size, file) != (size_t)file_size)
    {
        free(file_data);
        return NULL;
    }
    file_data[file_size] = '\0';

    fclose(file);
    return file_data;
}

int str_find_last_of(const char* str, char c)
{
    int i = 0;
    int latest_occurrence = -1;
    const char* curr = str;
    while(curr != NULL && *curr != '\0')
    {
        if(*curr == c) {
            latest_occurrence = i;
        }
        ++curr;
        ++i;
    }

    return latest_occurrence;
}

// from learnopengl
GLenum _gl_check_error(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        const char* error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        fprintf(stderr, "ERR: OpenGL - %s | %s (%d)", error, file, line);
    }
    return errorCode;
}

// from learnopengl
void APIENTRY gl_debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *user_param)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    printf("GLDEBUG: ID - %d\nMessage: %s\n", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             printf("SOURCE: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("SOURCE: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("SOURCE: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("SOURCE: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     printf("SOURCE: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER:           printf("SOURCE: Other\n"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               printf("TYPE: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("TYPE: Deprecated Behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("TYPE: Undefined Behaviour\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         printf("TYPE: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         printf("TYPE: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              printf("TYPE: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          printf("TYPE: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           printf("TYPE: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               printf("TYPE: Other\n"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         printf("SEVERITY: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       printf("SEVERITY: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          printf("SEVERITY: low\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("SEVERITY: notification\n"); break;
    }
}