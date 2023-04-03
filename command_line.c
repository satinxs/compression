#include "command_line.h"

#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *exe_name)
{
    printf("Usage:%s <e|d> <mode> <input> <output>\n", exe_name);
    printf(" -> e for encoding, d for decoding.\n");
    printf(" -> mode can be either of: LZSS, ROLZ or 1 or 2 respectively.\n");
    printf(" -> input is the path of the file to process.\n");
    printf(" -> output is the path of the resulting file.\n");
}

static inline command_line_error_t parse_operation(const char *string, command_line_options_t *options)
{
    if (string[0] == 'e')
        options->operation = OP_ENCODE;
    else if (string[0] == 'd')
        options->operation = OP_DECODE;
    else
        return CLI_BAD_FORMAT;

    return CLI_NO_ERROR;
}

static inline command_line_error_t parse_mode(const char *string, command_line_options_t *options)
{
    if (strcasecmp(string, "LZSS") == 0 || strcasecmp(string, "1") == 0)
        options->mode = MODE_LZSS;
    else if (strcasecmp(string, "ROLZ") == 0 || strcasecmp(string, "2") == 0)
        options->mode = MODE_ROLZ;
    else
        return CLI_BAD_FORMAT;

    return CLI_NO_ERROR;
}

command_line_error_t parse_command_line_arguments(int argc, const char **argv, command_line_options_t *options)
{
    command_line_error_t error = CLI_NO_ERROR;

    if (argc != 5)
    {
        print_usage(argv[0]);
        return CLI_NOT_ENOUGH_ARGUMENTS;
    }

    if ((error = parse_operation(argv[1], options)))
    {
        print_usage(argv[0]);
        return error;
    }

    if ((error = parse_mode(argv[2], options)))
    {
        print_usage(argv[0]);
        return error;
    }

    // TODO: Validate file exists? Ask to rewrite output file? Accept verbosity/silent options?

    options->input_file = argv[3];
    options->output_file = argv[4];

    return error;
}

command_line_error_t read_file(const char *file_name, array_t *buffer)
{
    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
        return CLI_FILE_NOT_FOUND;

    fseek(file, 0, SEEK_END);     // Seek to the end of the file
    buffer->length = ftell(file); // Get how many bytes the file contains
    fseek(file, 0, SEEK_SET);     // Rewind the file pointer to 0

    buffer->bytes = (u8 *)malloc(buffer->length);

    if (buffer->bytes == NULL)
    {
        fclose(file);
        return CLI_COULD_NOT_ALLOCATE;
    }

    u32 read_value = fread(buffer->bytes, sizeof(u8), buffer->length, file);

    if (read_value != buffer->length)
    {
        free(buffer->bytes);
        buffer->bytes = NULL;
        buffer->length = 0;
        fclose(file);
        return CLI_COULD_NOT_READ_FILE;
    }

    fclose(file);
    return CLI_NO_ERROR;
}

command_line_error_t write_file(const char *file_name, array_t buffer)
{
    FILE *file = fopen(file_name, "wb+");

    if (file == NULL)
        return CLI_COULD_NOT_OPEN_FILE;

    u32 written_bytes = fwrite(buffer.bytes, sizeof(u8), buffer.length, file);

    fflush(file);
    fclose(file);

    if (written_bytes != buffer.length)
        return CLI_COULD_NOT_WRITE_FILE;

    return CLI_NO_ERROR;
}
