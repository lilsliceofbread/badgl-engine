#ifndef BADGL_DEFINES_H
#define BADGL_DEFINES_H

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "log.h"

#ifdef BADGL_NO_DEBUG
    #define ASSERT(cond, msg, ...)

    #define PERF_TIMER_START()
    #define PERF_TIMER_END(str)
#else

    #define ASSERT(cond, msg, ...)                   \
    {                                                \
        if(!(cond))                                  \
        {                                            \
            BADGL_LOG(LOG_ERROR, msg, ##__VA_ARGS__);\
            exit(1);                                 \
        }                                            \
    }

    #define PERF_TIMER_START() double perf_start_time = platform_get_time()
    #define PERF_TIMER_END(str) BADGL_LOG(LOG_DEBUG, "%s took %lfs\n", str, platform_get_time() - perf_start_time) 
#endif

#define ALIGNED_SIZE(size, alignment) ((size) % (alignment) == 0) ? (size) : (size) + ((alignment) - ((size) % (alignment)))

#endif