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

typedef enum error_t
{
    ERROR_ALL_GOOD = 0,
    ERROR_NO_OP,
    ERROR_BUFFER_OUT_OF_BOUNDS,
    ERROR_COULD_NOT_ALLOCATE,
    ERROR_WRONG_OUTPUT_SIZE,
    ERROR_BAD_FORMAT,
    ERROR_FILE_NOT_FOUND,
    ERROR_COULD_NOT_OPEN_FILE,
    ERROR_COULD_NOT_READ_FILE,
    ERROR_COULD_NOT_WRITE_FILE,
} error_t;

typedef struct lzss_config_t
{
    u8 offset_bits;
    u32 max_offset;

    u8 minimum_length;
    u8 length_bits;
    u32 max_length;
} lzss_config_t;

#endif

lzss_config_t lzss_config_init(u8 offset_bits, u8 length_bits, u8 minimum_length);

u32 lzss_get_upper_bound(u32 input_length);
error_t lzss_encode(lzss_config_t config, u8 const *input, u32 input_length, u8 *output, u32 output_bound_size, u32 *output_length);

error_t lzss_get_original_length(u8 const *input, u32 input_length, u32 *original_length);
error_t lzss_decode(lzss_config_t config, u8 const *input, u32 input_length, u8 *output, u32 output_length);