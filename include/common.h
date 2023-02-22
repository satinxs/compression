#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#ifdef _SHARED
#define _API __declspec(dllexport)
#else
#define _API
#endif

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef enum error_t
{
    ERROR_ALL_GOOD = 0,
    ERROR_NO_OP,
    ERROR_BUFFER_OUT_OF_BOUNDS,
    ERROR_COULD_NOT_ALLOCATE,
    ERROR_WRONG_OUTPUT_SIZE
} error_t;

typedef struct buffer_t
{
    u8 *bytes;
    u32 length;
} buffer_t;

#endif

u32 jenkins32(buffer_t buffer);
u32 adler32(buffer_t buffer);
u32 hash_bytes(buffer_t buffer);
