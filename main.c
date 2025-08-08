#include <setjmp.h>
#include <stdio.h>
#include <string.h>

jmp_buf exception_buffer;

enum {
    TARGET_DIRECTORY_NOT_PASSED = 1,
    INVALID_COMMAND_PASSED = 2,
};

void throw_exception(const int error_code) {
    longjmp(exception_buffer, error_code);
}

char *get_target_directory(const int number_of_arguments, char *arguments[]) {
    if (number_of_arguments < 2) {
        throw_exception(TARGET_DIRECTORY_NOT_PASSED);
    }

    char *string = arguments[1];
    const size_t length = strlen(string);
    if (length < 3 || string[0] != '-' || string[1] != '-') {
        throw_exception(INVALID_COMMAND_PASSED);
    }

    char *target_directory = string + 2;
    if (*target_directory == '\0') {
        throw_exception(INVALID_COMMAND_PASSED);
    }

    return target_directory;
}

int main(const int argc, char *args[]) {
    int exception_code = setjmp(exception_buffer);
    if (exception_code == 0) {
        char *target_directory = get_target_directory(argc, args);
        printf("%s\n", target_directory);
    } else {
        printf("Exception code: %d\n", exception_code);
    }

    return 0;
}
