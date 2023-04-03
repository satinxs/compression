#include <common.h>

typedef struct bit_stream_t
{
    u8 *buffer;
    u32 buffer_length;
    u32 buffer_position;

    u8 byte_buffer;
    u8 bit_count;
} bit_stream_t;

void bit_stream_reset(bit_stream_t *stream);

bit_stream_t bit_stream_init(array_t buffer);

error_t bit_stream_unflush(bit_stream_t *stream);
error_t bit_stream_flush(bit_stream_t *stream);

error_t bit_stream_read_bit(bit_stream_t *stream, u8 *bit);
error_t bit_stream_write_bit(bit_stream_t *stream, u8 bit);

error_t bit_stream_read_int(bit_stream_t *stream, u32 *number, u8 bits);
error_t bit_stream_write_int(bit_stream_t *stream, u32 number, u8 bits);

// Reads an int using 7-bit VLQ approach
error_t bit_stream_read_7bit_int32(bit_stream_t *stream, u32 *number);
error_t bit_stream_write_7bit_int32(bit_stream_t *stream, u32 number);
