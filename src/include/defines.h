#ifndef BGL_DEFINES_H
#define BGL_DEFINES_H

#include <stdlib.h>

#include "types.h"
#include "platform.h"
#include "log.h"

#ifdef BGL_NO_DEBUG
    #define DEBUG_BREAK()
    #define BGL_ASSERT(cond, msg, ...)

    #define BGL_PERFORMANCE_START()
    #define BGL_PERFORMANCE_END(str)
#else
    #ifdef __linux__
        #define DEBUG_BREAK() __builtin_trap()
    #elif _WIN32
        #define DEBUG_BREAK() __debugbreak()
    #endif

    #define BGL_ASSERT(cond, msg, ...)               \
    {                                                \
        if(!(cond))                                  \
        {                                            \
            BGL_LOG(LOG_ERROR, msg, ##__VA_ARGS__);  \
            DEBUG_BREAK();                           \
        }                                            \
    }

    #define BGL_PERFORMANCE_START() double bgl_internal_perf_start_time = platform_get_time()
    #define BGL_PERFORMANCE_END(str) BGL_LOG(LOG_INFO, "%s took %lfs\n", str, platform_get_time() - bgl_internal_perf_start_time) 
#endif

#ifdef _WIN32
    #define BGL_EXPORT __declspec(dllexport)

    #define FILEPATH_SEPARATOR '\\'
#elif __linux__
    #define BGL_EXPORT

    #define FILEPATH_SEPARATOR '/'
#endif

#ifdef __GNUC__
    // ignore unused variable warnings, at least for gcc
    #define BGL_UNUSED __attribute__((unused))
#else
    #define BGL_UNUSED
#endif

#define ALIGNED_SIZE(size, alignment) ((size) % (alignment) == 0) ? (size) : (size) + ((alignment) - ((size) % (alignment)))

#endif