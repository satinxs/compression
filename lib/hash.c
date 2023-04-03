#include <common.h>

static const u32 ADLER_32_MOD = 65521;

u32 jenkins32(array_t buffer)
{
    u32 hash = 0;

    for (u32 i = 0; i < buffer.length; i += 1)
    {
        hash += buffer.bytes[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

u32 adler32(array_t buffer)
{
    u32 a = 1;
    u32 b = 0;

    for (u32 i = 0; i < buffer.length; i += 1)
    {
        a = (a + buffer.bytes[i]) % ADLER_32_MOD;
        b = (b + a) % ADLER_32_MOD;
    }

    return (b << 16) | a;
}

u32 hash_bytes(array_t buffer)
{
    u32 a = 1; // Part of adler32
    u32 b = 0; // Part of adler32
    u32 j = 0; // Hash accumulator for jenkins32

    for (u32 i = 0; i < buffer.length; i += 1)
    {
        // Read one byte from the buffer.
        const u8 byte = buffer.bytes[i];

        // Update adler32 variables.
        a = (a + byte) % ADLER_32_MOD;
        b = (b + a) % ADLER_32_MOD;

        // Update jenkins32 hash.
        j += byte;
        j += j << 10;
        j ^= j >> 6;
    }

    // Final mixing for jenkins32
    j += j << 3;
    j ^= j >> 11;
    j += j << 15;

    // Return xored hash results
    return ((b << 16) | a) ^ j;
}
