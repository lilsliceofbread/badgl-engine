#ifndef BGL_DEFINES_H
#define BGL_DEFINES_H

#include <stdlib.h>

#include "types.h"
#include "platform.h"
#include "log.h"

#ifdef NDEBUG
#ifndef BGL_NO_DEBUG
#define BGL_NO_DEBUG
#endif
#endif

#ifdef BGL_NO_DEBUG
    #define BGL_EXIT() exit(-1)

    #define BGL_PERFORMANCE_START()
    #define BGL_PERFORMANCE_END(str)
#else
    #if defined(__linux__)
        #define BGL_EXIT() __builtin_trap()
    #elif defined(_WIN32)
        #define BGL_EXIT() __debugbreak()
    #endif

    #define BGL_PERFORMANCE_START() f64 _bgl_perf_start_time = platform_get_time()
    #define BGL_PERFORMANCE_END(str) BGL_LOG_INFO("%s took %.3lfms", str, (platform_get_time() - _bgl_perf_start_time) * 1000.0) 
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
        BGL_EXIT();                        \
    }                                      \
}

#define BGL_ASSERT_NO_MSG(cond)            \
{                                          \
    if(!(cond))                            \
    {                                      \
        BGL_EXIT();                        \
    }                                      \
}

#ifdef _WIN32
    #define BGL_EXPORT __declspec(dllexport)
#else
    #define BGL_EXPORT
#endif

#ifdef __GNUC__
    // ignore unused variable warnings, at least for gcc
    #define BGL_UNUSED __attribute__((unused))
#else
    #define BGL_UNUSED
#endif

#define BGL_RESIZE_BLOCK_SIZE 8

#define ALIGNED_SIZE(size, alignment) (u32)( (size) + (alignment) - 1 - ( ((size) + (alignment) - 1) % (alignment) ) )

/* resize array by block size if reached maximum size */
#define BLOCK_RESIZE_ARRAY(ptr_address, type, count, increase) do { \
u32 prev_max = ALIGNED_SIZE(count, BGL_RESIZE_BLOCK_SIZE);                       \
if(count + increase > prev_max)                                                  \
{                                                                                \
    u32 new_max = ALIGNED_SIZE(count + increase, BGL_RESIZE_BLOCK_SIZE);         \
    *(ptr_address) = (type*)BGL_REALLOC(*(ptr_address), new_max * sizeof(type)); \
    BGL_ASSERT(*(ptr_address) != NULL, #type "s array reallocation failed");     \
    BGL_LOG_INFO(#type "s array resize from %u to %u", prev_max, new_max);       \
}} while(0)

#define CHAR_IS_NUMBER(c) (0x30 <= (c) && (c) <= 0x39)
#define CHAR_TO_INT(c) (i32)((c) - 0x30)
#define INT_TO_CHAR(i) (char)((i) + 0x30)

/* expand macro and convert to string */
#define MACRO_TO_MACRO_NAME(macro) #macro
#define MACRO_TO_STR(macro) MACRO_TO_MACRO_NAME(macro)


#endif
