#include <lzss.h>
#include "bit_stream.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

_API lzss_config_t lzss_config_init(u8 offset_bits, u8 length_bits, u8 minimum_length)
{
    return (lzss_config_t){

        .offset_bits = offset_bits,
        .max_offset = (1 << offset_bits) - 1,

        .minimum_length = minimum_length,
        .length_bits = length_bits,
        .max_length = (1 << length_bits) - 1,
    };
}

_API u32 lzss_get_upper_bound(u32 input_length)
{
    // We sum all bits in the worst case scenario: 32 for the total input length and input_lenght * 9 (literal flag + byte)
    u32 total_bits = 32 + input_length * 9;

    // If it's divisible by 8, we return the length. If not we sum 1 to account for the extra bits.
    return (total_bits / 8) + ((total_bits % 8 > 0) ? 1 : 0);
}

_API error_t lzss_get_original_length(buffer_t input, u32 *original_length)
{
    bit_stream_t stream = bit_stream_init(input);

    u32 length = 0;
    error_t error = bit_stream_read_7bit_int32(&stream, &length);

    if (error)
    {
        *original_length = 0;
        return error;
    }

    *original_length = length;
    return error;
}

typedef struct match_t
{
    u32 offset;
    u32 length;
} match_t;

static inline match_t __get_longest_match(lzss_config_t config, buffer_t input, u32 index)
{
    if (index + config.minimum_length >= input.length)
        return (match_t){.offset = 0, .length = 0};

    u32 best_offset = 0, best_length = 0;
    u32 offset = (config.max_offset > index) ? 0 : index - config.max_offset;

    while (offset < index && offset < input.length)
    {
        u32 length = 0;

        while (offset + length < input.length && index + length < input.length && input.bytes[offset + length] == input.bytes[index + length])
            length += 1;

        // We compare greater or equal, since a lower offset is better
        if (length >= best_length)
        {
            best_length = length;
            best_offset = offset;
        }

        offset += 1;
    }

    // Substract the found offset from the actual index to get the resulting offset.
    return (match_t){.offset = index - best_offset, .length = MIN(best_length, config.max_length)};
}

#define try(fn)       \
    if ((error = fn)) \
        goto error_exit;

error_t lzss_encode(lzss_config_t config, buffer_t input, buffer_t *output)
{
    error_t error = ERROR_ALL_GOOD;

    // If there are no input bytes, we don't have to do anything.
    if (input.length == 0)
        return ERROR_NO_OP;

    bit_stream_t stream = bit_stream_init(*output);

    // Write the initial size of the buffer
    try(bit_stream_write_7bit_int32(&stream, input.length));

    for (u32 index = 0; index < input.length;)
    {
        match_t match = __get_longest_match(config, input, index);

        if (match.length >= config.minimum_length)
        {
            try(bit_stream_write_bit(&stream, 1));
            try(bit_stream_write_int(&stream, match.offset, config.offset_bits));
            try(bit_stream_write_int(&stream, match.length, config.length_bits));
            index += match.length;
        }
        else
        {
            try(bit_stream_write_bit(&stream, 0));
            try(bit_stream_write_int(&stream, input.bytes[index], 8));
            index += 1;
        }
    }

    try(bit_stream_flush(&stream));

    goto no_error_exit;

error_exit:
    output->length = 0;
    return error;

no_error_exit:
    output->length = stream.buffer_position;
    return error;
}

error_t lzss_decode(lzss_config_t config, buffer_t input, buffer_t *output)
{
    error_t error = ERROR_ALL_GOOD;

    if (input.length == 0 || output->length == 0)
        return ERROR_NO_OP;

    bit_stream_t stream = bit_stream_init(input);

    u32 original_size = 0;
    try(bit_stream_read_7bit_int32(&stream, &original_size));

    if (original_size != output->length)
        return ERROR_WRONG_OUTPUT_SIZE;

    for (u32 index = 0; index < output->length;)
    {
        u8 is_pair = 0;
        try(bit_stream_read_bit(&stream, &is_pair));

        if (is_pair)
        {
            u32 offset = 0;
            try(bit_stream_read_int(&stream, &offset, config.offset_bits));

            u32 length = 0;
            try(bit_stream_read_int(&stream, &length, config.length_bits));

            for (u32 i = 0; i < length; i += 1)
                output->bytes[index + i] = output->bytes[index - offset + i];

            index += length;
        }
        else
        {
            u32 literal = 0;
            try(bit_stream_read_int(&stream, &literal, 8));
            output->bytes[index] = (u8)(literal & 0xFF);
            index += 1;
        }
    }

error_exit:
    return error;
}

#undef try
