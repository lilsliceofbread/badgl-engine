#ifndef BGL_DEFINES_H
#define BGL_DEFINES_H

#include <stdlib.h>

#include "types.h"
#include "platform.h"
#include "log.h"

#ifdef NDEBUG
#define BGL_NO_DEBUG
#endif

#ifdef BGL_NO_DEBUG
    #define DEBUG_BREAK() exit(-1)

    #define BGL_PERFORMANCE_START()
    #define BGL_PERFORMANCE_END(str)
#else
    #ifdef __linux__
        #define DEBUG_BREAK() __builtin_trap()
    #elif _WIN32
        #define DEBUG_BREAK() __debugbreak()
    #endif

    #define BGL_PERFORMANCE_START() f64 bgl_internal_perf_start_time = platform_get_time()
    #define BGL_PERFORMANCE_END(str) BGL_LOG_INFO("%s took %lfs\n", str, platform_get_time() - bgl_internal_perf_start_time) 
#endif

#define BGL_MALLOC(size) malloc(size)
#define BGL_CALLOC(size, count) calloc(size, count)  
#define BGL_REALLOC(ptr, size) realloc(ptr, size)
#define BGL_FREE(ptr) free(ptr)

/* assert defined in debug and release */
#define BGL_ASSERT(cond, msg, ...)         \
{                                          \
    if(!(cond))                            \
    {                                      \
        BGL_LOG_ERROR(msg, ##__VA_ARGS__); \
        DEBUG_BREAK();                     \
    }                                      \
}

#ifdef _WIN32
    #define BGL_EXPORT __declspec(dllexport)
#elif __linux__
    #define BGL_EXPORT
#endif

#ifdef __GNUC__
    // ignore unused variable warnings, at least for gcc
    #define BGL_UNUSED __attribute__((unused))
#else
    #define BGL_UNUSED
#endif

#define BGL_RESIZE_BLOCK_SIZE 8

/* resize array by block size if reached maximum size */
#define BLOCK_RESIZE_ARRAY(ptr_address, type, count, increase) do { \
u32 prev_max = ALIGNED_SIZE(count, BGL_RESIZE_BLOCK_SIZE);                       \
if(count + increase > prev_max)                                                  \
{                                                                                \
    u32 new_max = ALIGNED_SIZE(count + increase, BGL_RESIZE_BLOCK_SIZE);         \
    *(ptr_address) = (type*)BGL_REALLOC(*(ptr_address), new_max * sizeof(type)); \
    BGL_ASSERT(*(ptr_address) != NULL, #type "s array reallocation failed\n");   \
    BGL_LOG_INFO(#type "s array resize from %u to %u\n", prev_max, new_max);     \
}} while(0)

#define ALIGNED_SIZE(size, alignment) ((size) % (alignment) == 0) ? (size) : (size) + ((alignment) - ((size) % (alignment)))

#endif
