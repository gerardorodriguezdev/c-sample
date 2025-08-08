#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ARGUMENTS_PARSING_SUCCESS = 0,
    TARGET_DIRECTORY_NOT_PASSED = 1,
    INVALID_COMMAND_PASSED = 2,
} arguments_parsing_error;

typedef enum {
    FILE_PARSING_SUCCESS = 0,
    ERROR_OPENING_FILE = 1,
    ERROR_PARSING_LINE = 2,
} file_parsing_error;

arguments_parsing_error parse_arguments(const int number_of_arguments, char *arguments[], char **tar_dir) {
    if (number_of_arguments < 2) {
        return TARGET_DIRECTORY_NOT_PASSED;
    }

    char *string = arguments[1];
    const size_t length = strlen(string);
    if (length < 3 || string[0] != '-' || string[1] != '-') {
        return INVALID_COMMAND_PASSED;
    }

    char *target_directory = string + 2;
    if (*target_directory == '\0') {
        return INVALID_COMMAND_PASSED;
    }

    *tar_dir = target_directory;
    return ARGUMENTS_PARSING_SUCCESS;
}

int parse_line(const char *line) {
    const size_t length = strlen(line);

    if (length < 2) {
        return 1;
    }

    // Parse file here

    return 0;
}

file_parsing_error parse_file(const char *target_directory) {
    FILE *file = fopen(target_directory, "r");
    if (file == NULL) {
        return ERROR_OPENING_FILE;
    }

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        const int line_parsing_error = parse_line(line);
        if (line_parsing_error != 0) {
            return ERROR_PARSING_LINE;
        }
    }

    free(line);
    fclose(file);

    return FILE_PARSING_SUCCESS;
}

int main(const int argc, char *args[]) {
    char *target_directory = NULL;
    const arguments_parsing_error a_error = parse_arguments(argc, args, &target_directory);
    if (a_error != ARGUMENTS_PARSING_SUCCESS) {
        return a_error;
    }

    const file_parsing_error f_error = parse_file(target_directory);
    if (f_error != FILE_PARSING_SUCCESS) {
        return f_error;
    }

    return 0;
}
