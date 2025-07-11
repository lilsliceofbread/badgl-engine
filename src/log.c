#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include "types.h"

#define MAX_PREFIX_BUF_SIZE 64

void log_impl(LogType type, const char* filename, i32 line, const char* msg, ...)
{
    va_list args;
    FILE* output_stream;
    const char* prefix;

    switch(type) 
    {
        case BGL_LOG_ERROR:
            output_stream = stderr;
            prefix = "ERROR";
            break; 
        case BGL_LOG_WARN:
            output_stream = stderr;
            prefix = "WARN";
            break; 
        case BGL_LOG_INFO:
            output_stream = stdout;
            prefix = "INFO";
            break; 
        default:
            output_stream = stdout;
            prefix = "UNKNOWN_LOG_TYPE";
    }

    char prefix_buf[MAX_PREFIX_BUF_SIZE];
    if(filename != NULL) 
    {
        snprintf(prefix_buf, MAX_PREFIX_BUF_SIZE, "%s[%s:%d] ", prefix, filename, line);
    }
    else
    {
        snprintf(prefix_buf, MAX_PREFIX_BUF_SIZE, "%s[] ", prefix);
    }

    platform_print_coloured(output_stream, prefix_buf, (BGLColour)type);

    va_start(args, msg);
        vfprintf(output_stream, msg, args);
    va_end(args);
    fprintf(output_stream, "\n");
}
