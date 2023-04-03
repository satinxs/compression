#include <common.h>

_API typedef struct lzss_config_t
{
    u8 offset_bits;
    u32 max_offset;

    u8 minimum_length;
    u8 length_bits;
    u32 max_length;
} lzss_config_t;

_API lzss_config_t lzss_config_init(u8 offset_bits, u8 length_bits, u8 minimum_length);

_API u32 lzss_get_upper_bound(u32 input_length);
_API error_t lzss_encode(lzss_config_t config, array_t input, array_t *output);

_API error_t lzss_get_original_length(array_t input, u32 *original_length);
_API error_t lzss_decode(lzss_config_t config, array_t input, array_t *output);
