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

char *append_string(const char *a, const char *b) {
    if (!a && !b) return NULL;
    if (!a) return strdup(b);
    if (!b) return strdup(a);

    const size_t len = strlen(a) + strlen(b) + 1;
    char *result = malloc(len);
    if (!result) return NULL;

    strcpy(result, a);
    strcat(result, b);
    return result;
}

char *remove_spaces_and_new_lines(char *str) {
    int j = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] != '\n' && str[i] != ' ') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
    return str;
}

int create_nested_directories(const char *path) {
    char temp[1024];
    snprintf(temp, sizeof(temp), "%s", path);
    const size_t len = strlen(temp);

    if (temp[len - 1] == '/') {
        temp[len - 1] = '\0';
    }

    for (char *p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(temp, 0755) != 0 && errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(temp, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }

    return 0;
}

int create_file(const char *path) {
    FILE *file = fopen(path, "w");
    if (!file) {
        perror("fopen");
        return -1;
    }
    fclose(file);
    return 0;
}

int parse_line(int *spaces, char **parent, char *line) {
    const size_t length = strlen(line);
    if (length < *spaces + 1) return 1;

    for (int i = 0; i < *spaces; i++) {
        if (line[i] != ' ') return 1;
    }

    const bool is_dir = line[*spaces] == '/';
    char *cleaned = remove_spaces_and_new_lines(line);
    if (!cleaned) return 1;

    if (is_dir) {
        char *full_path = append_string(*parent ? *parent : "", cleaned + 1);
        if (!full_path) return 1;

        if (create_nested_directories(full_path) == -1) {
            free(full_path);
            return 1;
        }

        char *new_parent = append_string(full_path, "/");
        free(full_path);
        if (!new_parent) return 1;

        free(*parent);
        *parent = new_parent;
        *spaces += 2;
    } else {
        char *file_path = append_string(*parent ? *parent : "", cleaned);
        if (!file_path) return 1;

        if (create_file(file_path) == -1) {
            free(file_path);
            return 1;
        }
        free(file_path);
    }

    return 0;
}

file_parsing_error parse_file(const char *target_directory) {
    FILE *file = fopen(target_directory, "r");
    if (!file) return ERROR_OPENING_FILE;

    char *line = NULL;
    size_t len = 0;
    int spaces = 0;
    char *parent = NULL;

    while (getline(&line, &len, file) != -1) {
        if (parse_line(&spaces, &parent, line) != 0) {
            free(line);
            free(parent);
            fclose(file);
            return ERROR_PARSING_LINE;
        }
    }

    free(line);
    free(parent);
    fclose(file);
    return FILE_PARSING_SUCCESS;
}

int main(const int argc, char *args[]) {
    char *target_directory = NULL;
    const arguments_parsing_error a_error = parse_arguments(argc, args, &target_directory);
    if (a_error != ARGUMENTS_PARSING_SUCCESS) {
        fprintf(stderr, "Argument parsing error: %d\n", a_error);
        return a_error;
    }

    const file_parsing_error f_error = parse_file(target_directory);
    if (f_error != FILE_PARSING_SUCCESS) {
        fprintf(stderr, "File parsing error: %d\n", f_error);
        return f_error;
    }

    return 0;
}
