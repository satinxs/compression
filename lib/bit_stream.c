#include <bit_stream.h>

inline void bit_stream_reset(bit_stream_t *stream)
{
    stream->buffer_position = 0;
}

inline bit_stream_t bit_stream_init(u8 *buffer, u32 buffer_length)
{
    return (bit_stream_t){
        .buffer = buffer,
        .buffer_length = buffer_length,
        .buffer_position = 0,

        .byte_buffer = 0,
        .bit_count = 0,
    };
}

error_t bit_stream_unflush(bit_stream_t *stream)
{
    if (stream->buffer_position < stream->buffer_length)
    {
        stream->byte_buffer = stream->buffer[stream->buffer_position++];
        stream->bit_count = 8;

        return ERROR_ALL_GOOD;
    }

    return ERROR_BUFFER_OUT_OF_BOUNDS;
}

error_t bit_stream_flush(bit_stream_t *stream)
{
    if (stream->bit_count == 0)
        return ERROR_ALL_GOOD;

    if (stream->bit_count < 8)
        stream->byte_buffer <<= (8 - stream->bit_count);

    if (stream->buffer_position >= stream->buffer_length)
        return ERROR_BUFFER_OUT_OF_BOUNDS;

    stream->buffer[stream->buffer_position++] = stream->byte_buffer;
    stream->byte_buffer = 0;
    stream->bit_count = 0;

    return ERROR_ALL_GOOD;
}

error_t bit_stream_read_bit(bit_stream_t *stream, u8 *bit)
{
    error_t error = ERROR_ALL_GOOD;

    if (stream->bit_count == 0)
        if ((error = bit_stream_unflush(stream)))
            return error;

    stream->bit_count -= 1;

    *bit = (stream->byte_buffer & (1 << (stream->bit_count))) > 0;

    return error;
}

error_t bit_stream_write_bit(bit_stream_t *stream, u8 bit)
{
    stream->byte_buffer <<= 1;

    stream->byte_buffer |= bit & 1;

    stream->bit_count += 1;

    if (stream->bit_count == 8)
        return bit_stream_flush(stream);

    return ERROR_ALL_GOOD;
}

error_t bit_stream_read_int(bit_stream_t *stream, u32 *number, u8 bits)
{
    error_t error = ERROR_ALL_GOOD;

    u32 value = 0;
    for (u32 i = 0; i < bits; i += 1)
    {
        value <<= 1;

        u8 bit = 0;

        if ((error = bit_stream_read_bit(stream, &bit)))
            return error;

        value |= bit & 1;
    }
    *number = value;

    return error;
}

error_t bit_stream_write_int(bit_stream_t *stream, u32 number, u8 bits)
{
    while (bits > 0)
    {
        u32 mask = 1 << (bits - 1);
        u8 bit = (number & mask) > 0;
        error_t error = bit_stream_write_bit(stream, bit);

        if (error != ERROR_ALL_GOOD)
            return error;

        bits -= 1;
    }

    return ERROR_ALL_GOOD;
}

// Reads an int using 7-bit VLQ approach
error_t bit_stream_read_7bit_int32(bit_stream_t *stream, u32 *number)
{
    error_t error = ERROR_ALL_GOOD;

    u32 n = 0;
    u8 shift = 0;
    while (1)
    {
        u32 byte = 0;
        if ((error = bit_stream_read_int(stream, &byte, 8)))
            return error;

        n |= (byte & 127) << shift;
        shift += 7;

        if ((byte & 128) == 0 || shift > 32)
            break;
    }
    *number = n;

    return error;
}

// Writes an int using 7-bit VLQ approach
error_t bit_stream_write_7bit_int32(bit_stream_t *stream, u32 number)
{
    error_t error = ERROR_ALL_GOOD;

    u32 n = number;
    // 127 = 7 bits
    while (n > 127)
    {
        u32 b = 128 | (n & 127); // Set the first bit as 1
        if ((error = bit_stream_write_int(stream, b, 8)))
            return error;

        n >>= 7;
    }

    // This check is probably not required.
    if (n > 0)
        bit_stream_write_int(stream, n & 127, 8);

    return error;
}
