#ifndef BADGL_LOG_H
#define BADGL_LOG_H

#ifdef BADGL_NO_DEBUG
    #define BADGL_LOG(type, msg, ...)
    #define BADGL_LOG_NO_CTX(type, msg, ...)
#else
    #define BADGL_LOG(type, msg, ...) badgl_log_ctx_impl(type, msg, __FILE__, __LINE__, ##__VA_ARGS__)
    #define BADGL_LOG_NO_CTX(type, msg, ...) badgl_log_impl(type, msg, ##__VA_ARGS__) // no file/line suffix
#endif

typedef enum LogType
{
    #ifdef __linux__
        LOG_ERROR = 91, // use enum as colours
        LOG_WARN = 93,
        LOG_DEBUG = 35
    #elif _WIN32
        LOG_ERROR = 12,
        LOG_WARN = 14,
        LOG_DEBUG = 13 
    #endif
} LogType;

void badgl_log_impl(LogType type, const char* msg, ...);

void badgl_log_ctx_impl(LogType type, const char* msg, const char* file, int line, ...);

#endif