#ifndef BGL_LOG_H
#define BGL_LOG_H

#include <string.h>
#include "types.h"

// theoretically this unholy macro should be optimised out in compilation
#ifdef __linux__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif _WIN32
#define __FILENAME__ (strrchr(__FILE__, '/') || strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '/') > strrchr(__FILE__, '\\') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') + 1) : __FILE__)
#endif
#define BGL_LOG(type, msg, ...) log_impl(type, __FILENAME__, __LINE__, msg,  ##__VA_ARGS__)
#define BGL_LOG_NO_CTX(type, msg, ...) log_impl(type, NULL, 0, msg, ##__VA_ARGS__) // no file/line suffix

#ifdef BGL_NO_DEBUG
#define BGL_LOG_INFO(msg, ...)
#define BGL_LOG_WARN(msg, ...)
#define BGL_LOG_ERROR(msg, ...) BGL_LOG(BGL_LOG_ERROR, msg, ##__VA_ARGS__)
#define BGL_LOG_INFO_NO_CTX(msg, ...)
#define BGL_LOG_WARN_NO_CTX(msg, ...)
#define BGL_LOG_ERROR_NO_CTX(msg, ...) BGL_LOG_NO_CTX(BGL_LOG_ERROR, msg, ##__VA_ARGS__)
#else
#define BGL_LOG_INFO(msg, ...) BGL_LOG(BGL_LOG_INFO, msg, ##__VA_ARGS__)
#define BGL_LOG_WARN(msg, ...) BGL_LOG(BGL_LOG_WARN, msg, ##__VA_ARGS__)
#define BGL_LOG_ERROR(msg, ...) BGL_LOG(BGL_LOG_ERROR, msg, ##__VA_ARGS__)
#define BGL_LOG_INFO_NO_CTX(msg, ...) BGL_LOG_NO_CTX(BGL_LOG_INFO, msg, ##__VA_ARGS__)
#define BGL_LOG_WARN_NO_CTX(msg, ...) BGL_LOG_NO_CTX(BGL_LOG_WARN, msg, ##__VA_ARGS__)
#define BGL_LOG_ERROR_NO_CTX(msg, ...) BGL_LOG_NO_CTX(BGL_LOG_ERROR, msg, ##__VA_ARGS__)
#endif

typedef enum LogType
{
    #ifdef __linux__
        BGL_LOG_ERROR = 91, // use enum as colours
        BGL_LOG_WARN = 93,
        BGL_LOG_INFO = 35
    #elif _WIN32
        BGL_LOG_ERROR = 12,
        BGL_LOG_WARN = 14,
        BGL_LOG_INFO = 13 
    #endif
} LogType;

void log_impl(LogType type, const char* filename, i32 line, const char* msg, ...);

#endif
