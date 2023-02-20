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

typedef struct lzss_config_t
{
    u8 offset_bits;
    u32 max_offset;

    u8 minimum_length;
    u8 length_bits;
    u32 max_length;
} lzss_config_t;

typedef struct buffer_t
{
    u8 *bytes;
    u32 length;
} buffer_t;

#endif

u32 jenkins32(buffer_t buffer);
u32 adler32(buffer_t buffer);
u32 hash_bytes(buffer_t buffer);

lzss_config_t lzss_config_init(u8 offset_bits, u8 length_bits, u8 minimum_length);

u32 lzss_get_upper_bound(u32 input_length);
error_t lzss_encode(lzss_config_t config, u8 const *input, u32 input_length, u8 *output, u32 output_bound_size, u32 *output_length);

error_t lzss_get_original_length(u8 const *input, u32 input_length, u32 *original_length);
error_t lzss_decode(lzss_config_t config, u8 const *input, u32 input_length, u8 *output, u32 output_length);
