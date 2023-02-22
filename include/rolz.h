#include <common.h>

_API typedef struct rolz_config_t
{
    u8 step_bits;
    u32 max_step;

    u8 count_bits;
    u32 max_count;

    u8 history_buffer_bits;
    u32 max_offset;

    u8 minimum_match;
} rolz_config_t;

_API rolz_config_t rolz_config_init(u8 step_bits, u8 count_bits, u8 minimum_match, u8 history_buffer_bits);

_API u32 rolz_get_upper_bound(u32 input_length);
_API error_t rolz_encode(rolz_config_t config, buffer_t input, buffer_t *output);

_API error_t rolz_get_original_length(buffer_t input, u32 *original_length);
_API error_t rolz_decode(rolz_config_t config, buffer_t input, buffer_t *output);
