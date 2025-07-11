#ifndef BGL_LOG_H
#define BGL_LOG_H

#include <string.h>
#include "types.h"
#include "platform.h"

/* _SOURCE_PATH_LENGTH is defined by cmake - move ptr past path to filename */
#define __FILENAME__ ((__FILE__) + (_SOURCE_PATH_LENGTH))

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
    /* use enum as colours */
    BGL_LOG_ERROR = BGL_RED,
    BGL_LOG_WARN = BGL_YELLOW,
    BGL_LOG_INFO = BGL_MAGENTA,
} LogType;

void log_impl(LogType type, const char* filename, i32 line, const char* msg, ...);

#endif
