#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "command_line.h"

void test_lzss(const char *file_name)
{
    printf("Testing LZSS compression with \"%s\"\n", file_name);

    buffer_t input_file = {0};
    if (read_file(file_name, &input_file))
    {
        printf("Failed when reading input file \"%s\"\n", file_name);
        return;
    }

    const u32 original_hash1 = jenkins32(input_file);
    const u32 original_hash2 = adler32(input_file);
    const u32 original_hash3 = hash_bytes(input_file);

    const lzss_config_t config = lzss_config_init(10, 6, 2);

    const u32 upper_bound_length = lzss_get_upper_bound(input_file.length);

    buffer_t encoded = {0};
    if (!(encoded.bytes = (u8 *)calloc(upper_bound_length, sizeof(u8))))
    {
        printf("Failed when allocating memory for the compressed buffer.\n");
        return;
    }
    encoded.length = upper_bound_length;

    error_t error = ERROR_ALL_GOOD;

    const clock_t start_encoding = clock();

    if ((error = lzss_encode(config, input_file, &encoded, upper_bound_length)))
    {
        printf("Failed encoding, error: %d\n", error);
        free(encoded.bytes);
        free(input_file.bytes);
        return;
    }

    const clock_t end_encoding = clock();

    const u64 elapsed_encoding = (end_encoding - start_encoding) / (u64)(CLOCKS_PER_SEC / 1000.0f);
    const float encoding_percentage = (float)encoded.length / (float)input_file.length * 100.0f;
    const float encoding_bits_per_ms = (encoded.length * 8.0f) / elapsed_encoding;

    printf("Encoded %d->%d, a %f%% compression rate in %lldms. Speed of %f bits/ms\n", input_file.length, encoded.length, encoding_percentage, elapsed_encoding, encoding_bits_per_ms);

    buffer_t decoded = {0};
    if (!(decoded.bytes = (u8 *)calloc(input_file.length, sizeof(u8))))
    {
        printf("Failed when allocating memory for the decoded buffer.\n");
        return;
    }
    decoded.length = input_file.length;

    const clock_t start_decoding = clock();

    if ((error = lzss_decode(config, encoded, &decoded)))
    {
        printf("Failed decoding, error: %d\n", error);
        free(encoded.bytes);
        free(decoded.bytes);
        free(input_file.bytes);
        return;
    }

    const clock_t end_decoding = clock();

    const u64 elapsed_decoding = (end_decoding - start_decoding) / (u64)(CLOCKS_PER_SEC / 1000.0f);
    const float decoding_bits_per_ms = (decoded.length * 8.0f) / elapsed_decoding;

    printf("Decoded %d bytes in %lldms. Speed of %f bits/ms\n", input_file.length, elapsed_decoding, decoding_bits_per_ms);

    const u32 decoded_hash1 = jenkins32(decoded);
    const u32 decoded_hash2 = adler32(decoded);
    const u32 decoded_hash3 = hash_bytes(decoded);

    if (!(original_hash1 == decoded_hash1 && original_hash2 == decoded_hash2 && original_hash3 == decoded_hash3))
    {
        printf("Failed comparing hashes.\n");
        return;
    }

    printf("\n");

    printf("%x %s %x (Jenkins)\n", original_hash1, original_hash1 == decoded_hash1 ? "==" : "!=", decoded_hash1);
    printf("%x %s %x (Adler)\n", original_hash2, original_hash2 == decoded_hash2 ? "==" : "!=", decoded_hash2);
    printf("%x %s %x (Combined)\n", original_hash3, original_hash3 == decoded_hash3 ? "==" : "!=", decoded_hash3);

    printf("Success!\n\n");
}

int main(int argc, const char **argv)
{
    test_lzss("KingsBounty.md");
    test_lzss("package-lock.json");
    test_lzss("Makefile");
    test_lzss("main.c");

    return EXIT_SUCCESS;
}
