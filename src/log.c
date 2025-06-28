#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
    #include <windows.h>
#endif
#include "types.h"

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

    /* set colour */
    #if defined(__linux__)
    if(filename != NULL) 
    {
        fprintf(output_stream, "\x1B[%dm%s[%s:%d]\x1B[0m ", (i32)type, prefix, filename, line);
    }
    else
    {
        fprintf(output_stream, "\x1B[%dm%s[]\x1B[0m ", (i32)type, prefix);
    }
    #elif defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO cb_info;
    HANDLE console_handle = (output_stream == stderr)
                          ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console_handle, &cb_info);
    i32 original_colour = cb_info.wAttributes;

    SetConsoleTextAttribute(console_handle, (WORD)type);
        if(filename != NULL) 
        {
            fprintf(output_stream, "%s[%s:%d] ", prefix, filename, line);
        }
        else
        {
            fprintf(output_stream, "%s[] ", prefix);
        }
    SetConsoleTextAttribute(console_handle, (WORD)original_colour);
    #endif

    va_start(args, msg);
        vfprintf(output_stream, msg, args);
    va_end(args);
    fprintf(output_stream, "\n");
}
