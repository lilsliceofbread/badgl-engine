#ifndef BGL_LOG_H
#define BGL_LOG_H

#ifdef BGL_NO_DEBUG
    #define BGL_LOG(type, msg, ...)
    #define BGL_LOG_NO_CTX(type, msg, ...)
#else
    #define BGL_LOG(type, msg, ...) bgl_log_ctx_impl(type, msg, __FILE__, __LINE__, ##__VA_ARGS__)
    #define BGL_LOG_NO_CTX(type, msg, ...) bgl_log_impl(type, msg, ##__VA_ARGS__) // no file/line suffix
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

void bgl_log_impl(LogType type, const char* msg, ...);

void bgl_log_ctx_impl(LogType type, const char* msg, const char* file, int line, ...);

#endif