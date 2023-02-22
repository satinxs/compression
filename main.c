#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <lzss.h>
#include <rolz.h>

#include "command_line.h"

static error_t do_lzss_encoding(buffer_t input, buffer_t *output)
{
    lzss_config_t config = lzss_config_init(10, 6, 2);
    u32 output_upper_bound = lzss_get_upper_bound(input.length);

    output->bytes = (u8 *)calloc(output_upper_bound, sizeof(u8));
    output->length = output_upper_bound;

    if (output->bytes == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    return lzss_encode(config, input, output);
}

static error_t do_lzss_decoding(buffer_t input, buffer_t *output)
{
    error_t error = ERROR_ALL_GOOD;

    lzss_config_t config = lzss_config_init(10, 6, 2);

    u32 original_length = 0;
    if ((error = lzss_get_original_length(input, &original_length)))
        return error;

    output->bytes = (u8 *)calloc(original_length, sizeof(u8));
    output->length = original_length;

    return lzss_decode(config, input, output);
}

static error_t do_rolz_encoding(buffer_t input, buffer_t *output)
{
    const rolz_config_t config = rolz_config_init(8, 4, 2, 16);
    u32 output_upper_bound = rolz_get_upper_bound(input.length);

    output->bytes = (u8 *)calloc(output_upper_bound, sizeof(u8));
    output->length = output_upper_bound;

    if (output->bytes == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    return rolz_encode(config, input, output);
}

static error_t do_rolz_decoding(buffer_t input, buffer_t *output)
{
    error_t error = ERROR_ALL_GOOD;

    const rolz_config_t config = rolz_config_init(8, 4, 2, 16);

    u32 original_length = 0;
    if ((error = rolz_get_original_length(input, &original_length)))
        return error;

    output->bytes = (u8 *)calloc(original_length, sizeof(u8));
    output->length = original_length;

    return rolz_decode(config, input, output);
}

static int print_error_message(command_line_error_t cli_error, error_t lib_error)
{
    if (cli_error)
    {
        switch (cli_error)
        {
        case CLI_COULD_NOT_ALLOCATE:
            printf("Error: Could not allocate enough memory.\n");
            break;

        case CLI_COULD_NOT_OPEN_FILE:
            printf("Error: Could not open the file.\n");
            break;

        case CLI_COULD_NOT_READ_FILE:
            printf("Error: Could not read the file.\n");
            break;

        case CLI_FILE_NOT_FOUND:
            printf("Error: File not found.\n");
            break;

        default:
            printf("CLI Error code: %d\n", cli_error);
            break;
        }

        // TODO: Insert switch/case to print errors
        return EXIT_FAILURE;
    }

    if (lib_error)
    {
        // TODO: Insert switch/case to print errors
        printf("Lib Error code: %d\n", lib_error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, const char **argv)
{
    command_line_error_t cli_error = CLI_NO_ERROR;
    error_t lib_error = ERROR_ALL_GOOD;

    command_line_options_t options = {0};
    if ((cli_error = parse_command_line_arguments(argc, argv, &options)))
        goto exit;

    buffer_t input_file = {0};
    if ((cli_error = read_file(argv[3], &input_file)))
    {
        printf("Failed when reading input file \"%s\"\n", argv[3]);
        goto exit;
    }

    buffer_t output_file = {0};

    clock_t start_time = clock();

    switch (options.mode)
    {
    case MODE_LZSS:
        if (options.operation == OP_ENCODE)
        {
            if ((lib_error = do_lzss_encoding(input_file, &output_file)))
                goto exit;
        }
        else
        {
            if ((lib_error = do_lzss_decoding(input_file, &output_file)))
                goto exit;
        }
        break;
    case MODE_ROLZ:
        if (options.operation == OP_ENCODE)
        {
            if ((lib_error = do_rolz_encoding(input_file, &output_file)))
                goto exit;
        }
        else
        {
            if ((lib_error = do_rolz_decoding(input_file, &output_file)))
                goto exit;
        }
        break;
    }

    clock_t end_time = clock();

    printf("Compressed %d to %d bytes in %ldms\n", input_file.length, output_file.length, (end_time - start_time) / (CLOCKS_PER_SEC / 1000));

    if ((cli_error = write_file(argv[4], output_file)))
    {
        printf("Failed when writing output file \"%s\"\n", argv[4]);
        goto exit;
    }

exit:
    // TODO: Should we deallocate? We're finishing the program here.
    return print_error_message(cli_error, lib_error);
}