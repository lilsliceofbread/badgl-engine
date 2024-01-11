#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
    #include <windows.h>
#endif

void badgl_log_impl(LogType type, const char* msg, ...)
{
    va_list args;
    FILE* output_stream;
    const char* prefix;

    switch(type) 
    {
        case LOG_ERROR:
            output_stream = stderr;
            prefix = "ERROR";
            break; 
        case LOG_WARN:
            output_stream = stdout;
            prefix = "WARN";
            break; 
        case LOG_DEBUG:
            output_stream = stdout;
            prefix = "DEBUG";
            break; 
    }

    // set colour for prefix
    #ifdef __linux__
        fprintf(output_stream, "\x1B[%dm%s:\x1B[0m ", (int)type, prefix);
    #elif _WIN32
        CONSOLE_SCREEN_BUFFER_INFO cb_info;
        HANDLE console_handle = (type == LOG_ERROR)
                              ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(console_handle, &cb_info);
        int original_colour = cb_info.wAttributes;

        SetConsoleTextAttribute(console_handle, (WORD)type);
            fprintf(output_stream, "%s: ", prefix);
        SetConsoleTextAttribute(console_handle, (WORD)original_colour);
    #endif

    va_start(args, msg);
        vfprintf(output_stream, msg, args);
    va_end(args);
}

void badgl_log_ctx_impl(LogType type, const char* msg, const char* file, int line, ...)
{
    va_list args;
    char buffer[8192];

    // ? bit scuffed doing this twice but whatever
    va_start(args, line);
        vsprintf(buffer, msg, args);
    va_end(args);

    badgl_log_impl(type, "%s at %s in line %d\n\n", buffer, file, line);
}