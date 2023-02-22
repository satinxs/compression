#include <rolz.h>
#include <bit_stream.h>
#include <stdlib.h>

typedef struct match_t
{
    u32 steps;
    u32 length;
} match_t;

_API rolz_config_t rolz_config_init(u8 step_bits, u8 count_bits, u8 minimum_match, u8 history_buffer_bits)
{
    return (rolz_config_t){
        .step_bits = step_bits,
        .max_step = (1 << step_bits) - 1,

        .count_bits = count_bits,
        .max_count = (1 << count_bits) - 1,

        .history_buffer_bits = history_buffer_bits,
        .max_offset = (1 << history_buffer_bits) - 1,

        .minimum_match = minimum_match,
    };
}

_API u32 rolz_get_upper_bound(u32 input_length)
{
    // We sum all bits in the worst case scenario: 32 for the total input length and input_lenght * 9 (literal flag + byte)
    u32 total_bits = 32 + input_length * 9;

    // If it's divisible by 8, we return the length. If not we sum 1 to account for the extra bits.
    return (total_bits / 8) + ((total_bits % 8 > 0) ? 1 : 0);
}

static inline match_t __get_longest_match(rolz_config_t config, buffer_t input, u32 index, u32 *dictionary, u32 buffer_mask)
{
    // If index-length difference is smaller than minimum match, we can't match a pair.
    if (index + config.minimum_match >= input.length)
        return (match_t){.steps = 0, .length = 0};

    u32 last_position = index;

    u32 max_count = 0, max_steps = 0;
    u32 steps = 0, count;

    while (1)
    {
        u32 position = dictionary[last_position & buffer_mask];

        // We reached the index or there is no other match.
        if (position >= last_position)
            break;

        // The position would end up in a bigger offset than what is supported
        if ((index - position) > config.max_offset)
            break;

        count = 0;

        while (count < config.max_count && (index + count + 1) < input.length)
        {
            // If the current byte is equal to the previous match
            if (input.bytes[index + count + 1] == input.bytes[position + count + 1])
                count += 1;
            else
                break;
        }

        if (count > max_count)
        {
            max_count = count;
            max_steps = steps;
        }

        if (steps >= config.max_step)
            break;

        steps += 1;
        last_position = position;
    }

    return (match_t){.steps = max_steps, .length = max_count};
}

#define try(fn)       \
    if ((error = fn)) \
        goto error_exit;

_API error_t rolz_encode(rolz_config_t config, buffer_t input, buffer_t *output)
{
    error_t error = ERROR_ALL_GOOD;

    // If there are no input bytes, we don't have to do anything.
    if (input.length == 0)
        return ERROR_NO_OP;

    // Rolz dictionary creation
    u32 buffer_mask = (1 << config.history_buffer_bits) - 1;
    u32 last_position_lookup[256] = {0};
    u32 *dictionary = (u32 *)calloc(buffer_mask + 1, sizeof(u32));

    if (dictionary == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    u32 dictionary_index = 0;

    bit_stream_t stream = bit_stream_init(*output);

    try(bit_stream_write_7bit_int32(&stream, input.length));

    u32 index = 0;

    do
    {
        u8 byte = input.bytes[index];
        dictionary[dictionary_index & buffer_mask] = last_position_lookup[byte];
        last_position_lookup[byte] = dictionary_index;
        dictionary_index += 1;

        try(bit_stream_write_bit(&stream, 0));
        try(bit_stream_write_int(&stream, byte, 8));

        while (1)
        {
            match_t match = __get_longest_match(config, input, index, dictionary, buffer_mask);

            if (match.length >= config.minimum_match)
            {
                try(bit_stream_write_bit(&stream, 1));
                try(bit_stream_write_int(&stream, match.length, config.count_bits));
                try(bit_stream_write_int(&stream, match.steps, config.step_bits));

                for (u32 i = 0; i < match.length; i += 1)
                {
                    index += 1;
                    u8 literal = input.bytes[index];
                    dictionary[dictionary_index & buffer_mask] = last_position_lookup[literal];
                    last_position_lookup[literal] = dictionary_index;
                    dictionary_index += 1;
                }
            }
            else
                break;
        }
        index += 1;
    } while (index < input.length);

    try(bit_stream_flush(&stream));

    goto no_error_exit;

error_exit:
    free(dictionary);
    output->length = 0;
    return error;

no_error_exit:
    free(dictionary);
    output->length = stream.buffer_position;
    return ERROR_ALL_GOOD;
}

_API error_t rolz_get_original_length(buffer_t input, u32 *original_length)
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

_API error_t rolz_decode(rolz_config_t config, buffer_t input, buffer_t *output)
{
    error_t error = ERROR_ALL_GOOD;

    // If there are no input bytes, we don't have to do anything.
    if (input.length == 0 || output->length == 0)
        return ERROR_NO_OP;

    // Rolz dictionary creation
    u32 buffer_mask = (1 << config.history_buffer_bits) - 1;
    u32 last_position_lookup[256] = {0};
    u32 *dictionary = (u32 *)calloc(buffer_mask + 1, sizeof(u32));

    if (dictionary == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    u32 dictionary_index = 0;

    bit_stream_t stream = bit_stream_init(input);

    u32 total_length = 0;
    try(bit_stream_read_7bit_int32(&stream, &total_length));

    if (total_length != output->length)
    {
        error = ERROR_WRONG_OUTPUT_SIZE;
        goto error_exit;
    }

    u32 index = 0;

    while (index < output->length)
    {
        u8 is_pair = 0;
        try(bit_stream_read_bit(&stream, &is_pair));

        if (is_pair)
        {
            u32 count = 0;
            try(bit_stream_read_int(&stream, &count, config.count_bits));

            u32 steps = 0;
            try(bit_stream_read_int(&stream, &steps, config.step_bits));

            // Find the position from the amount of steps.
            u32 position = index - 1;
            for (u32 i = 0; i <= steps; i += 1)
                position = dictionary[position & buffer_mask];

            u32 offset = index - 1 - position;
            for (u32 i = 0; i < count; i += 1)
            {
                u8 literal = output->bytes[index - offset];

                // We update the dictionary.
                dictionary[dictionary_index & buffer_mask] = last_position_lookup[literal];
                last_position_lookup[literal] = dictionary_index;
                dictionary_index += 1;

                // And output the literal.
                output->bytes[index] = literal;
                index += 1;
            }
        }
        else
        {
            u32 literal = 0;
            try(bit_stream_read_int(&stream, &literal, 8));

            // We update the dictionary.
            dictionary[dictionary_index & buffer_mask] = last_position_lookup[(u8)literal];
            last_position_lookup[(u8)literal] = dictionary_index;
            dictionary_index += 1;

            // And output the literal
            output->bytes[index] = (u8)literal;
            index += 1;
        }
    }

error_exit:
    free(dictionary);
    return error;
}

#undef try
