#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
    #include <windows.h>
#endif
#include "util.h"
#include "types.h"

void bgl_log_impl(LogType type, const char* msg, ...)
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
        case LOG_INFO:
            output_stream = stdout;
            prefix = "INFO";
            break; 
    }

    // set colour for prefix
    #ifdef __linux__
        fprintf(output_stream, "\x1B[%dm%s:\x1B[0m ", (i32)type, prefix);
    #elif _WIN32
        CONSOLE_SCREEN_BUFFER_INFO cb_info;
        HANDLE console_handle = (type == LOG_ERROR)
                              ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(console_handle, &cb_info);
        i32 original_colour = cb_info.wAttributes;

        SetConsoleTextAttribute(console_handle, (WORD)type);
            fprintf(output_stream, "%s: ", prefix);
        SetConsoleTextAttribute(console_handle, (WORD)original_colour);
    #endif

    va_start(args, msg);
        vfprintf(output_stream, msg, args);
    va_end(args);
}

void bgl_log_ctx_impl(LogType type, const char* msg, const char* file, i32 line, ...)
{
    va_list args;
    char buffer[8192];

    char file_no_path[128];
    find_file_from_path(file_no_path, 128, file);

    // ? bit scuffed doing this twice but whatever
    va_start(args, line);
        vsprintf(buffer, msg, args);
    va_end(args);

    bgl_log_impl(type, "%s in %s on line %d\n\n", buffer, file_no_path, line);
}