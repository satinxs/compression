#ifndef __COMMAND_LINE_H__
#define __COMMAND_LINE_H__

typedef enum mode_t
{
    MODE_LZSS,
    MODE_ROLZ
} mode_t;

typedef enum operation_t
{
    OP_ENCODE,
    OP_DECODE
} operation_t;

typedef enum command_line_error_t
{
    CLI_NO_ERROR,
    CLI_BAD_FORMAT,
    CLI_NOT_ENOUGH_ARGUMENTS,
    CLI_FILE_NOT_FOUND,
    CLI_COULD_NOT_OPEN_FILE,
    CLI_COULD_NOT_ALLOCATE,
    CLI_COULD_NOT_READ_FILE,
    CLI_COULD_NOT_WRITE_FILE
} command_line_error_t;

typedef struct command_line_options_t
{
    mode_t mode;
    operation_t operation;
    const char *input_file;
    const char *output_file;
} command_line_options_t;

#include <common.h>

command_line_error_t parse_command_line_arguments(int argc, const char **argv, command_line_options_t *options);

command_line_error_t read_file(const char *file_name, buffer_t *buffer);
command_line_error_t write_file(const char *file_name, buffer_t buffer);

#endif
