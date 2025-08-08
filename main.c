#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

typedef enum {
    ARGUMENTS_PARSING_SUCCESS = 0,
    TARGET_DIRECTORY_NOT_PASSED = 1,
    INVALID_COMMAND_PASSED = 2,
} arguments_parsing_error;

typedef enum {
    FILE_PARSING_SUCCESS = 0,
    ERROR_OPENING_FILE = 3,
    ERROR_PARSING_LINE = 4,
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

char *append_string(char *dest, const char *src) {
    if (src == NULL) return dest;

    if (dest == NULL) {
        dest = malloc(strlen(src) + 1);
        if (dest != NULL)
            strcpy(dest, src);
    } else {
        const size_t length = strlen(dest) + strlen(src) + 1;
        char *temp = realloc(dest, length);

        if (temp != NULL) {
            dest = temp;
            strcat(dest, src);
        }
    }

    return dest;
}

char *remove_spaces_and_new_lines(const char *str) {
    if (str == NULL) return NULL;

    const size_t len = strlen(str);
    char *result = malloc(len + 1);

    if (result == NULL) return NULL;

    int j = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] != ' ' && str[i] != '\n') {
            result[j] = str[i];
            j++;
        }
    }

    result[j] = '\0';

    return result;
}

int create_directory(const char *path) {
    const int result = mkdir(path, 0755);

    if (result != 0) {
        if (errno == EEXIST) {
            return 0;
        }
        return -1;
    }

    return 0;
}

int create_file(char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return -1;
    }

    fclose(file);
    return 0;
}

int parse_line(int *spaces, char **parent, const char *line) {
    const size_t length = strlen(line);

    if (length < *spaces + 1) return 1;

    for (int i = 0; i < *spaces; i++) {
        const char current_char = line[i];
        if (current_char != ' ') return 1;
    }

    const bool isDirectory = line[*spaces] == '/';
    if (isDirectory) {
        char *directory_name = remove_spaces_and_new_lines(line);
        if (directory_name == NULL) return 1;

        const char *directory_path = directory_name + 1;

        if (create_directory(directory_path) == -1) {
            free(directory_name);
            return 1;
        }

        *spaces += 2;
        *parent = append_string(*parent, directory_name);
        free(directory_name);
    } else {
        char *file_name = remove_spaces_and_new_lines(line);
        if (file_name == NULL) return 1;

        char *file_path = NULL;
        file_path = append_string(file_path, *parent);
        file_path = append_string(file_path, "/");
        file_path = append_string(file_path, file_name);
        file_path = file_path + 1;

        if (create_file(file_path) == -1) {
            free(file_name);
            return 1;
        }

        free(file_name);
    }

    return 0;
}

file_parsing_error parse_file(const char *target_directory) {
    FILE *file = fopen(target_directory, "r");
    if (file == NULL) {
        return ERROR_OPENING_FILE;
    }

    size_t len = 0;
    char *line = NULL;
    int spaces = 0;
    char *parent = NULL;

    while (getline(&line, &len, file) != -1) {
        const int line_parsing_error = parse_line(&spaces, &parent, line);
        if (line_parsing_error != 0) {
            return ERROR_PARSING_LINE;
        }
    }

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
