#include <stdio.h>
#include <common.h>
#include <string.h>
#include <stdlib.h>

static void print_usage(char *exe_name)
{
    printf("Usage:%s <mode> <input> <output>\n", exe_name);
    printf("Where mode can be one of LZSS, ROLZ or 1 or 2 respectively.\n");
}

typedef enum mode_t
{
    MODE_LZSS,
    MODE_ROLZ
} mode_t;

static error_t parse_mode(const char *string, mode_t *mode)
{
    if (strcmpi(string, "LZSS") || strcmpi(string, "1"))
    {
        *mode = MODE_LZSS;
        return ERROR_ALL_GOOD;
    }
    if (strcmpi(string, "ROLZ") || strcmpi(string, "2"))
    {
        *mode = MODE_ROLZ;
        return ERROR_ALL_GOOD;
    }

    return ERROR_BAD_FORMAT;
}

static error_t read_file(const char *file_name, u8 **buffer, u32 *buffer_size)
{
    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
        return ERROR_FILE_NOT_FOUND;

    fseek(file, 0, SEEK_END);   // Seek to the end of the file
    *buffer_size = ftell(file); // Get how many bytes the file contains
    fseek(file, 0, SEEK_SET);   // Rewind the file pointer to 0

    *buffer = calloc(*buffer_size, sizeof(u8));

    if (*buffer == NULL)
    {
        fclose(file);
        return ERROR_COULD_NOT_ALLOCATE;
    }

    u32 read_value = fread(*buffer, sizeof(u8), *buffer_size, file);

    if (read_value != *buffer_size)
    {
        free(*buffer);
        *buffer = NULL;
        *buffer_size = 0;
        fclose(file);
        return ERROR_COULD_NOT_READ_FILE;
    }

    fclose(file);
    return ERROR_ALL_GOOD;
}

static error_t write_file(const char *file_name, u8 *buffer, u32 buffer_size)
{
    FILE *file = fopen(file_name, "wb+");

    if (file == NULL)
        return ERROR_COULD_NOT_OPEN_FILE;

    u32 written_bytes = fwrite(buffer, sizeof(u8), buffer_size, file);

    fflush(file);
    fclose(file);

    if (written_bytes != buffer_size)
        return ERROR_WRONG_OUTPUT_SIZE;

    return ERROR_ALL_GOOD;
}

int main(int argc, char **argv)
{
    if (argc != 4)
        print_usage(argv[0]);
    else
    {
        mode_t mode;
        if (parse_mode(argv[1], &mode))
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        u8 *input_file;
        u32 input_file_size;
        if (read_file(argv[2], &input_file, &input_file_size))
        {
            printf("Failed when reading input file \"%s\"\n", argv[2]);
            return EXIT_FAILURE;
        }

        printf("Bytes read: %d\n", input_file_size);

        switch (mode)
        {
        case MODE_LZSS:
        {
            lzss_config_t config = lzss_config_init(10, 6, 2);
            u32 output_upper_bound = lzss_get_upper_bound(input_file_size);

            u8 *buffer = calloc(output_upper_bound, sizeof(u8));
            u32 output_length = 0;
            if (lzss_encode(config, input_file, input_file_size, buffer, output_upper_bound, &output_length))
            {
                printf("An error ocurred!\n");
                return EXIT_FAILURE;
            }

            if (write_file(argv[3], buffer, output_length))
            {
                printf("Failed when writing output file \"%s\"\n", argv[3]);
                return EXIT_FAILURE;
            }

            break;
        }
        case MODE_ROLZ:
        {
            break;
        }
        }
    }

    return EXIT_SUCCESS;
}